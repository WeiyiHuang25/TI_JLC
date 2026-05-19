#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"

/* CAN send timeout: max iterations waiting for NORMAL mode */
#define CAN_SEND_TIMEOUT  (1000000U)

bool CAN_send_std_frame(uint32_t id, uint8_t* data, uint16_t len);
bool CAN_send_ext_frame(uint32_t id, uint8_t* data, uint16_t len);


