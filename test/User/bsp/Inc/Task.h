#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ZDT_Driver.h"
#include "bsp_oled.h"
#include "bsp_PWM.h"
#include "bsp_CAN.h"
#include "pid.h"
#include "chasis.h"
#include "bsp_key.h"

#define TASK_INIT                   0
#define TASK1                       1
#define TASK2                       2
#define TASK3                       3
#define TASK4                       4
#define GIMBAL_SET_ZERO             5
#define CHASIS_X_TEST               6
#define CHASIS_Y_TEST               7
#define CHASIS_WZ_TEST              8
#define GIMBAL_RETURN_ZERO          9
#define GIMBAL_ENABLE               10
#define GIMBAL_DISABLE              11
#define GIMBAL_CONNECT_TEST         12
#define GYRO_ACC                    13
#define GYRO_VEL                    14
#define GYRO_POS                    15
#define GIMBAL_MOVE_TEST            16
#define ULTRASONIC_DEBUG            17
#define Q1_TASK1                    18
#define Q1_TASK2                    19

/* ============================ 时序执行上下文 ============================ */
typedef struct {
    uint8_t  flag;      /* RUN_ONCE 是否已触发         */
    uint8_t  after_done;/* RUN_AFTER 是否已触发        */
    uint32_t tick;      /* RUN_ONCE 触发时刻(g_tick)   */
} once_ctx_t;

/* 执行 fn 一次（fn 可以是函数名、有参调用、或任意表达式） */
#define RUN_ONCE(ctx, fn) \
    do { if (!(ctx).flag) { (ctx).flag = 1; (ctx).tick = g_tick; (void)(fn); } } while(0)

/* RUN_ONCE 触发后 delay_ms 毫秒执行 fn（仅一次，fn 可为 NULL，完成后自动回收） */
#define RUN_AFTER(ctx, delay_ms, fn) \
    do { \
        if ((ctx).flag && !(ctx).after_done \
            && (g_tick - (ctx).tick >= (uint32_t)(delay_ms))) { \
            (ctx).after_done = 1; \
            (void)(fn); \
        } \
    } while(0)

/* 查询 RUN_AFTER 是否已完成（延时到达，上下文已回收） */
#define RUN_ONCE_DONE(ctx)  ((ctx).after_done)


void User_Init(void);
void Task_Cleanup(void);
void Task_Jump(uint32_t next_mode);

