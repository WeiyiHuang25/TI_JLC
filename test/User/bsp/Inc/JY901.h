#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"

/* JY901 传感器水平安装（平行于地面） */

extern float jy901_yaw;   /* 偏航角 (°) */

void JY901_Init(void);
void JY901_Update(void);
