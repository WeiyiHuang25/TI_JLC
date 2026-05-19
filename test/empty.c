/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "bsp_UART.h"
#include "bsp_CAN.h"
#include "bsp_PWM.h"


// test

uint8_t test_uart_buffer[64];
size_t len = 0;

char   uart_send_buff[] = "SBTI";

uint32_t id = 0x123;
uint8_t data[4] = {2, 2, 3, 4};

float speed_fr = 0.0f;
float speed_fl = 0.0f;
float speed_rl = 0.0f;
float speed_rr = 0.0f;
volatile uint32_t g_tick = 0;
volatile uint8_t  g_flag_100hz = 0;


// test end

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(CANFD0_INT_IRQn);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    uart_receive_start();
    

    DL_GPIO_setPins(LED_PORT, LED_LED_PIN_PIN);

    while (1)
    {
        if (g_flag_100hz) {
            g_flag_100hz = 0;
            
        }
    }
}

void SysTick_Handler(void)
{
    uart_isIDLE();
    g_tick++;
    if ((g_tick % 10U) == 0U) {
        g_flag_100hz = 1;
    }
    speed_fr = motor_get_speed(MOTOR_FR);
    speed_fl = motor_get_speed(MOTOR_FL);
    speed_rr = motor_get_speed(MOTOR_RR);
    speed_rl = motor_get_speed(MOTOR_RL);
}
