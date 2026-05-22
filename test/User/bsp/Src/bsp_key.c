#include "bsp_key.h"

static volatile uint32_t key_tick[2] = {0};
extern volatile uint32_t g_tick;

/* =========================================================================
 * 初始化：使能 GPIO 中断
 * ========================================================================= */
void KEY_Init(void)
{
    NVIC_EnableIRQ(KEY_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(KEY_GPIOB_INT_IRQN);
}

/* =========================================================================
 * GROUP1 中断分发
 * PA18 上升沿 / PB21 下降沿 → 记录时间戳
 * ========================================================================= */
void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
    {
    case KEY_GPIOA_INT_IIDX:
        key_tick[0] = g_tick;
        break;
    case KEY_GPIOB_INT_IIDX:
        key_tick[1] = g_tick;
        break;
    }
}

/* =========================================================================
 * 消抖轮询 — 在 ms_callback() 中每 1ms 调用
 * ========================================================================= */
void KEY_Poll(void)
{
    /* PA18: 默认低，上升沿 → 按下为高 */
    if (key_tick[0] && (g_tick - key_tick[0] >= KEY_DEBOUNCE_MS)) {
        key_tick[0] = 0;
        if (DL_GPIO_readPins(KEY_A18_PORT, KEY_A18_PIN))
            Key_A18_Pressed();
    }

    /* PB21: 默认高(上拉)，下降沿 → 按下为低 */
    if (key_tick[1] && (g_tick - key_tick[1] >= KEY_DEBOUNCE_MS)) {
        key_tick[1] = 0;
        if (!DL_GPIO_readPins(KEY_B21_PORT, KEY_B21_PIN))
            Key_B21_Pressed();
    }
}

/* =========================================================================
 * 用户回调（弱定义，在别处覆盖即可）
 * ========================================================================= */
__attribute__((weak)) void Key_A18_Pressed(void) {}
__attribute__((weak)) void Key_B21_Pressed(void) {}
