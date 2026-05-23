#include "task_scheduler.h"
#include "bsp_oled.h"
#include <string.h>

extern uint32_t system_mode;

/* ============================ 菜单树 ============================ */
/* 根据你的任务自定义以下内容 */

// static menu_item_t menu_chasis_items[];
// static menu_item_t menu_gimbal_items[];

// /* ---- 底盘子菜单 ---- */
// static menu_item_t menu_chasis_items[] = {
//     {"forward",       0, NULL, TASK1},
//     {"backward",       0, NULL, TASK2},
//     {"stop",       0, NULL, TASK_INIT},
//     {"return",       0, NULL, MENU_VAL_BACK},
// };
// #define CHASIS_CNT  (sizeof(menu_chasis_items) / sizeof(menu_item_t))

// /* ---- 云台子菜单 ---- */
// static menu_item_t menu_gimbal_items[] = {
//     {"pitch",       0, NULL, TASK3},
//     {"row",       0, NULL, TASK4},
//     {"row",       0, NULL, TASK4},
//     {"return",       0, NULL, MENU_VAL_BACK},
// };
// #define GIMBAL_CNT  (sizeof(menu_gimbal_items) / sizeof(menu_item_t))

// /* ---- 根菜单项数组 ---- */
// static menu_item_t menu_root_items[] = {
//     {"chasis",   CHASIS_CNT, menu_chasis_items, 0},
//     {"gimbal",   GIMBAL_CNT, menu_gimbal_items, 0},
// };
// #define ROOT_CNT  (sizeof(menu_root_items) / sizeof(menu_item_t))

// /* ---- 根菜单容器 ---- */
// static menu_item_t menu_root = {
//     "", ROOT_CNT, menu_root_items, 0
// };


static menu_item_t menu_debug_items[];




static menu_item_t menu_gyro_items[] = {
    {"  Yaw",          0, NULL, GYRO_ACC},
    {"  return",       0, NULL, MENU_VAL_BACK},
};
#define GYRO_CNT  (sizeof(menu_gyro_items) / sizeof(menu_item_t))

static menu_item_t menu_gimbal_items[] = {
    {"  Connect Test", 0, NULL, GIMBAL_CONNECT_TEST},
    {"  Move Test",    0, NULL, GIMBAL_MOVE_TEST},
    {"  Set Zero",     0, NULL, GIMBAL_SET_ZERO},
    {"  Enable",       0, NULL, GIMBAL_ENABLE},
    {"  Disable",      0, NULL, GIMBAL_DISABLE},
    {"  Return Zero",  0, NULL, GIMBAL_RETURN_ZERO},
    {"  return",       0, NULL, MENU_VAL_BACK},
};
#define GIMBAL_CNT  (sizeof(menu_gimbal_items) / sizeof(menu_item_t))

static menu_item_t menu_chasis_items[] = {
    {"  X Test",       0, NULL, CHASIS_X_TEST},
    {"  Y Test",       0, NULL, CHASIS_Y_TEST},
    {"  Wz Test",      0, NULL, CHASIS_WZ_TEST},
    {"  return",       0, NULL, MENU_VAL_BACK},
};
#define CHASIS_CNT  (sizeof(menu_chasis_items) / sizeof(menu_item_t))

static menu_item_t menu_debug_items[] = {
    {"Gyro",          GYRO_CNT,   menu_gyro_items,   0},
    {"Gimbal",        GIMBAL_CNT, menu_gimbal_items, 0},
    {"Chasis",        CHASIS_CNT, menu_chasis_items,  0},
    {"return",       0, NULL, MENU_VAL_BACK},
};
#define DEBUG_CNT  (sizeof(menu_debug_items) / sizeof(menu_item_t))

/* ---- 根菜单项数组 ---- */
static menu_item_t menu_root_items[] = {
    {"Debug",   DEBUG_CNT, menu_debug_items, 0},
};
#define ROOT_CNT  (sizeof(menu_root_items) / sizeof(menu_item_t))

/* ---- 根菜单容器 ---- */
static menu_item_t menu_root = {
    "", ROOT_CNT, menu_root_items, 0
};












/* ============================ 状态 ============================ */

static menu_item_t *cur_menu = NULL;
static menu_item_t *menu_stack[MENU_MAX_DEPTH];
static uint8_t      menu_sp   = 0;
static uint8_t      cursor    = 0;
static uint8_t      scroll    = 0;
static uint8_t      need_draw = 1;

/* 按键标志（由 Key_A18_Pressed / Key_B21_Pressed 设置） */
volatile uint8_t flag_next  = 0;
volatile uint8_t flag_enter = 0;

/* 倒计时状态 */
static menu_item_t *pending_task    = NULL;
static uint32_t     countdown_start = 0;
extern volatile uint32_t g_tick;

/* ============================ 绘制 ============================ */

static void Menu_Draw(void)
{
    uint8_t cnt = cur_menu->child_cnt;
    uint8_t vis = (cnt < MENU_PAGE_SIZE) ? cnt : MENU_PAGE_SIZE;

    OLED_Clear();
    for (uint8_t i = 0; i < vis; i++) {
        uint8_t idx = scroll + i;
        uint8_t y   = i * 2;   /* 16px 字体: y = 0,2,4,6 */
        if (idx == cursor) {
            OLED_ShowString(0,  y, (uint8_t *)">", 16);
            OLED_ShowString(16, y, (uint8_t *)cur_menu->child[idx].name, 16);
        } else {
            OLED_ShowString(8,  y, (uint8_t *)cur_menu->child[idx].name, 16);
        }
    }
    need_draw = 0;
}

/* ============================ 导航 ============================ */

static void Menu_Next(void)
{
    uint8_t cnt = cur_menu->child_cnt;
    if (cnt == 0) return;
    cursor++;
    if (cursor >= cnt) cursor = 0;

    /* 滚动窗口 */
    if (cursor < scroll) scroll = cursor;
    if (cursor >= scroll + MENU_PAGE_SIZE) scroll = cursor - MENU_PAGE_SIZE + 1;

    need_draw = 1;
}

static void Menu_Enter(void)
{
    menu_item_t *sel = &cur_menu->child[cursor];

    if (sel->child_cnt > 0) {
        /* 子菜单: 压栈进入 */
        if (menu_sp < MENU_MAX_DEPTH) {
            menu_stack[menu_sp++] = cur_menu;
            cur_menu = sel;
            cursor = 0;
            scroll = 0;
            need_draw = 1;
        }
    } else if (sel->val == MENU_VAL_BACK) {
        /* 返回上级 */
        if (menu_sp > 0) {
            cur_menu = menu_stack[--menu_sp];
            cursor = 0;
            scroll = 0;
            need_draw = 1;
        }
    } else {
        /* 叶子: 开始倒计时 5s */
        pending_task    = sel;
        countdown_start = g_tick;
        OLED_Clear();
        OLED_ShowString(0, 2, (uint8_t *)"start in", 16);
        OLED_ShowString(0, 4, (uint8_t *)sel->name, 16);
    }
}

/* ============================ 对外接口 ============================ */

void Menu_Init(void)
{
    cur_menu  = &menu_root;
    cursor    = 0;
    scroll    = 0;
    menu_sp   = 0;
    need_draw = 1;
    Menu_Draw();
}

void Menu_Run(void)
{
    /* ---- 倒计时中 ---- */
    if (pending_task) {
        uint32_t elapsed = (g_tick - countdown_start) / 1000;
        if (elapsed >= MENU_COUNTDOWN_S) {
            /* 倒计时到, 执行任务 */
            const char *name = pending_task->name;
            system_mode  = pending_task->val;
            pending_task = NULL;
            OLED_Clear();
            OLED_ShowString(0, 2, (uint8_t *)"running:", 16);
            OLED_ShowString(0, 4, (uint8_t *)name, 16);
        } else {
            OLED_ShowNum(96, 4, MENU_COUNTDOWN_S - elapsed, 1, 16);
            if (flag_enter) {   /* PB21 取消 */
                flag_enter   = 0;
                pending_task = NULL;
                Menu_Init();
            }
        }
        return;
    }

    if (flag_next)  { flag_next  = 0; Menu_Next();  }
    if (flag_enter) { flag_enter = 0; Menu_Enter(); }
    if (need_draw)  { Menu_Draw(); }
}

/* ============================ 任务完成 ============================ */

void Task_Done(void)
{
    system_mode = TASK_INIT;
    Task_Cleanup();
    Menu_Init();
}
