#include "pid.h"
#include <stdlib.h>   /* abs() */

/* =========================================================================
 * PID 初始化
 * ========================================================================= */
void pid_init(pid_t *pid, float Kp, float Ki, float Kd, float i_max, int32_t o_max)
{
    pid->Kp        = Kp;
    pid->Ki        = Ki;
    pid->Kd        = Kd;

    pid->target    = 0;
    pid->current   = 0;
    pid->error     = 0;
    pid->prev_error = 0;

    pid->integral  = 0.0f;
    pid->i_limit   = i_max;

    pid->output    = 0;
    pid->out_limit = o_max;

    /* 死区 + 方向迟滞默认值 */
    pid->deadband      = 0;
    pid->direction     = 0;
    pid->dir_flip_cnt  = 0;
    pid->dir_flip_thr  = 0;
}

/* =========================================================================
 * 通用 PID 计算 (位置式 + 积分限幅抗饱和 + 输出限幅)
 *
 * 公式: output = Kp*error + Σ(Ki*error) + Kd*(error - prev_error)
 *
 * 同一函数可复用于多个 PID 实例：
 *   pid_t pid_speed[4];  // 4 个速度环
 *   pid_t pid_pos[4];    // 4 个位置环
 *   pid_compute(&pid_speed[m], target, current);
 *
 * 由于 PID 周期固定 (ENC_PID_PERIOD_MS=10ms)，dt 已合并到 Ki/Kd 系数中。
 * ========================================================================= */
int32_t pid_compute(pid_t *pid, int32_t target, int32_t current)
{
    float p_term, i_term, d_term;
    float output_f;

    /* 1. 保存输入 */
    pid->target  = target;
    pid->current = current;

    /* 2. 计算误差 */
    pid->prev_error = pid->error;
    pid->error      = target - current;

    /* 3. 比例项 */
    p_term = pid->Kp * (float)pid->error;

    /* 4. 积分项 (带积分分离 + 限幅抗饱和) */
    pid->integral += pid->Ki * (float)pid->error;

    /* 积分限幅 */
    if (pid->integral >  pid->i_limit) pid->integral =  pid->i_limit;
    if (pid->integral < -pid->i_limit) pid->integral = -pid->i_limit;

    i_term = pid->integral;

    /* 5. 微分项 (基于误差变化率, 避免微分突变) */
    d_term = pid->Kd * (float)(pid->error - pid->prev_error);

    /* 6. 合成输出 */
    output_f = p_term + i_term + d_term;

    /* 7. 输出限幅 */
    if (output_f >  (float)pid->out_limit) output_f =  (float)pid->out_limit;
    if (output_f < -(float)pid->out_limit) output_f = -(float)pid->out_limit;

    pid->output = (int32_t)output_f;

    return pid->output;
}

/* =========================================================================
 * PID 重置
 * ========================================================================= */
void pid_reset(pid_t *pid)
{
    pid->error       = 0;
    pid->prev_error  = 0;
    pid->integral    = 0.0f;
    pid->output      = 0;
    pid->direction   = 0;
    pid->dir_flip_cnt = 0;
}

/* =========================================================================
 * 方向强制设定
 * ========================================================================= */
void pid_set_direction(pid_t *pid, int8_t dir)
{
    if (dir >= 0) pid->direction =  1;
    else          pid->direction = -1;
    pid->dir_flip_cnt = 0;
}

/* =========================================================================
 * 速度 PID 驱动 (编码器无方向 → 死区 + 方向迟滞保护)
 *
 * 适用场景：编码器只能测 |速度|，方向由 PID 输出符号推定。
 *
 * 方向翻转保护策略：
 *   1. 目标符号翻转 → 立即强制切换方向（响应遥控指令）
 *   2. PID 输出符号与当前方向不一致 → 需连续 dir_flip_thr 个周期才翻转
 *   3. |output| < deadband → 返回 0，不驱动电机
 *
 * 调用方用法：
 *   pid->deadband     = 50;    // 死区阈值
 *   pid->dir_flip_thr = 4;     // 需连续 4 周期才允许反向
 *   int32_t drive = pid_speed_drive(&pid, target, abs_encoder_speed);
 *   motor_set(motor, drive >= 0, (uint16_t)abs(drive));
 * ========================================================================= */
int32_t pid_speed_drive(pid_t *pid, int32_t target, uint32_t abs_speed)
{
    int32_t cur_signed;
    int8_t  output_sign;

    /* ── 1. 目标变号 → 立即切换方向并重置积分 ── */
    if (pid->prev_error != 0) {  /* 非首次 */
        int8_t target_sign = (target >= 0) ? 1 : -1;
        if (pid->direction != 0 && pid->direction != target_sign) {
            pid->direction    = target_sign;
            pid->dir_flip_cnt = 0;
            pid->integral     = 0.0f;    /* 换向时清零积分，防饱和反向冲出 */
        }
    }

    /* ── 2. 方向未知 → 用目标符号初始化 ── */
    if (pid->direction == 0) {
        pid->direction = (target >= 0) ? 1 : -1;
    }

    /* ── 3. 给编码器速度赋符号 ── */
    cur_signed = (int32_t)abs_speed * pid->direction;

    /* ── 4. 核心 PID 计算 ── */
    pid_compute(pid, target, cur_signed);

    /* ── 5. 方向迟滞判断 ── */
    output_sign = (pid->output >= 0) ? 1 : -1;

    if (pid->dir_flip_thr > 0 && pid->direction != output_sign) {
        /* PID 输出方向与当前推定方向不一致，累计计数 */
        pid->dir_flip_cnt++;
        if (pid->dir_flip_cnt >= pid->dir_flip_thr) {
            /* 连续 N 周期要求反向 → 确认翻转 */
            pid->direction    = output_sign;
            pid->dir_flip_cnt = 0;
            pid->integral     = 0.0f;    /* 换向清零积分 */
        } else {
            /* 还未达阈值 → 强制输出为 0，等待更多证据 */
            pid->output = 0;
            return 0;
        }
    } else {
        /* 方向一致 → 重置翻转计数 */
        pid->dir_flip_cnt = 0;
    }

    /* ── 6. 死区 ── */
    if (pid->deadband > 0 && abs(pid->output) < pid->deadband) {
        pid->output = 0;
    }

    return pid->output;
}