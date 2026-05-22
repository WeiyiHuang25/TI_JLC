#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"
#include "ZDT_Driver.h"

#define PITCH_MOTOR     0X02
#define YAW_MOTOR       0X01


#define PITCH_MAX_ANGLE    45.0f



void gimbal_set_zero(void);
void gimbal_disable(void);
void gimbal_enable(void);
