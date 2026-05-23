#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"

/* JY901 传感器水平安装（平行于地面） */

typedef struct {
    /* 原始传感器值 */
    float acc_x, acc_y, acc_z;   /* 加速度  m/s²      */
    float gyro_x, gyro_y, gyro_z;/* 角速度  °/s       */
    float roll, pitch, yaw;      /* 姿态角  °         */

    /* 积分解算（仅水平面 XY） */
    float vel_x, vel_y;          /* 速度    m/s       */
    float pos_x, pos_y;          /* 位置    m         */
} jy901_t;

extern jy901_t jy901;

void JY901_Init(void);
void JY901_Update(void);   /* 100ms 调用一次 */
