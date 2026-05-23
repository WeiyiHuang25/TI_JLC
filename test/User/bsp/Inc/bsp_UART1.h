#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"

#define UART1_SEND_BUFF_SIZE 100
#define UART1_RX_BUFF_SIZE   100

void uart1_send(uint8_t *buff, uint16_t size);
void uart1_receive_start(void);
void uart1_isIDLE(void);

void UART1_Rx_DMA_ToIdle_Callback(uint16_t size);

extern __IO uint8_t uart1_rx_buff[UART1_RX_BUFF_SIZE];
