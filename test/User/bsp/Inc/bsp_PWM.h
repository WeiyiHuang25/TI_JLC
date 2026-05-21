#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"

/* =========================================================================
 * 电机编号
 * ========================================================================= */
typedef enum {
    MOTOR_FL = 0,   /* 前左 (Front-Left)   → FRONT_OUT CCP0/CCP1 */
    MOTOR_FR = 1,   /* 前右 (Front-Right)  → FRONT_OUT CCP2/CCP3 */
    MOTOR_RL = 2,   /* 后左 (Rear-Left)    → RL_OUT    CCP0/CCP1 */
    MOTOR_RR = 3,   /* 后右 (Rear-Right)   → RR_OUT    CCP0/CCP1 */
    MOTOR_NUM
} motor_id_t;

/* =========================================================================
 * PWM 参数 (与 SysConfig timerCount 一致)
 * ========================================================================= */
#define PWM_PERIOD               (4999U)
#define PWM_PERIOD_MAX           (PWM_PERIOD)

/* =========================================================================
 * 四电机引脚接线表
 * ========================================================================= */
/*
 *                                                   编码器
 *  电机          H桥 IN1      H桥 IN2      A相          B相
 *  ────────────  ───────────  ───────────  ───────────  ───────────
 *  MOTOR_FL 前左  PA0  (CCP0)  PA1  (CCP1)  PA23 (CCP0)  PA13 (CCP1) [FFRONT_IN]
 *  MOTOR_FR 前右  PA15 (CCP2)  PA12 (CCP3)  -            -
 *  MOTOR_RL 后左  PA28 (CCP0)  PA16 (CCP1)  PA17 (CCP0)  PA18 (CCP1) [REAR_IN]
 *  MOTOR_RR 后右  PA29 (CCP0)  PA22 (CCP1)  -            -
 *
 *  定时器资源 (输出保留 3 个 Timer 控制 4 电机 / 输入现保留 2 个 Timer):
 *    FRONT_OUT = TIMA0  (4ch PWM)  → FL + FR
 *    RL_OUT    = TIMA1  (2ch PWM)  → RL
 *    RR_OUT    = TIMG6  (2ch PWM)  → RR
 *
 *    FFRONT_IN = TIMG0  (2ch CAPT) → 前方编码器
 *       - CCP0 : 对应左轮还是右轮？视乎你在 SysConfig 绑定的引脚 (由对应宏映射)
 *       - CCP1 : 对应另一轮
 *    REAR_IN   = TIMG7  (2ch CAPT) → 后方编码器
 *       - CCP0 : 对应左轮还是右轮？
 *       - CCP1 : 对应另一轮
 *
 *  【重要引脚接线指引】
 *   既然遇到了 SysConfig 的 CCP 通道顺序不可调换的问题，
 *   你只需要把你最终在 SysConfig 里能够选定并锁死的 A 相引脚记录下来即可：
 *   
 *   假设你强行分配的结果如下（请根据你 SysConfig 界面实际显示的为准去接线）：
 *   前左 (MOTOR_FL) 的 A 相线 ➡️ 接到 FFRONT_IN 的 CCP1 引脚 (比如 PA23)
 *   前右 (MOTOR_FR) 的 A 相线 ➡️ 接到 FFRONT_IN 的 CCP0 引脚 (比如 PA13)
 *   后左 (MOTOR_RL) 的 A 相线 ➡️ 接到 REAR_IN 的 CCP1 引脚 (比如 PA17)
 *   后右 (MOTOR_RR) 的 A 相线 ➡️ 接到 REAR_IN 的 CCP0 引脚 (比如 PA18)
 *
 *  PWM:  200Hz (80MHz / 80 prescaler / 5000 period)
 *  CAPT: TIMG0/7=1MHz (单边沿上升沿触发)
 *  已占用引脚 (不可冲突):
 *    PA10/11=UART0  PA26/27=CAN  PB22=LED  PA19/20=SWD
 *    PA3/4=LFXT  PA5/6=HFXT
 */

/* =========================================================================
 * 编码器参数 (方波 AB 相, 100kHz 响应)
 * ========================================================================= */
#define ENC_PPR                 (11U)     /* 基础脉冲数/转 */
#define ENC_POLES               (22U)     /* 磁环触发极数 (11对极) */
#define ENC_EDGES_PER_REV       (ENC_PPR * ENC_POLES * 4U)  /* 4x 正交解码 = 968 */
#define ENC_PID_PERIOD_MS       (10U)     /* PID 周期 10ms */

/* =========================================================================
 * 公开 API
 * ========================================================================= */

/**
 * @brief 设置电机转速与方向
 * @param motor   电机编号 (MOTOR_FL / MOTOR_FR / MOTOR_RL / MOTOR_RR)
 * @param forward true=正转, false=反转
 * @param duty    占空比 0~PWM_PERIOD (0=停转, PWM_PERIOD=全速)
 *
 * 正转：IN1 = PWM(duty), IN2 = LOW
 * 反转：IN1 = LOW,       IN2 = PWM(duty)
 */
void motor_set(motor_id_t motor, bool forward, uint16_t duty);

/**
 * @brief 电机滑行/惰行 (Coast)
 * IN1 = LOW, IN2 = LOW → H 桥 OUT1=Z, OUT2=Z
 */
void motor_coast(motor_id_t motor);

/**
 * @brief 电机刹车 (Brake)
 * IN1 = HIGH, IN2 = HIGH → H 桥 OUT1=L, OUT2=L
 */
void motor_brake(motor_id_t motor);

/**
 * @brief 获取当前电机方向
 * @return true=正转, false=反转
 */
bool motor_get_direction(motor_id_t motor);

/**
 * @brief 获取当前电机占空比
 * @return 0 ~ PWM_PERIOD
 */
uint16_t motor_get_duty(motor_id_t motor);

/**
 * @brief 一键关闭所有电机 (Coast)
 */
void motor_all_coast(void);

// /**
//  * @brief 获取电机转速 (RPM, 每次调用时根据编码器实时计算)
//  * @param motor 电机编号
//  * @return 转速 (转/分钟, RPM, 浮点), 正=正转, 负=反转, 0=停转
//  *
//  * 调用方式: 在 100Hz PID 循环中每周期调用一次。
//  * 不调用时不计算, 无后台中断开销。
//  *
//  * 原理: 读定时器当前计数值 counter 与 A/B 相捕获值,
//  *       τ = counter − capture = 距上一次边沿的时间 (ticks),
//  *       RPM = timer_hz × 60 / (τ × ENC_EDGES_PER_REV)
//  */
// float motor_get_speed(motor_id_t motor);

