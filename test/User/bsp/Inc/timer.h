/**
 * @file    timer.h
 * @brief   TIMG12 64-bit timestamp (10MHz / 0.1us resolution)
 *
 * TIMG12 is a 32-bit down-counter running at 10 MHz.
 * Each LOAD (zero) event ISR extends the counter to 64 bits.
 *
 * Maximum count duration:
 *   2^64 / 10,000,000 = 1,844,674,407,370.955 seconds
 *   ≈ 58,494 years
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 *  API
 * ======================================================================== */

/**
 * @brief  Initialize the timestamp module.
 *         Enables TIMG12 NVIC interrupt (the timer peripheral itself
 *         is already configured by SysConfig).
 */
void Timestamp_Init(void);

/**
 * @brief  Get a 64-bit raw tick counter (monotonic, 0.1 us per tick).
 * @return Raw ticks since timer start.
 */
uint64_t Timestamp_GetTicks(void);

/**
 * @brief  Get elapsed time in microseconds.
 * @return Microseconds since timer start (wraps after ~584,942 years).
 */
uint64_t Timestamp_GetUs(void);

/**
 * @brief  Get elapsed time in milliseconds.
 * @return Milliseconds since timer start.
 */
uint64_t Timestamp_GetMs(void);

/**
 * @brief  Convert tick difference to microseconds.
 * @param  ticks  Delta ticks (from Timestamp_GetTicks).
 * @return Microseconds.
 */
static inline uint64_t Timestamp_TicksToUs(uint64_t ticks)
{
    return ticks / 10U;   /* 10 ticks = 1 us at 10 MHz */
}

/**
 * @brief  Convert tick difference to milliseconds.
 * @param  ticks  Delta ticks.
 * @return Milliseconds.
 */
static inline uint64_t Timestamp_TicksToMs(uint64_t ticks)
{
    return ticks / 10000U;  /* 10,000 ticks = 1 ms */
}

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H__ */
