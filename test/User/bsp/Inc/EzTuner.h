#pragma once


#define INIT_TIME 1000 // ms


/* =========================================================================
 * 开环控制 — 每轮 PWM 比例 (1.0 = 标准, 调大=更快, 调小=更慢)
 * 公式: duty = |exp_rpm| / OPENLOOP_MAX_RPM * RATIO * PWM_PERIOD
 * ========================================================================= */
#define OPENLOOP_MAX_RPM        282.0f   /* 满占空比对应的 RPM */
#define OPENLOOP_FL_RATIO       1.0f     /* FL 前左轮 PWM 比例 */
#define OPENLOOP_FR_RATIO       1.0f     /* FR 前右轮 PWM 比例 */
#define OPENLOOP_RL_RATIO       1.0f     /* RL 后左轮 PWM 比例 */
#define OPENLOOP_RR_RATIO       1.0f     /* RR 后右轮 PWM 比例 */
#define OPENLOOP_MIN_DUTY       300      /* 最小启动占空比 (克服静摩擦) */

/* =========================================================================
 * 麦轮各轮方向修正系数 (1.0=不变, >1 加速, <1 减速)
 * POS = 正转(从车轮自身视角), NEG = 反转
 * 前进时四轮均为 POS, 后退均为 NEG
 * 左移时 FL/RR 为 NEG, FR/RL 为 POS
 * 右移时 FL/RR 为 POS, FR/RL 为 NEG
 * ========================================================================= */
#define CHASIS_FL_POS_SCALE  1.0f
#define CHASIS_FL_NEG_SCALE  1.0f
#define CHASIS_FR_POS_SCALE  1.0f
#define CHASIS_FR_NEG_SCALE  1.0f
#define CHASIS_RL_POS_SCALE  1.0f
#define CHASIS_RL_NEG_SCALE  1.0f
#define CHASIS_RR_POS_SCALE  1.0f
#define CHASIS_RR_NEG_SCALE  1.0f

/* =========================================================================
 * 运动方向级 — 各轮比例修正（FL, FR, RL, RR 顺序）
 *   FWD  = 前进(vx>0)  BWD  = 后退(vx<0)
 *   LEFT = 左移(vy>0)  RIGHT= 右移(vy<0)
 * 优先级: 先应用 POS/NEG_SCALE, 再乘此系数
 * 调法: 某方向偏哪边，就降那侧电机的值
 * ========================================================================= */
 //                        FL    FR    RL    RR
#define MEC_FWD_SCALE   { 1.0f, 1.0f, 1.0f, 1.0f }   /* 前进 */
#define MEC_BWD_SCALE   { 1.0f, 1.0f, 1.0f, 1.0f }   /* 后退 */
#define MEC_LEFT_SCALE  { 1.0f, 1.2f, 1.0f, 1.0f }   /* 左移 */
#define MEC_RIGHT_SCALE { 1.0f, 1.0f, 1.0f, 1.0f }   /* 右移 */



#define Q2_TASK1_VELOCITY       0.5f       //平移避障的时间
#define Q2_TASK1_TIME_0           1000U   // 到障碍区前的时间  OK
#define Q2_TASK1_TIME_1           3000U   // 向左平移的最长时间
#define Q2_TASK1_TIME_2         1000U  //通过障碍区的时间

/* Q1: X→Y→Wz 三步任务，每步时间独立可调 */
#define Q1_TASK1_TIME_X           2700U
#define Q1_TASK1_TIME_Y           3700U
#define Q1_TASK1_TIME_WZ          400U

#define Q1_TASK2_TIME_X           2000U
#define Q1_TASK2_TIME_Y           2000U
#define Q1_TASK2_TIME_WZ          2000U

#define Q1_TASK3_TIME_X           3000U
#define Q1_TASK3_TIME_Y           3000U
#define Q1_TASK3_TIME_WZ          3000U

/* Q1: 运动段间延时（归零后直接走，X→Y、Y→Wz 间可调） */
#define Q1_TASK1_PAUSE_1          500U
#define Q1_TASK1_PAUSE_2          500U
#define Q1_TASK2_PAUSE_1          500U
#define Q1_TASK2_PAUSE_2          500U
#define Q1_TASK3_PAUSE_1          500U
#define Q1_TASK3_PAUSE_2          500U

/* Q1: 底盘运动参数（速度/加减速，XY 独立） */
#define Q1_VX                   0.5f
#define Q1_VY                   0.5f
#define Q1_VWZ                 3.14f
#define Q1_ACCEL_X             0.7f
#define Q1_DECEL_X             0.3f
#define Q1_ACCEL_Y             0.7f
#define Q1_DECEL_Y             0.3f


// #define FL_PID_KP 1.0f
// #define FL_PID_KI 0.01f
// #define FL_PID_KD 0.005f
// #define FL_PID_I_MAX 1000.0f
// #define FL_PID_O_MAX 4999
// #define FL_PID_DEADBAND     50      /* 死区: |output| < 50 不驱动 */
// #define FL_PID_DIR_FLIP_THR 4       /* 方向翻转需连续 4 周期 (40ms) */

// #define FR_PID_KP 1.0f
// #define FR_PID_KI 0.01f
// #define FR_PID_KD 0.005f
// #define FR_PID_I_MAX 1000.0f
// #define FR_PID_O_MAX 4999
// #define FR_PID_DEADBAND     50
// #define FR_PID_DIR_FLIP_THR 4

// #define RL_PID_KP 1.0f
// #define RL_PID_KI 0.01f
// #define RL_PID_KD 0.005f
// #define RL_PID_I_MAX 1000.0f
// #define RL_PID_O_MAX 4999
// #define RL_PID_DEADBAND     50
// #define RL_PID_DIR_FLIP_THR 4

// #define RR_PID_KP 1.0f
// #define RR_PID_KI 0.01f
// #define RR_PID_KD 0.005f
// #define RR_PID_I_MAX 1000.0f
// #define RR_PID_O_MAX 4999
// #define RR_PID_DEADBAND     50
// #define RR_PID_DIR_FLIP_THR 4

// /* ── 刹车/惰行阈值 (全局) ── */
// #define BRAKE_SPEED_THR     5.0f    /* 低于此速度(RPM)时短路刹车, 高于时PID减速 */



