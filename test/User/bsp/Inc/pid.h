#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"
#include "bsp_PWM.h"

/* =========================================================================
 * PID 控制器结构体
 * ========================================================================= */
typedef struct {
    /* ── PID 增益 ── */
    float Kp;            /* 比例系数 */
    float Ki;            /* 积分系数 */
    float Kd;            /* 微分系数 */

    /* ── 状态变量 ── */
    int32_t target;      /* 目标值 */
    int32_t current;     /* 当前值（带符号） */
    int32_t error;       /* 本次误差 */
    int32_t prev_error;  /* 上次误差 */

    float   integral;    /* 积分累加值 */
    float   i_limit;     /* 积分限幅 (绝对值) */

    int32_t output;      /* PID 原始输出值 */
    int32_t out_limit;   /* 输出限幅 (绝对值), 通常 = PWM_PERIOD */

    /* ── 死区 + 方向迟滞 (编码器无方向时的保护) ── */
    int32_t deadband;        /* 死区: |output| < deadband → 输出 0 */
    int8_t  direction;       /* 当前推定方向: +1=正转, -1=反转, 0=未知 */
    int8_t  dir_flip_cnt;    /* 方向翻转连续计数 */
    int8_t  dir_flip_thr;    /* 方向翻转需持续 N 周期才确认 (建议 3~5) */
} pid_t;

/* =========================================================================
 * 公开 API
 * ========================================================================= */

/**
 * @brief 初始化 PID 控制器
 * @param pid    PID 结构体指针
 * @param Kp     比例系数
 * @param Ki     积分系数
 * @param Kd     微分系数
 * @param i_max  积分限幅绝对值
 * @param o_max  输出限幅绝对值 (通常 PWM_PERIOD)
 */
void pid_init(pid_t *pid, float Kp, float Ki, float Kd, float i_max, int32_t o_max);

/**
 * @brief 通用 PID 计算（可用于速度环、位置环、电流环等）
 * @param pid     PID 结构体指针
 * @param target  目标值（与 current 同单位即可）
 * @param current 当前值（带符号）
 * @return        PID 原始输出值
 *
 * @note  只需为每个控制对象声明一个 pid_t 实例，传入同一函数即可复用。
 *        此函数只做纯 PID 数学运算，不做死区/方向处理。
 */
int32_t pid_compute(pid_t *pid, int32_t target, int32_t current);

/**
 * @brief 速度 PID 驱动（专用于编码器无法测方向、仅能获取 |速度| 的场景）
 * @param pid       PID 结构体指针
 * @param target    目标速度（带符号，正=期望正转）
 * @param abs_speed 编码器实测速度绝对值（≥0）
 * @return          最终 PWM 输出（带符号），0 表示死区/停止
 *
 * @note  内部自动处理：
 *        1. 用 pid->direction 给 abs_speed 赋符号
 *        2. 当 pid->output 符号与当前方向不一致时，需连续 dir_flip_thr 个周期才翻转
 *        3. |output| < deadband → 返回 0，方向不变
 *        4. 目标符号翻转时立即强制切换方向
 */
int32_t pid_speed_drive(pid_t *pid, int32_t target, uint32_t abs_speed);

/**
 * @brief 强制设定方向（用于目标变号或初始定向）
 * @param pid  PID 结构体指针
 * @param dir  +1=正转, -1=反转
 */
void pid_set_direction(pid_t *pid, int8_t dir);

/**
 * @brief 重置 PID 历史状态（积分、上次误差清零）
 * @param pid  PID 结构体指针
 */
void pid_reset(pid_t *pid);
