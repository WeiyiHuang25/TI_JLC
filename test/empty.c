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
#include "bsp_oled.h"


volatile uint32_t start_time;

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


void one_hundured_ms_callback();
void ms_callback();

int main(void)
{
    SYSCFG_DL_init();
    // OLED_Init();
    NVIC_EnableIRQ(CANFD0_INT_IRQn);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    uart_receive_start();
    

    DL_GPIO_setPins(LED_PORT, LED_LED_PIN_PIN);


    
    

    uint8_t t=' ';
    while (1)
    {
        // OLED_ShowChinese(0,0,0,16);//中
        // OLED_ShowChinese(18,0,1,16);//景
        // OLED_ShowChinese(36,0,2,16);//园
        // OLED_ShowChinese(54,0,3,16);//电
        // OLED_ShowChinese(72,0,4,16);//子
        // OLED_ShowChinese(90,0,5,16);//科
        // OLED_ShowChinese(108,0,6,16);//技
        // OLED_ShowString(8,2,(uint8_t *)"ZHONGJINGYUAN",16);
        // OLED_ShowString(20,4,(uint8_t *)"2014/05/01",16);
        // OLED_ShowString(0,6,(uint8_t *)"ASCII:",16);  
        // OLED_ShowString(63,6,(uint8_t *)"CODE:",16);
        // OLED_ShowChar(48,6,t,16);
        // t++;
        // if(t>'~')t=' ';
        // OLED_ShowNum(103,6,t,3,16);
        // delay_ms(500);
        // OLED_Clear();

        if (g_flag_100hz) {
            g_flag_100hz = 0;
            one_hundured_ms_callback();

            
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

    // speed_fr = motor_get_speed(MOTOR_FR);
    // speed_fl = motor_get_speed(MOTOR_FL);
    // speed_rr = motor_get_speed(MOTOR_RR);
    // speed_rl = motor_get_speed(MOTOR_RL);
    ms_callback();
}

int mspm0_delay_ms(unsigned long num_ms)
{
    delay_cycles(80000* num_ms);
}


int mspm0_get_clock_ms(unsigned long *count)
{
    if (!count)
        return 1;
    count[0] = g_tick;
    return 0;
}

