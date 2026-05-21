#include "Task.h"
#include "EzTuner.h"
#include <math.h>
#include <stdlib.h>



void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg);
void one_hundured_ms_callback();
void ms_callback();
void UART_Rx_DMA_ToIdle_Callback(uint16_t size);
void chasis_move_done_callback();


extern bool init_ok;

/* =========================================================================
 * 初始化
 * ========================================================================= */
void User_Init()
{
    /* ── 前左 FL ── */
    pid_init(&user_chasis.FL.pid, FL_PID_KP, FL_PID_KI, FL_PID_KD, FL_PID_I_MAX, FL_PID_O_MAX);
    user_chasis.FL.pid.deadband     = FL_PID_DEADBAND;
    user_chasis.FL.pid.dir_flip_thr = FL_PID_DIR_FLIP_THR;
    user_chasis.FL.id = MOTOR_FL;

    /* ── 前右 FR ── */
    pid_init(&user_chasis.FR.pid, FR_PID_KP, FR_PID_KI, FR_PID_KD, FR_PID_I_MAX, FR_PID_O_MAX);
    user_chasis.FR.pid.deadband     = FR_PID_DEADBAND;
    user_chasis.FR.pid.dir_flip_thr = FR_PID_DIR_FLIP_THR;
    user_chasis.FR.id = MOTOR_FR;

    /* ── 后左 RL ── */
    pid_init(&user_chasis.RL.pid, RL_PID_KP, RL_PID_KI, RL_PID_KD, RL_PID_I_MAX, RL_PID_O_MAX);
    user_chasis.RL.pid.deadband     = RL_PID_DEADBAND;
    user_chasis.RL.pid.dir_flip_thr = RL_PID_DIR_FLIP_THR;
    user_chasis.RL.id = MOTOR_RL;

    /* ── 后右 RR ── */
    pid_init(&user_chasis.RR.pid, RR_PID_KP, RR_PID_KI, RR_PID_KD, RR_PID_I_MAX, RR_PID_O_MAX);
    user_chasis.RR.pid.deadband     = RR_PID_DEADBAND;
    user_chasis.RR.pid.dir_flip_thr = RR_PID_DIR_FLIP_THR;
    user_chasis.RR.id = MOTOR_RR;
}








inline void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg)
{

}


inline void one_hundured_ms_callback()
{
    if (!init_ok)
        return;

    /* 1. 方向测试状态机 */
    motor_test_tick();

    /* 2. 逆运动学 + PID + 电机驱动 */
    chasis_cal();
}

/* =========================================================================
 * 1ms 周期: 传感器刷�?+ 正向运动�?+ 里程计积�?
 * ========================================================================= */
inline void ms_callback()
{
    if (!init_ok)
        return;

    chasis_update();

}



void chasis_move_done_callback()
{
    
}