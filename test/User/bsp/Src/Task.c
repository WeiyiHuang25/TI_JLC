#include "Task.h"
#include "task_scheduler.h"
#include "EzTuner.h"
#include <math.h>
#include <stdlib.h>
#include "Task.h"
#include "gimbal.h"
#include <string.h>
#include "bsp_UART.h"



void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg);
void one_hundured_ms_callback();
void ms_callback();

void chasis_move_done_callback();
void UART_Rx_DMA_ToIdle_Callback(uint16_t size);
uint32_t system_mode = TASK_INIT;
extern uint32_t g_tick;
extern bool init_ok;






/* =========================================================================
 * 初始化
 * ========================================================================= */
static once_ctx_t once_flag[4];

void User_Init()
{
    KEY_Init();
    Menu_Init();
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
            RUN_ONCE(once_flag[1], chasis_set_velocity(0.5, 0, 0));
            RUN_AFTER(once_flag[1], 5000, NULL);
            if(RUN_ONCE_DONE(once_flag[1]))
            {
                RUN_ONCE(once_flag[2], chasis_brake());
                RUN_AFTER(once_flag[2], 100, NULL);
            }
            if(RUN_ONCE_DONE(once_flag[2]))
            {
                RUN_ONCE(once_flag[3], chasis_set_velocity(-0.5, 0, 0));
                RUN_AFTER(once_flag[3], 5000, chasis_brake());                
            }
            if(RUN_ONCE_DONE(once_flag[3]))
            {
                Task_Done();
            }
        }break;
    case CHASIS_Y_TEST:
        {
            RUN_ONCE(once_flag[1], chasis_set_velocity(0, 0.5, 0));
            RUN_AFTER(once_flag[1], 5000, NULL);
            if(RUN_ONCE_DONE(once_flag[1]))
            {
                RUN_ONCE(once_flag[2], chasis_brake());
                RUN_AFTER(once_flag[2], 100, NULL);
            }
            if(RUN_ONCE_DONE(once_flag[2]))
            {
                RUN_ONCE(once_flag[3], chasis_set_velocity(0, -0.5, 0));
                RUN_AFTER(once_flag[3], 5000, chasis_brake());
            }
            if(RUN_ONCE_DONE(once_flag[3]))
            {
                Task_Done();
            }     
        }break;
    case CHASIS_WZ_TEST:
        {
            RUN_ONCE(once_flag[1], chasis_set_velocity(0, 0, 3.14/6));
            RUN_AFTER(once_flag[1], 5000, NULL);
            if(RUN_ONCE_DONE(once_flag[1]))
            {
                RUN_ONCE(once_flag[2], chasis_brake());
                RUN_AFTER(once_flag[2], 100, NULL);
            }
            if(RUN_ONCE_DONE(once_flag[2]))
            {
                RUN_ONCE(once_flag[3], chasis_set_velocity(0, 0, -3.14/6));
                RUN_AFTER(once_flag[3], 5000, chasis_brake());
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
    case GIMBAL_TEST:
        {
            RUN_ONCE(once_flag[1], gimbal_enable());
            RUN_AFTER(once_flag[1], 5,NULL);
        }
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
    case GIMBAL_TEST:
        {

        }
    default:
        break;
    }
}


inline void one_hundured_ms_callback()
{
    if (!init_ok)
        return;
    chasis_cal();
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
    case GIMBAL_TEST:
        {
            
        }
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
    case GIMBAL_TEST:
        {
            
        }
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
    case GIMBAL_TEST:
        {
            
        }
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
    case GIMBAL_TEST:
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
    default:
        break;
    }
}


void Task_Cleanup(void)
{
    for (uint8_t i = 0; i < 4; i++) {
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
