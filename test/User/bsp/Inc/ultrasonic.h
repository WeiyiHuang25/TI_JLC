/**
 * @file    ultrasonic.h
 * @brief   HC-SR04 超声波测距模块驱动（2 通道）
 *
 * 通道映射:
 *   CH0: TRIG=PB15, ECHO=PB13
 *   CH1: TRIG=PB16, ECHO=PB14
 *
 * 测量原理:
 *   1. TRIG 发 >10us 高脉冲 → 模块发 8 个 40kHz 方波
 *   2. ECHO 变高，开始计时
 *   3. ECHO 变低，停止计时
 *   4. distance_mm = Δt_us * 0.17  (声速 340m/s, 往返/2)
 */

#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 *  Constants
 * ======================================================================== */

#define ULTRASONIC_CH_MAX   2

/** HC-SR04 最大量程 ~4m → 脉宽上限 ~23.5ms，我们用 25ms 超时 */
#define ULTRASONIC_TIMEOUT_US  25000U

/* ========================================================================
 *  API
 * ======================================================================== */

/**
 * @brief 初始化 2 路超声波模块（仅初始化内部状态）
 * @note  TRIG/ECHO 引脚已由 SysConfig 配置，调用前确保 SYSCFG_DL_init() 已执行。
 */
void Ultrasonic_Init(void);

/**
 * @brief 触发一次测距（CH0 或 CH1）
 * @param ch  通道号: 0 或 1
 * @note  非阻塞，触发后立即返回。通过 Ultrasonic_IsDataReady() 轮询结果。
 */
void Ultrasonic_Trigger(uint8_t ch);

/**
 * @brief 查询指定通道是否有新数据
 * @param ch  通道号: 0 或 1
 * @return true=数据就绪，可读取
 */
bool Ultrasonic_IsDataReady(uint8_t ch);

/**
 * @brief 获取最近一次测距结果（毫米）
 * @param ch  通道号: 0 或 1
 * @return 距离 (mm)，0 表示超时或无有效数据
 */
uint32_t Ultrasonic_GetDistance_mm(uint8_t ch);

/**
 * @brief 获取最近一次回波脉宽（微秒），用于调试
 * @param ch  通道号: 0 或 1
 * @return 脉宽 (us)
 */
uint32_t Ultrasonic_GetPulseWidth_us(uint8_t ch);

/**
 * @brief GPIOB 中断内部分发 — 由 GROUP1_IRQHandler 调用
 * @param iidx  DL_GPIO_getPendingInterrupt(GPIOB) 的返回值（IIDX）
 * @note  应用层不要直接调用
 */
void Ultrasonic_GPIOB_ISR(uint32_t iidx);

#ifdef __cplusplus
}
#endif

#endif /* __ULTRASONIC_H__ */
