#pragma once


#define INIT_TIME 1000 // ms


#define FL_PID_KP 0.1f
#define FL_PID_KI 0.01f
#define FL_PID_KD 0.005f
#define FL_PID_I_MAX 1000.0f
#define FL_PID_O_MAX 4999
#define FL_PID_DEADBAND     50      /* 死区: |output| < 50 不驱动 */
#define FL_PID_DIR_FLIP_THR 4       /* 方向翻转需连续 4 周期 (40ms) */

#define FR_PID_KP 0.1f
#define FR_PID_KI 0.01f
#define FR_PID_KD 0.005f
#define FR_PID_I_MAX 1000.0f
#define FR_PID_O_MAX 4999
#define FR_PID_DEADBAND     50
#define FR_PID_DIR_FLIP_THR 4

#define RL_PID_KP 0.1f
#define RL_PID_KI 0.01f
#define RL_PID_KD 0.005f
#define RL_PID_I_MAX 1000.0f
#define RL_PID_O_MAX 4999
#define RL_PID_DEADBAND     50
#define RL_PID_DIR_FLIP_THR 4

#define RR_PID_KP 0.1f
#define RR_PID_KI 0.01f
#define RR_PID_KD 0.005f
#define RR_PID_I_MAX 1000.0f
#define RR_PID_O_MAX 4999
#define RR_PID_DEADBAND     50
#define RR_PID_DIR_FLIP_THR 4

/* ── 刹车/惰行阈值 (全局) ── */
#define BRAKE_SPEED_THR     5.0f    /* 低于此速度(RPM)时短路刹车, 高于时PID减速 */



