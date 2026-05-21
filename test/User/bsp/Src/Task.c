#include "Task.h"
#include "EzTuner.h"
#include <math.h>
#include <stdlib.h>



void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg);
void one_hundured_ms_callback();
void ms_callback();
void UART_Rx_DMA_ToIdle_Callback(uint16_t size);
void chasis_move_done_callback();
uint32_t system_mode = TASK_INIT;

extern bool init_ok;

/* =========================================================================
 * 初始化
 * ========================================================================= */
void User_Init()
{

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

/* =========================================================================
 * 1ms 周期: 传感器刷�?+ 正向运动�?+ 里程计积�?
 * ========================================================================= */
inline void ms_callback()
{
    if (!init_ok)
        return;
    switch (system_mode)
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
    
}