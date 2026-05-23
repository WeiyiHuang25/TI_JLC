#include "Task.h"
#include "task_scheduler.h"
#include "EzTuner.h"
#include <math.h>
#include <stdlib.h>
#include "Task.h"
#include "gimbal.h"
#include <string.h>
#include "bsp_UART.h"
#include "bsp_UART1.h"
#include "wit_c_sdk.h"
#include "JY901.h"
#include "ultrasonic.h"

void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg);
void one_hundured_ms_callback();
void ms_callback();

void chasis_move_done_callback();
void UART_Rx_DMA_ToIdle_Callback(uint16_t size);
uint32_t system_mode = TASK_INIT;
extern uint32_t g_tick;
extern bool init_ok;






/* =========================================================================
 * 初始�?
 * ========================================================================= */
static once_ctx_t once_flag[10];

void User_Init()
{
    KEY_Init();
    Menu_Init();
    JY901_Init();
}

/* OLED 显示带符号整�?*/
static void _show_signed(uint8_t x, uint8_t y, int32_t val, uint8_t sizey)
{
    if (val < 0) {
        OLED_ShowString(x, y, (uint8_t *)"-", sizey);
        OLED_ShowNum(x + 8, y, (uint32_t)(-val), 4, sizey);
    } else {
        OLED_ShowNum(x, y, (uint32_t)val, 5, sizey);
    }
}

void while_task(void)
{
    switch (system_mode)
    {
    case GIMBAL_SET_ZERO:
        {
            gimbal_set_zero();
            Task_Done();
        }break;
        case CHASIS_X_TEST:
        {
            static uint8_t step = 0;
            switch (step) {
            case 0:
                RUN_ONCE(once_flag[1], chasis_trapezoid_move(0.5, 0, 0, 0.5, 4000));
                if (chasis_trapezoid_done) step = 1;
                break;
            case 1:
                RUN_ONCE(once_flag[2], chasis_trapezoid_move(-0.5, 0, 0, 0.5, 4000));
                if (chasis_trapezoid_done) step = 2;
                break;
            case 2:
                step = 0;
                Task_Done();
                break;
            }
        }break;

    case CHASIS_Y_TEST:
        {
            static uint8_t step = 0;
            switch (step) {
            case 0:
                RUN_ONCE(once_flag[3], chasis_trapezoid_move(0, 0.5, 0, 0.5, 4000));
                if (chasis_trapezoid_done) step = 1;
                break;
            case 1:
                RUN_ONCE(once_flag[4], chasis_trapezoid_move(0, -0.5, 0, 0.5, 4000));
                if (chasis_trapezoid_done) step = 2;
                break;
            case 2:
                step = 0;
                Task_Done();
                break;
            }
        }break;

    case CHASIS_WZ_TEST:
        {
            RUN_ONCE(once_flag[1], chasis_set_velocity(0, 0, 3.14/6));
            RUN_AFTER(once_flag[1], 3000, NULL);
            if(RUN_ONCE_DONE(once_flag[1]))
            {
                RUN_ONCE(once_flag[2], chasis_brake());
                RUN_AFTER(once_flag[2], 100, NULL);
            }
            if(RUN_ONCE_DONE(once_flag[2]))
            {
                RUN_ONCE(once_flag[3], chasis_set_velocity(0, 0, -3.14/6));
                RUN_AFTER(once_flag[3], 3000, chasis_brake());
            }
            if(RUN_ONCE_DONE(once_flag[3]))
            {
                Task_Done();
            }  
        }break;
    
    case GIMBAL_RETURN_ZERO:
        {
            gimbal_return_zero();
            Task_Done();
        }break;
    case GIMBAL_ENABLE:
        {
            gimbal_enable();
            Task_Done();
        }break;
    case GIMBAL_DISABLE:
        {
            gimbal_disable();
            Task_Done();
        }break;
    case GIMBAL_CONNECT_TEST:
        {
            RUN_ONCE(once_flag[1], gimbal_enable());
            RUN_AFTER(once_flag[1], 5,NULL);
        }break;
    case GIMBAL_MOVE_TEST:
        {
            static uint8_t step = 0;
            /* 256细分, 51200脉冲/�? 10°=1422脉冲 */
            switch (step) {
            case 0:
                RUN_ONCE(once_flag[1], gimbal_enable());
                RUN_AFTER(once_flag[1], 100, NULL);
                if (RUN_ONCE_DONE(once_flag[1])) step = 1;
                break;
            case 1: /* Pitch CCW 10° */
                RUN_ONCE(once_flag[2], gimbal_update(1, 500, 1422, 0, 0, 0));
                RUN_AFTER(once_flag[2], 2000, NULL);
                if (RUN_ONCE_DONE(once_flag[2])) step = 2;
                break;
            case 2: /* Pitch CW 10° */
                RUN_ONCE(once_flag[3], gimbal_update(0, 500, 1422, 0, 0, 0));
                RUN_AFTER(once_flag[3], 2000, NULL);
                if (RUN_ONCE_DONE(once_flag[3])) step = 3;
                break;
            case 3: /* Yaw CCW 10° */
                RUN_ONCE(once_flag[4], gimbal_update(0, 0, 0, 1, 500, 1422));
                RUN_AFTER(once_flag[4], 2000, NULL);
                if (RUN_ONCE_DONE(once_flag[4])) step = 4;
                break;
            case 4: /* Yaw CW 10° */
                RUN_ONCE(once_flag[5], gimbal_update(0, 0, 0, 0, 500, 1422));
                RUN_AFTER(once_flag[5], 2000, NULL);
                if (RUN_ONCE_DONE(once_flag[5])) step = 5;
                break;
            case 5: /* Done */
                gimbal_disable();
                step = 0;
                Task_Done();
                break;
            }
        }break;
    case GYRO_ACC:
        {
            OLED_Clear();
            OLED_ShowString(0, 0, (uint8_t *)"Yaw", 16);
            _show_signed(0,  2, (int32_t)(jy901_yaw * 100), 16);
            if (flag_enter) { flag_enter = 0; Task_Done(); }
        }break;
    default:
        break;
    }    
}


inline void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg)
{
    switch (system_mode)
    {
    case GIMBAL_SET_ZERO:
        {

        }break;
        case CHASIS_X_TEST:
        {

        }break;

    case CHASIS_Y_TEST:
        {

        }break;

    case CHASIS_WZ_TEST:
        {

        }break;
    case GIMBAL_RETURN_ZERO:
        {

        }break;
    case GIMBAL_ENABLE:
        {

        }break;
    case GIMBAL_DISABLE:
        {

        }break;
    case GIMBAL_CONNECT_TEST:
        {

        }
    case GIMBAL_MOVE_TEST:
        { }break;
    case GYRO_ACC:
    case GYRO_VEL:
    case GYRO_POS:
        { }break;
    default:
        break;
    }
}


inline void one_hundured_ms_callback()
{
    if (!init_ok)
        return;

    /* ---- 超声波每 100ms 触发+读取 ---- */
    static uint8_t us_cnt = 0;
    if (++us_cnt >= 10) {  /* 100Hz / 10 = 10Hz = 100ms */
        us_cnt = 0;
        Ultrasonic_Trigger(0);
        Ultrasonic_Trigger(1);
    }
    /* 读取上次结果（ISR 中已算好，非阻塞） */
    if (Ultrasonic_IsDataReady(0)) {
        uint32_t d0 = Ultrasonic_GetDistance_mm(0);  /* mm */
        (void)d0;  /* TODO: 用 d0 */
    }
    if (Ultrasonic_IsDataReady(1)) {
        uint32_t d1 = Ultrasonic_GetDistance_mm(1);  /* mm */
        (void)d1;  /* TODO: 用 d1 */
    }

    chasis_cal();
    JY901_Update();
    switch (system_mode)
    {
    case GIMBAL_SET_ZERO:
        {

        }break;
        case CHASIS_X_TEST:
        {

        }break;

    case CHASIS_Y_TEST:
        {

        }break;

    case CHASIS_WZ_TEST:
        {

        }break;
    case GIMBAL_RETURN_ZERO:
        {

        }break;
    case GIMBAL_ENABLE:
        {

        }break;
    case GIMBAL_DISABLE:
        {

        }break;
    case GIMBAL_CONNECT_TEST:
        {
            
        }
    case GIMBAL_MOVE_TEST:
        { }break;
    case GYRO_ACC:
    case GYRO_VEL:
    case GYRO_POS:
        { }break;
    default:
        break;
    }
}


inline void ms_callback()
{
    if (!init_ok)
        return;
    KEY_Poll();
    if (system_mode == TASK_INIT) {
        Menu_Run();
    }
    switch (system_mode)
    {
    case GIMBAL_SET_ZERO:
        {

        }break;
    case CHASIS_WZ_TEST:
        {

        }break;
    case GIMBAL_RETURN_ZERO:
        {

        }break;
    case GIMBAL_ENABLE:
        {

        }break;
    case GIMBAL_DISABLE:
        {

        }break;
    case GIMBAL_CONNECT_TEST:
        {
            
        }
    case GIMBAL_MOVE_TEST:
        { }break;

    case GYRO_ACC:
    case GYRO_VEL:
    case GYRO_POS:
        { }break;

    default:
        break;
    }

}



void chasis_move_done_callback()
{
    switch (system_mode)
    {
    case GIMBAL_SET_ZERO:
        {

        }break;
        case CHASIS_X_TEST:
        {
            
        }break;
    case CHASIS_Y_TEST:
        {

        }break;
    case CHASIS_WZ_TEST:
        {

        }break;
    case GIMBAL_RETURN_ZERO:
        {

        }break;
    case GIMBAL_ENABLE:
        {

        }break;
    case GIMBAL_DISABLE:
        {

        }break;
    case GIMBAL_CONNECT_TEST:
        {
            
        }
    case GIMBAL_MOVE_TEST:
        { }break;
    case GYRO_ACC:
    case GYRO_VEL:
    case GYRO_POS:
        { }break;
    default:
        break;
    }
}


void UART_Rx_DMA_ToIdle_Callback(uint16_t size)
{
    switch (system_mode)
    {
    case GIMBAL_SET_ZERO:
        {

        }break;
        case CHASIS_X_TEST:
        {

        }break;

    case CHASIS_Y_TEST:
        {

        }break;

    case CHASIS_WZ_TEST:
        {

        }break;
    case GIMBAL_RETURN_ZERO:
        {

        }break;
    case GIMBAL_ENABLE:
        {

        }break;
    case GIMBAL_DISABLE:
        {

        }break;
    case GIMBAL_CONNECT_TEST:
        {
            uint8_t pitch_dir, yaw_dir;
            uint16_t pitch_vel, yaw_vel;
            uint32_t pitch_clk, yaw_clk;
            
            pitch_dir = uart_rx_buff[0];
            pitch_vel = uart_rx_buff[1] | ((uint16_t)uart_rx_buff[2] << 8);
            pitch_clk = uart_rx_buff[3] | ((uint32_t)uart_rx_buff[4] << 8)
                      | ((uint32_t)uart_rx_buff[5] << 16) | ((uint32_t)uart_rx_buff[6] << 24);
            yaw_dir   = uart_rx_buff[7];
            yaw_vel   = uart_rx_buff[8] | ((uint16_t)uart_rx_buff[9] << 8);
            yaw_clk   = uart_rx_buff[10] | ((uint32_t)uart_rx_buff[11] << 8)
                      | ((uint32_t)uart_rx_buff[12] << 16) | ((uint32_t)uart_rx_buff[13] << 24);
            if(RUN_ONCE_DONE(once_flag[1]))
            {
                gimbal_update(pitch_dir, pitch_vel, pitch_clk, yaw_dir, yaw_vel, yaw_clk);
            }
        }
    case GIMBAL_MOVE_TEST:
        { }break;
    case GYRO_ACC:
    case GYRO_VEL:
    case GYRO_POS:
        { }break;
    default:
        break;
    }
}

/* UART1 �?测试回显 (�?bsp_UART1.c)，测试完成后取消注释 */
#if 1
void UART1_Rx_DMA_ToIdle_Callback(uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        WitSerialDataIn(uart1_rx_buff[i]);
    }
}
#endif


void Task_Cleanup(void)
{
    for (uint8_t i = 0; i < sizeof(once_flag) / sizeof(once_flag[0]); i++) {
        once_flag[i] = (once_ctx_t){0};
    }
}

void Key_A18_Pressed(void) 
{
    flag_next = 1;
}
void Key_B21_Pressed(void) 
{
    flag_enter = 1;
}
