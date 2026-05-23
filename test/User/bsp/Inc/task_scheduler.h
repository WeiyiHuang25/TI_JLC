#pragma once
#include "Task.h"
#include "ti_msp_dl_config.h"
#include <stdint.h>

/* OLED 128x64, 16px 字体, 每页 4 行 */
#define MENU_PAGE_SIZE   4
#define MENU_MAX_DEPTH   6
#define MENU_COUNTDOWN_S 2   /* 任务启动倒计时(秒) */

typedef struct menu_item {
    const char          *name;        /* 显示名称 */
    uint8_t              child_cnt;   /* 子项数量 (0=叶子) */
    struct menu_item    *child;       /* 子项数组首地址 */
    uint32_t             val;         /* 叶子: system_mode 目标值 */
} menu_item_t;

/* 特殊值: 返回上级菜单 */
#define MENU_VAL_BACK    0xFFFFFFFF

void Menu_Init(void);
void Menu_Run(void);   /* 主循环每周期调用 */
void Task_Done(void);  /* 任务完成，返回菜单 */

/* 按键标志 — 在 Key_A18_Pressed / Key_B21_Pressed 中置 1 */
extern volatile uint8_t flag_next;   /* PA18: 光标下移 */
extern volatile uint8_t flag_enter;  /* PB21: 确认    */