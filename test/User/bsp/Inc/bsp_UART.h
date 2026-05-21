#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"


#define UART_SEND_BUFF_SIZE 100
#define UART_RX_BUFF_SIZE   100

#define TO_IDLE_TIMEOUT     150 //us

void uart_send(uint8_t *buff, uint16_t size);
void uart_receive_start();
void uart_isIDLE();


void UART_Rx_DMA_ToIdle_Callback(uint16_t size); // 可用户自定义回调函数

extern uint8_t uart_rx_buff[UART_RX_BUFF_SIZE];
