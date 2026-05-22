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

#define TASK_INIT   0
#define TASK1       1
#define TASK2       2
#define TASK3       3
#define TASK4       4


void User_Init(void);

