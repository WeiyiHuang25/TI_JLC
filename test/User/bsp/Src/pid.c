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
    pid->error      = 0;
    pid->prev_error = 0;
    pid->integral   = 0.0f;
    pid->output     = 0;
}