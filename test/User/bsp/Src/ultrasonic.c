/**
 * @file    ultrasonic.c
 * @brief   HC-SR04 超声波测距驱动（2 通道，GPIO 中断 + TIMG12 64-bit 时间戳）
 *
 * 通道映射（全部由 SysConfig 配置）:
 *   CH0: TRIG=PB15, ECHO=PB13 (ULTRASONIC_0)
 *   CH1: TRIG=PB16, ECHO=PB14 (ULTRASONIC_1)
 *
 * 时序:
 *   TRIG ___/‾‾‾‾‾‾‾‾‾‾‾‾\_____________________________
 *               ≥10us
 *   ECHO ___________________/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\______
 *                           ← Δt →  (∝ distance)
 *
 *   distance_mm = Δt_us × 340 / 2000 = Δt_us × 17 / 100
 */

#include "ultrasonic.h"
#include "timer.h"
#include "ti_msp_dl_config.h"

/* ========================================================================
 *  TRIG 脉冲参数
 * ======================================================================== */

/* TRIG 高电平 ≥10us，取 15us；CPUCLK=80MHz → 1200 cycles */
#define TRIG_PULSE_US      15U
#define TRIG_PULSE_CYCLES  1200U

/* ========================================================================
 *  引脚查找表（全部来自 SysConfig）
 * ======================================================================== */

static const uint32_t _trig_pin[ULTRASONIC_CH_MAX] = {
    ULTRASONIC_TRIG_0_PIN,          /* CH0: PB15 */
    ULTRASONIC_TRIG_1_PIN,          /* CH1: PB16 */
};

static const uint32_t _echo_pin[ULTRASONIC_CH_MAX] = {
    ULTRASONIC_ULTRASONIC_0_PIN,    /* CH0: PB13 */
    ULTRASONIC_ULTRASONIC_1_PIN,    /* CH1: PB14 */
};

/** ECHO 引脚 IIDX（DL_GPIO_getPendingInterrupt 返回值） */
static const uint32_t _echo_iidx[ULTRASONIC_CH_MAX] = {
    ULTRASONIC_ULTRASONIC_0_IIDX,   /* CH0: DIO13 */
    ULTRASONIC_ULTRASONIC_1_IIDX,   /* CH1: DIO14 */
};

/* ========================================================================
 *  通道状态
 * ======================================================================== */

typedef enum {
    US_STATE_IDLE = 0,      /**< 空闲，等待触发          */
    US_STATE_WAIT_RISE,     /**< 已触发，等待 ECHO 上升沿 */
    US_STATE_WAIT_FALL,     /**< 已捕获上升沿，等待下降沿 */
} us_state_t;

typedef struct {
    us_state_t state;
    uint64_t   rise_ticks;      /**< 上升沿时间戳 (tick)  */
    uint64_t   fall_ticks;      /**< 下降沿时间戳 (tick)  */
    uint32_t   pulse_us;        /**< 脉宽 (us)            */
    uint32_t   distance_mm;     /**< 距离 (mm)            */
    bool       data_ready;      /**< 新数据就绪标志       */
} us_ch_t;

static us_ch_t g_us[ULTRASONIC_CH_MAX];

/* ========================================================================
 *  调试计数器
 * ======================================================================== */

volatile uint32_t g_dbg_trig_cnt  = 0;
volatile uint32_t g_dbg_isr_cnt   = 0;
volatile uint32_t g_dbg_last_iidx = 0xFFFFFFFF;
volatile uint8_t  g_dbg_state[2]  = {0, 0};

/* ========================================================================
 *  初始化
 * ======================================================================== */

void Ultrasonic_Init(void)
{
    for (uint8_t i = 0; i < ULTRASONIC_CH_MAX; i++) {
        g_us[i].state       = US_STATE_IDLE;
        g_us[i].data_ready  = false;
        g_us[i].distance_mm = 0;
        g_us[i].pulse_us    = 0;
    }
    /* TRIG/ECHO 引脚已由 SYSCFG_DL_GPIO_init() 配置完毕 */
}

/* ========================================================================
 *  触发测距
 * ======================================================================== */

void Ultrasonic_Trigger(uint8_t ch)
{
    if (ch >= ULTRASONIC_CH_MAX) return;

    /* 仅在空闲时触发 */
    if (g_us[ch].state != US_STATE_IDLE) return;  /* 忙则跳过 */

    g_us[ch].state      = US_STATE_WAIT_RISE;
    g_us[ch].data_ready = false;

    g_dbg_trig_cnt++;

    /* 发 TRIG 脉冲期间屏蔽 ECHO 中断以免误触发 */
    DL_GPIO_disableInterrupt(ULTRASONIC_PORT, _echo_pin[ch]);

    DL_GPIO_setPins(ULTRASONIC_PORT, _trig_pin[ch]);
    delay_cycles(TRIG_PULSE_CYCLES);
    DL_GPIO_clearPins(ULTRASONIC_PORT, _trig_pin[ch]);

    /* 清 ECHO 残留标志后重新使能中断 */
    DL_GPIO_clearInterruptStatus(ULTRASONIC_PORT, _echo_pin[ch]);
    DL_GPIO_enableInterrupt(ULTRASONIC_PORT, _echo_pin[ch]);
}

/* ========================================================================
 *  GPIOB 中断处理（由 GROUP1_IRQHandler 分发）
 * ======================================================================== */

void Ultrasonic_GPIOB_ISR(uint32_t iidx)
{
    g_dbg_isr_cnt++;
    g_dbg_last_iidx = iidx;

    for (uint8_t ch = 0; ch < ULTRASONIC_CH_MAX; ch++) {
        if (iidx != _echo_iidx[ch]) continue;

        g_dbg_state[ch] = (uint8_t)g_us[ch].state;

        bool high = (DL_GPIO_readPins(ULTRASONIC_PORT, _echo_pin[ch]) != 0);

        switch (g_us[ch].state) {
        case US_STATE_WAIT_RISE:
            if (high) {
                g_us[ch].rise_ticks = Timestamp_GetTicks();
                g_us[ch].state      = US_STATE_WAIT_FALL;
            }
            /* ⚠ 必须清除中断状态，否则下降沿不会被检测 */
            DL_GPIO_clearInterruptStatus(ULTRASONIC_PORT, _echo_pin[ch]);
            break;

        case US_STATE_WAIT_FALL:
            if (!high) {
                g_us[ch].fall_ticks = Timestamp_GetTicks();
                uint64_t dt = g_us[ch].fall_ticks - g_us[ch].rise_ticks;

                /* 10 MHz → /10 = us */
                g_us[ch].pulse_us = (uint32_t)(dt / 10U);

                if (g_us[ch].pulse_us > 0 &&
                    g_us[ch].pulse_us <= ULTRASONIC_TIMEOUT_US) {
                    /* distance_mm = pulse_us × 17 / 100  (×0.17, 纯整数) */
                    g_us[ch].distance_mm = (g_us[ch].pulse_us * 17U) / 100U;
                } else {
                    g_us[ch].distance_mm = ULTRASONIC_INVALID_MM;
                }

                g_us[ch].data_ready = true;
                g_us[ch].state      = US_STATE_IDLE;
            }
            /* ⚠ 必须清除中断状态 */
            DL_GPIO_clearInterruptStatus(ULTRASONIC_PORT, _echo_pin[ch]);
            break;

        case US_STATE_IDLE:
        default:
            /* 干扰脉冲也要清除，否则会持续触发中断 */
            DL_GPIO_clearInterruptStatus(ULTRASONIC_PORT, _echo_pin[ch]);
            break;
        }
    }
}

/* ========================================================================
 *  查询接口
 * ======================================================================== */

bool Ultrasonic_IsDataReady(uint8_t ch)
{
    if (ch >= ULTRASONIC_CH_MAX) return false;
    return g_us[ch].data_ready;
}

uint32_t Ultrasonic_GetDistance_mm(uint8_t ch)
{
    if (ch >= ULTRASONIC_CH_MAX) return 0;
    g_us[ch].data_ready = false;  /* 读后清除标志 */
    return g_us[ch].distance_mm;
}

uint32_t Ultrasonic_GetPulseWidth_us(uint8_t ch)
{
    if (ch >= ULTRASONIC_CH_MAX) return 0;
    return g_us[ch].pulse_us;
}
