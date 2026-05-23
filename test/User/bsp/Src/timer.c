/**
 * @file    timer.c
 * @brief   TIMG12 64-bit timestamp implementation.
 *
 * TIMG12 is a 32-bit down-counter @ 10 MHz (0.1 us resolution).
 * The LOAD (zero) ISR extends the counter to 64 bits.
 *
 * 原理:
 *   TIMG12 从 0xFFFFFFFF 向下计数到 0，触发 LOAD 中断后重载。
 *   每次中断将 32-bit 溢出计数 +1，合成 64-bit 时间戳：
 *     timestamp = (overflow << 32) | (0xFFFFFFFF - CNT)
 *
 * 可计最大时长:
 *   2^64 ticks / 10^7 Hz = 1.84467e12 秒 ≈ 58,494 年
 */

#include "timer.h"
#include "ti_msp_dl_config.h"

/* ========================================================================
 *  Static Variables
 * ======================================================================== */

/** TIMG12 zero/load event overflow counter (high 32-bit of 64-bit timestamp) */
static volatile uint32_t g_timer_overflow = 0;

/* ========================================================================
 *  TIMG12 ISR
 * ======================================================================== */

/**
 * @brief  TIMG12 LOAD (zero) interrupt handler.
 *         Increments the 32-bit overflow counter to extend to 64 bits.
 */
void TIMG12_IRQHandler(void)
{
    /* Check and clear the LOAD (zero) event flag */
    if (DL_TimerG_getPendingInterrupt(US_TIMER_INST) & DL_TIMERG_INTERRUPT_LOAD_EVENT) {
        DL_TimerG_clearInterruptStatus(US_TIMER_INST, DL_TIMERG_INTERRUPT_LOAD_EVENT);
        g_timer_overflow++;
    }
}

/* ========================================================================
 *  Initialization
 * ======================================================================== */

/**
 * @brief  Enable TIMG12 NVIC interrupt.
 * @note   The timer peripheral clock/config is already done by
 *         SYSCFG_DL_US_TI_init() in ti_msp_dl_config.c.
 */
void Timestamp_Init(void)
{
    g_timer_overflow = 0;

    /* Enable TIMG12 interrupt in NVIC */
    NVIC_EnableIRQ(US_TIMER_INST_INT_IRQN);
}

/* ========================================================================
 *  Timestamp Read (atomic, retry-safe)
 * ======================================================================== */

/**
 * @brief  Atomically read the 64-bit tick counter.
 *
 * Since the 32-bit hardware counter and the software overflow counter
 * are updated in the ISR, we use a retry loop to ensure atomicity.
 *
 * TIMG12 is a DOWN-counter: CNT goes from 0xFFFFFFFF → 0.
 * Within one wrap cycle, elapsed ticks = 0xFFFFFFFF - CNT.
 * Total   = overflow * 0x100000000 + (0xFFFFFFFF - CNT)
 */
uint64_t Timestamp_GetTicks(void)
{
    uint32_t ovf_before, ovf_after;
    uint32_t cnt;

    do {
        ovf_before = g_timer_overflow;
        cnt        = DL_TimerG_getTimerCount(US_TIMER_INST);
        ovf_after  = g_timer_overflow;
    } while (ovf_before != ovf_after);  /* retry if ISR fired during read */

    /* Convert down-counter value to upward elapsed ticks */
    return ((uint64_t)ovf_before << 32) | (0xFFFFFFFFUL - cnt);
}

/* ========================================================================
 *  Time Conversion Helpers
 * ======================================================================== */

uint64_t Timestamp_GetUs(void)
{
    return Timestamp_GetTicks() / 10U;
}

uint64_t Timestamp_GetMs(void)
{
    return Timestamp_GetTicks() / 10000U;
}
