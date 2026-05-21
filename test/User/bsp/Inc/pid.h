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
    float Kp;            /* 比例系数 */
    float Ki;            /* 积分系数 */
    float Kd;            /* 微分系数 */

    int32_t target;      /* 目标速度 (编码器边沿数 / PID周期) */
    int32_t current;     /* 当前速度 (编码器边沿数 / PID周期) */
    int32_t error;       /* 本次误差 */
    int32_t prev_error;  /* 上次误差 */

    float   integral;    /* 积分累加值 */
    float   i_limit;     /* 积分限幅 (绝对值) */

    int32_t output;      /* PID 输出值 */
    int32_t out_limit;   /* 输出限幅 (绝对值), 通常 = PWM_PERIOD */
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
 * @param target  目标值（与 current 同单位即可，如编码器边沿数/周期、角度、电流等）
 * @param current 当前值
 * @return        PID 输出值，符号隐含方向
 *
 * @note  只需为每个控制对象声明一个 pid_t 实例，传入同一函数即可复用：
 *        pid_t pid_speed[MOTOR_NUM];    // 4个速度环
 *        pid_t pid_pos[MOTOR_NUM];      // 4个位置环
 *        pid_compute(&pid_speed[0], target_spd, cur_spd);
 *        pid_compute(&pid_pos[0],  target_pos, cur_pos);
 */
int32_t pid_compute(pid_t *pid, int32_t target, int32_t current);

/**
 * @brief 重置 PID 历史状态（积分、上次误差清零）
 * @param pid  PID 结构体指针
 */
void pid_reset(pid_t *pid);
