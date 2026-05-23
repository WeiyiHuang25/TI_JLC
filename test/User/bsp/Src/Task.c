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

/* 超声波实时距离（mm），由 one_hundured_ms_callback 更新 */
volatile uint32_t g_us_dist0 = 0;
volatile uint32_t g_us_dist1 = 0;
volatile uint32_t g_us_pulse0 = 0;
volatile uint32_t g_us_pulse1 = 0;

uint32_t x_origin, y_origin;




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

/* 超声波采样滤波：取 n 个样本，剔除偏离均值 >100mm 的野点后取平均 */
static uint32_t filter_average(uint32_t *s, uint8_t n)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < n; i++) sum += s[i];
    uint32_t mean = sum / n;

    sum = 0;
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < n; i++) {
        int32_t diff = (int32_t)s[i] - (int32_t)mean;
        if (diff < 0) diff = -diff;
        if (diff <= 200) { sum += s[i]; cnt++; }
    }
    return (cnt >= 3) ? (sum / cnt) : mean;
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
                RUN_ONCE(once_flag[1], chasis_trapezoid_move(0.5, 0, 0, 0.7, 0.3, 4000));
                if (chasis_trapezoid_done) step = 1;
                break;
            case 1:
                RUN_ONCE(once_flag[2], chasis_trapezoid_move(-0.5, 0, 0, 0.7, 0.3, 4000));
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
                RUN_ONCE(once_flag[3], chasis_trapezoid_move(0, 0.5, 0, 0.7, 0.3, 4000));
                if (chasis_trapezoid_done) step = 1;
                break;
            case 1:
                RUN_ONCE(once_flag[4], chasis_trapezoid_move(0, -0.5, 0, 0.7, 0.3, 4000));
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
            RUN_ONCE(once_flag[1], chasis_set_velocity(0, 0, 3.14));
            RUN_AFTER(once_flag[1], 3000, NULL);
            if(RUN_ONCE_DONE(once_flag[1]))
            {
                RUN_ONCE(once_flag[2], chasis_brake());
                RUN_AFTER(once_flag[2], 100, NULL);
            }
            if(RUN_ONCE_DONE(once_flag[2]))
            {
                RUN_ONCE(once_flag[3], chasis_set_velocity(0, 0, -3.14));
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
    case ULTRASONIC_DEBUG:
        {
            OLED_Clear();
            OLED_ShowString(0, 0, (uint8_t *)"US Dist(mm)", 16);
            OLED_ShowString(0, 2, (uint8_t *)"CH0:", 16);
            if (g_us_dist0 == ULTRASONIC_INVALID_MM) {
                OLED_ShowString(40, 2, (uint8_t *)"----", 16);
            } else {
                OLED_ShowNum(40, 2, g_us_dist0, 4, 16);
            }
            OLED_ShowString(0, 4, (uint8_t *)"CH1:", 16);
            if (g_us_dist1 == ULTRASONIC_INVALID_MM) {
                OLED_ShowString(40, 4, (uint8_t *)"----", 16);
            } else {
                OLED_ShowNum(40, 4, g_us_dist1, 4, 16);
            }
            OLED_ShowString(0, 6, (uint8_t *)"<- Exit", 16);
            if (flag_enter) { flag_enter = 0; Task_Done(); }
        }break;
    case Q2_TASK1:
        {
            RUN_ONCE(once_flag[9], gimbal_return_zero());
            RUN_AFTER(once_flag[9], 1, NULL);
            uint8_t q2_task1_start_buffer[2] = {Q2_TASK1, 0xFF};
            RUN_ONCE(once_flag[0], uart_send(q2_task1_start_buffer, sizeof(q2_task1_start_buffer)));
            RUN_AFTER(once_flag[0], 1, NULL);
            if (RUN_ONCE_DONE(once_flag[0]))
            {
                uint8_t q2_task1_to_barrier_buffer[2] = {0x01, 0x1B};
                RUN_ONCE(once_flag[1], chasis_trapezoid_move(0.5, 0, 0, 0.7, 0.3, Q2_TASK1_TIME_0));
                RUN_AFTER(once_flag[1], Q2_TASK1_TIME_0, uart1_send(q2_task1_to_barrier_buffer, sizeof(q2_task1_to_barrier_buffer)));
            }
        }break;
    case Q2_TASK2:
        {
            uint8_t q2_task2_start_buffer[2] = {Q2_TASK2, 0xFF};
            RUN_ONCE(once_flag[0], uart_send(q2_task2_start_buffer, sizeof(q2_task2_start_buffer)));
            RUN_AFTER(once_flag[0], 1, NULL);
        }break;
    case Q1_TASK1:
        {
            static uint8_t step = 0;
            switch (step) {
            case 0: /* Enable motor */
                RUN_ONCE(once_flag[7], gimbal_enable());
                RUN_AFTER(once_flag[7], 1, NULL);
                if (!RUN_ONCE_DONE(once_flag[7])) break;
                step = 1;
                /* fall through */
            case 1: /* Return zero + wait */
                RUN_ONCE(once_flag[8], gimbal_return_zero());
                RUN_AFTER(once_flag[8], 1000, NULL);
                if (RUN_ONCE_DONE(once_flag[8])) step = 2;
                break;
            case 2: /* +X */
                RUN_ONCE(once_flag[0], chasis_trapezoid_move(Q1_VX, 0, 0, Q1_ACCEL_X, Q1_DECEL_X, Q1_TASK1_TIME_X));
                RUN_AFTER(once_flag[0], Q1_TASK1_TIME_X, NULL);
                if (RUN_ONCE_DONE(once_flag[0])) step = 3;
                break;
            case 3: /* Pause X→Y */
                RUN_AFTER(once_flag[4], Q1_TASK1_PAUSE_1, NULL);
                if (!RUN_ONCE_DONE(once_flag[4])) { RUN_ONCE(once_flag[4], (void)0); break; }
                step = 4;
                break;
            case 4: /* +Y */
                RUN_ONCE(once_flag[1], chasis_trapezoid_move(0, Q1_VY, 0, Q1_ACCEL_Y, Q1_DECEL_Y, Q1_TASK1_TIME_Y));
                RUN_AFTER(once_flag[1], Q1_TASK1_TIME_Y, NULL);
                if (RUN_ONCE_DONE(once_flag[1])) step = 5;
                break;
            case 5: /* Pause Y→Wz */
                RUN_AFTER(once_flag[5], Q1_TASK1_PAUSE_2, NULL);
                if (!RUN_ONCE_DONE(once_flag[5])) { RUN_ONCE(once_flag[5], (void)0); break; }
                step = 6;
                break;
            case 6: /* +Wz */
                RUN_ONCE(once_flag[2], chasis_set_velocity(0, 0, Q1_VWZ));
                RUN_AFTER(once_flag[2], Q1_TASK1_TIME_WZ, chasis_brake());
                if (RUN_ONCE_DONE(once_flag[2])) step = 7;
                break;
            case 7:
                step = 0;
                Task_Done();
                break;
            }
        }break;
    case Q1_TASK2:
        {
            static uint8_t step = 0;
            switch (step) {
            case 0: /* Enable motor */
                RUN_ONCE(once_flag[7], gimbal_enable());
                RUN_AFTER(once_flag[7], 1, NULL);
                if (!RUN_ONCE_DONE(once_flag[7])) break;
                step = 1;
                /* fall through */
            case 1: /* Return zero + wait */
                RUN_ONCE(once_flag[8], gimbal_return_zero());
                RUN_AFTER(once_flag[8], 1000, NULL);
                if (RUN_ONCE_DONE(once_flag[8])) step = 2;
                break;
            case 2: /* +X */
                RUN_ONCE(once_flag[0], chasis_trapezoid_move(Q1_VX, 0, 0, Q1_ACCEL_X, Q1_DECEL_X, Q1_TASK2_TIME_X));
                RUN_AFTER(once_flag[0], Q1_TASK2_TIME_X, NULL);
                if (RUN_ONCE_DONE(once_flag[0])) step = 3;
                break;
            case 3: /* Pause X→Y */
                RUN_AFTER(once_flag[4], Q1_TASK2_PAUSE_1, NULL);
                if (!RUN_ONCE_DONE(once_flag[4])) { RUN_ONCE(once_flag[4], (void)0); break; }
                step = 4;
                break;
            case 4: /* +Y */
                RUN_ONCE(once_flag[1], chasis_trapezoid_move(0, Q1_VY, 0, Q1_ACCEL_Y, Q1_DECEL_Y, Q1_TASK2_TIME_Y));
                RUN_AFTER(once_flag[1], Q1_TASK2_TIME_Y, NULL);
                if (RUN_ONCE_DONE(once_flag[1])) step = 5;
                break;
            case 5: /* Pause Y→Wz */
                RUN_AFTER(once_flag[5], Q1_TASK2_PAUSE_2, NULL);
                if (!RUN_ONCE_DONE(once_flag[5])) { RUN_ONCE(once_flag[5], (void)0); break; }
                step = 6;
                break;
            case 6: /* +Wz */
                RUN_ONCE(once_flag[2], chasis_set_velocity(0, 0, Q1_VWZ));
                RUN_AFTER(once_flag[2], Q1_TASK2_TIME_WZ, chasis_brake());
                if (RUN_ONCE_DONE(once_flag[2])) step = 7;
                break;
            case 7:
                step = 0;
                Task_Done();
                break;
            }
        }break;
    case Q1_TASK3:
        {
            static uint8_t step = 0;
            switch (step) {
            case 0: /* Enable motor */
                RUN_ONCE(once_flag[7], gimbal_enable());
                RUN_AFTER(once_flag[7], 1, NULL);
                if (!RUN_ONCE_DONE(once_flag[7])) break;
                step = 1;
                /* fall through */
            case 1: /* Return zero + wait */
                RUN_ONCE(once_flag[8], gimbal_return_zero());
                RUN_AFTER(once_flag[8], 1000, NULL);
                if (RUN_ONCE_DONE(once_flag[8])) step = 2;
                break;
            case 2: /* +X */
                RUN_ONCE(once_flag[0], chasis_trapezoid_move(Q1_VX, 0, 0, Q1_ACCEL_X, Q1_DECEL_X, Q1_TASK3_TIME_X));
                RUN_AFTER(once_flag[0], Q1_TASK3_TIME_X, NULL);
                if (RUN_ONCE_DONE(once_flag[0])) step = 3;
                break;
            case 3: /* Pause X→Y */
                RUN_AFTER(once_flag[4], Q1_TASK3_PAUSE_1, NULL);
                if (!RUN_ONCE_DONE(once_flag[4])) { RUN_ONCE(once_flag[4], (void)0); break; }
                step = 4;
                break;
            case 4: /* +Y */
                RUN_ONCE(once_flag[1], chasis_trapezoid_move(0, Q1_VY, 0, Q1_ACCEL_Y, Q1_DECEL_Y, Q1_TASK3_TIME_Y));
                RUN_AFTER(once_flag[1], Q1_TASK3_TIME_Y, NULL);
                if (RUN_ONCE_DONE(once_flag[1])) step = 5;
                break;
            case 5: /* Pause Y→Wz */
                RUN_AFTER(once_flag[5], Q1_TASK3_PAUSE_2, NULL);
                if (!RUN_ONCE_DONE(once_flag[5])) { RUN_ONCE(once_flag[5], (void)0); break; }
                step = 6;
                break;
            case 6: /* +Wz */
                RUN_ONCE(once_flag[2], chasis_set_velocity(0, 0, Q1_VWZ));
                RUN_AFTER(once_flag[2], Q1_TASK3_TIME_WZ, chasis_brake());
                if (RUN_ONCE_DONE(once_flag[2])) step = 7;
                break;
            case 7:
                step = 0;
                Task_Done();
                break;
            }
        }break;
    case US_SET_ORIGIN:
        {
            static uint8_t  sample_idx = 0;
            static uint32_t samples0[5], samples1[5];
            static uint32_t last_d0 = 0xFFFFFFFF, last_d1 = 0xFFFFFFFF;
            static uint8_t  last_idx = 0xFF;
            static bool     done = false;

            if (!done && sample_idx == 0) {
                OLED_Clear();
                OLED_ShowString(0, 0, (uint8_t *)"Set Origin", 16);
            }

            if (!done && sample_idx < 5) {
                /* 检测新数据：值变化即代表一次新测量 */
                if (g_us_dist0 != ULTRASONIC_INVALID_MM &&
                    g_us_dist1 != ULTRASONIC_INVALID_MM &&
                    (g_us_dist0 != last_d0 || g_us_dist1 != last_d1)) {
                    last_d0 = g_us_dist0;
                    last_d1 = g_us_dist1;
                    samples0[sample_idx] = g_us_dist0;
                    samples1[sample_idx] = g_us_dist1;
                    sample_idx++;
                }
                /* 只在计数变化时刷新 OLED，避免 I2C 拖慢循环 */
                if (sample_idx != last_idx) {
                    last_idx = sample_idx;
                    OLED_ShowString(0, 2, (uint8_t *)"Sample:", 16);
                    OLED_ShowNum(64, 2, sample_idx, 1, 16);
                }
            }

            if (!done && sample_idx >= 5) {
                /* 右手系: X前 Y左 Z上 → CH0=前方=+X, CH1=右方=-Y */
                x_origin = filter_average(samples0, 5);
                y_origin = filter_average(samples1, 5);
                done = true;
                OLED_Clear();
                OLED_ShowString(0, 0, (uint8_t *)"Origin Set", 16);
                OLED_ShowString(0, 2, (uint8_t *)"X:", 16);
                OLED_ShowNum(24, 2, x_origin, 4, 16);
                OLED_ShowString(0, 4, (uint8_t *)"Y:", 16);
                OLED_ShowNum(24, 4, y_origin, 4, 16);
                OLED_ShowString(0, 6, (uint8_t *)"<- OK", 16);
            }

            if (done && flag_enter) {
                flag_enter = 0;
                sample_idx = 0;
                last_idx = 0xFF;
                last_d0 = 0xFFFFFFFF;
                last_d1 = 0xFFFFFFFF;
                done = false;
                Task_Done();
            }
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
        g_us_dist0  = Ultrasonic_GetDistance_mm(0);
        g_us_pulse0 = Ultrasonic_GetPulseWidth_us(0);
    }
    if (Ultrasonic_IsDataReady(1)) {
        g_us_dist1  = Ultrasonic_GetDistance_mm(1);
        g_us_pulse1 = Ultrasonic_GetPulseWidth_us(1);
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
    case Q2_TASK1:
    {
        if (RUN_ONCE_DONE(once_flag[1]))
        {
            if (uart_rx_buff[0] == Q2_TASK1)
            {
                if (uart_rx_buff[1] == false)
                {
                    RUN_ONCE(once_flag[2], chasis_trapezoid_move(0, 0.5, 0, 0.7, 0.3, Q2_TASK1_TIME_1));
                    RUN_AFTER(once_flag[2], 1, NULL);
                }
                else
                {
                    chasis_brake();
                    RUN_ONCE(once_flag[3], chasis_trapezoid_move(0.5, 0, 0, 0.7, 0.3, Q2_TASK1_TIME_2));
                    RUN_AFTER(once_flag[3], Q2_TASK1_TIME_2,Task_Jump(Q2_TASK2));
                }
            }
        }
    }break;
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
