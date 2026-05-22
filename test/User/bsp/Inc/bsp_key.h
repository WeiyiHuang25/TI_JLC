#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"

/* 消抖时间 (ms) */
#define KEY_DEBOUNCE_MS  20U

void KEY_Init(void);
void KEY_Poll(void);

/* 用户实现：按键按下回调（弱定义，可在别处覆盖） */
void Key_A18_Pressed(void);
void Key_B21_Pressed(void);
