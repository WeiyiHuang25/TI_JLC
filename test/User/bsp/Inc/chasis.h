#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ZDT_Driver.h"
#include "bsp_oled.h"
#include "bsp_PWM.h"
#include "bsp_CAN.h"
#include "pid.h"
#include "EzTuner.h"
#include <stdlib.h>




typedef struct 
{
    uint32_t id;
    float rpm;
    float exp_rpm;
    pid_t pid;
}motor_t;

typedef struct
{
    motor_t FL;
    motor_t FR;
    motor_t RL;
    motor_t RR;
    float vx;
    float vy;
    float wz;
    float exp_vx;
    float exp_vy;
    float exp_wz;
}chasis;


typedef struct 
{
    uint32_t yaw;
    uint32_t pitch;
}gimbal;


/* =========================================================================
 * 底盘控制 API
 * ========================================================================= */
extern chasis user_chasis;
extern bool  chasis_braked;          /* 刹车完成=1, 开始运动=0 */
extern bool  chasis_trapezoid_done;  /* 梯形运动完成=1 */

void chasis_update(void);
void chasis_cal(void);
void chasis_set_velocity(float vx, float vy, float wz);
void chasis_stop(void);
void chasis_brake(void);
void chasis_linear_brake(uint32_t time_ms);
void chasis_trapezoid_move(float vx, float vy, float wz, float accel, float decel, uint32_t time_ms);
void chasis_scale_use_q2(void);
void chasis_scale_use_q1(void);
void chasis_scale_use_pos(void);




