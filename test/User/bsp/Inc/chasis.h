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

void chasis_update(void);
void chasis_cal(void);
void chasis_set_velocity(float vx, float vy, float wz);
void chasis_stop(void);
void chasis_move(float dx, float dy, float dtheta, float speed);
bool chasis_move_is_done(void);
void chasis_move_abort(void);

float chasis_get_odom_x(void);
float chasis_get_odom_y(void);
float chasis_get_odom_theta(void);
void  chasis_reset_odom(void);


/* ── 电机方向测试与配置 ── */
void motor_set_invert(motor_id_t motor, bool invert);
bool motor_get_invert(motor_id_t motor);
void motor_test_direction_start(void);
void motor_test_direction_stop(void);
void motor_test_tick(void);