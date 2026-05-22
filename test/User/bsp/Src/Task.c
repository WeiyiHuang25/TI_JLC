#include "Task.h"
#include "EzTuner.h"
#include <math.h>
#include <stdlib.h>



void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg);
void one_hundured_ms_callback();
void ms_callback();

void chasis_move_done_callback();
void UART_Rx_DMA_ToIdle_Callback(uint16_t size);
uint32_t system_mode = TASK_INIT;

extern bool init_ok;

/* =========================================================================
 * 初始化
 * ========================================================================= */
void User_Init()
{
    KEY_Init();
}








inline void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg)
{

}


inline void one_hundured_ms_callback()
{
    if (!init_ok)
        return;
    chasis_cal();
    switch (system_mode)
    {
    case TASK1:
        {

        }break;
    case TASK2:
        {

        }break;
    case TASK3:
        {

        }break;
    case TASK4:
        {

        }break;
    default:
        break;
    }
}


inline void ms_callback()
{
    if (!init_ok)
        return;
    KEY_Poll();
    switch (system_mode)
    {
    case TASK1:
        {

        }break;
    case TASK2:
        {

        }break;
    case TASK3:
        {

        }break;
    case TASK4:
        {

        }break;
    default:
        break;
    }

}



void chasis_move_done_callback()
{
    switch (system_mode)
    {
    case TASK1:
        {

        }break;
    case TASK2:
        {

        }break;
    case TASK3:
        {

        }break;
    case TASK4:
        {

        }break;
    default:
        break;
    }
}


void UART_Rx_DMA_ToIdle_Callback(uint16_t size)
{
    switch (system_mode)
    {
    case TASK1:
        {

        }break;
    case TASK2:
        {

        }break;
    case TASK3:
        {

        }break;
    case TASK4:
        {

        }break;
    default:
        break;
    }
}


void Key_A18_Pressed(void) 
{

}
void Key_B21_Pressed(void) 
{
    
}
