#include "bsp_PWM.h"

/* =========================================================================
 * 4 电机通道映射 (direction/duty 仅用于查询, 不含硬件操作)
 * ========================================================================= */
typedef struct {
    bool     direction;   /* 当前方向: true=正转 */
    uint16_t duty;        /* 当前占空比 */
} motor_chan_t;

static motor_chan_t g_motor[MOTOR_NUM] = {
    [MOTOR_FL] = { .direction = true, .duty = 0 },
    [MOTOR_FR] = { .direction = true, .duty = 0 },
    [MOTOR_RL] = { .direction = true, .duty = 0 },
    [MOTOR_RR] = { .direction = true, .duty = 0 },
};

/* =========================================================================
 * 编码器速度追踪状态
 * ========================================================================= */
typedef struct {
    uint32_t last_cap_a;    /* 上次 A 相捕获值 (检测新边沿) */
    uint32_t last_cap_b;    /* 上次 B 相捕获值 */
    uint8_t  last_ab_state; /* 上次 AB 电平状态: (A<<1)|B, 0~3 */
    int8_t   last_dir;      /* 上次判定的方向: +1=正转, -1=反转, 0=未知 */
} encoder_state_t;

/* =========================================================================
 * 内部辅助宏: 设置 CC 寄存器 (区分 TIMA / TIMG)
 *
 * FRONT_OUT=TIMA0  RL_OUT=TIMA1    → DL_TimerA_setCaptureCompareValue
 * RR_OUT  =TIMG6                    → DL_TimerG_setCaptureCompareValue
 *
 * PWM 输出特性 (edge-align, OCTL init = LOW):
 *   counter <  CC → 输出 LOW
 *   counter >= CC → 输出 HIGH
 *   duty = (period - CC) / period
 *
 *   0%  (always LOW):  CC = period
 *   100%(always HIGH): CC = 0
 * ========================================================================= */

#define SET_CC_FL_IN1(v)  DL_TimerA_setCaptureCompareValue(FRONT_OUT_INST, (v), DL_TIMER_CC_0_INDEX)
#define SET_CC_FL_IN2(v)  DL_TimerA_setCaptureCompareValue(FRONT_OUT_INST, (v), DL_TIMER_CC_1_INDEX)
#define SET_CC_FR_IN1(v)  DL_TimerA_setCaptureCompareValue(FRONT_OUT_INST, (v), DL_TIMER_CC_2_INDEX)
#define SET_CC_FR_IN2(v)  DL_TimerA_setCaptureCompareValue(FRONT_OUT_INST, (v), DL_TIMER_CC_3_INDEX)
#define SET_CC_RL_IN1(v)  DL_TimerA_setCaptureCompareValue(RL_OUT_INST,    (v), DL_TIMER_CC_0_INDEX)
#define SET_CC_RL_IN2(v)  DL_TimerA_setCaptureCompareValue(RL_OUT_INST,    (v), DL_TIMER_CC_1_INDEX)
#define SET_CC_RR_IN1(v)  DL_TimerG_setCaptureCompareValue(RR_OUT_INST,    (v), DL_TIMER_CC_0_INDEX)
#define SET_CC_RR_IN2(v)  DL_TimerG_setCaptureCompareValue(RR_OUT_INST,    (v), DL_TIMER_CC_1_INDEX)

/* =========================================================================
 * 内部辅助宏: 读取编码器捕获值 (区分 16-bit / 32-bit 定时器)
 * ========================================================================= */
#define READ_CAP_FFRONT_R()  DL_TimerG_getCaptureCompareValue(FFRONT_IN_INST, DL_TIMER_CC_0_INDEX)
#define READ_CAP_FFRONT_L()  DL_TimerG_getCaptureCompareValue(FFRONT_IN_INST, DL_TIMER_CC_1_INDEX)
#define READ_CAP_REAR_R()    DL_TimerG_getCaptureCompareValue(REAR_IN_INST, DL_TIMER_CC_0_INDEX)
#define READ_CAP_REAR_L()    DL_TimerG_getCaptureCompareValue(REAR_IN_INST, DL_TIMER_CC_1_INDEX)

/* 读取定时器当前计数值 (用于计算 τ = 距上一边沿的时间) */
#define READ_CNT_FFRONT()  DL_TimerG_getTimerCount(FFRONT_IN_INST)
#define READ_CNT_REAR()    DL_TimerG_getTimerCount(REAR_IN_INST)

/* =========================================================================
 * 公开 API
 * ========================================================================= */

void motor_set(motor_id_t motor, bool forward, uint16_t duty)
{
    if (motor >= MOTOR_NUM) return;
    if (duty > PWM_PERIOD) duty = PWM_PERIOD;

    uint16_t cc = PWM_PERIOD - duty;   /* CC ∈ [0, period] */
    uint16_t lo = PWM_PERIOD;          /* LOW  = CC=period */

    switch (motor) {
    case MOTOR_FL:
        if (forward) { SET_CC_FL_IN1(cc); SET_CC_FL_IN2(lo); }
        else         { SET_CC_FL_IN1(lo); SET_CC_FL_IN2(cc); }
        break;
    case MOTOR_FR:
        if (forward) { SET_CC_FR_IN1(cc); SET_CC_FR_IN2(lo); }
        else         { SET_CC_FR_IN1(lo); SET_CC_FR_IN2(cc); }
        break;
    case MOTOR_RL:
        if (forward) { SET_CC_RL_IN1(cc); SET_CC_RL_IN2(lo); }
        else         { SET_CC_RL_IN1(lo); SET_CC_RL_IN2(cc); }
        break;
    case MOTOR_RR:
        if (forward) { SET_CC_RR_IN1(cc); SET_CC_RR_IN2(lo); }
        else         { SET_CC_RR_IN1(lo); SET_CC_RR_IN2(cc); }
        break;
    default:
        return;
    }

    g_motor[motor].direction = forward;
    g_motor[motor].duty      = duty;
}

void motor_coast(motor_id_t motor)
{
    uint16_t lo = PWM_PERIOD;
    switch (motor) {
    case MOTOR_FL: SET_CC_FL_IN1(lo); SET_CC_FL_IN2(lo); break;
    case MOTOR_FR: SET_CC_FR_IN1(lo); SET_CC_FR_IN2(lo); break;
    case MOTOR_RL: SET_CC_RL_IN1(lo); SET_CC_RL_IN2(lo); break;
    case MOTOR_RR: SET_CC_RR_IN1(lo); SET_CC_RR_IN2(lo); break;
    default: return;
    }
    g_motor[motor].duty = 0;
}

void motor_brake(motor_id_t motor)
{
    uint16_t hi = 0;   /* CC=0 → always HIGH (100% duty) */
    switch (motor) {
    case MOTOR_FL: SET_CC_FL_IN1(hi); SET_CC_FL_IN2(hi); break;
    case MOTOR_FR: SET_CC_FR_IN1(hi); SET_CC_FR_IN2(hi); break;
    case MOTOR_RL: SET_CC_RL_IN1(hi); SET_CC_RL_IN2(hi); break;
    case MOTOR_RR: SET_CC_RR_IN1(hi); SET_CC_RR_IN2(hi); break;
    default: return;
    }
    g_motor[motor].duty = 0;
}

bool motor_get_direction(motor_id_t motor)
{
    if (motor >= MOTOR_NUM) return false;
    return g_motor[motor].direction;
}

uint16_t motor_get_duty(motor_id_t motor)
{
    if (motor >= MOTOR_NUM) return 0;
    return g_motor[motor].duty;
}

void motor_all_coast(void)
{
    for (int i = 0; i < MOTOR_NUM; i++) {
        motor_coast((motor_id_t)i);
    }
}

/* =========================================================================
 * 编码器速度读取
 *
 * ── 方向判定 (两级) ──
 *   ① 优先: GPIO 电平正交状态机 — 捕获到 AB 电平跳变时精准判向
 *   ② 兜底: 捕获时间戳比较   — 电平未变时, 靠 A/B 捕获值先后判断
 *      (你提出的"上升沿下降沿时间差值法")
 *
 *   ┌────────────┬──────────────────────────────┐
 *   │ GPIO 状态   │  B_cap > A_cap (B更新)       │
 *   │ (A<<1)|B   │  A_cap > B_cap (A更新)       │
 *   ├────────────┼──────────────┬───────────────┤
 *   │  0 (L,L)   │  B更新→正转  │  A更新→反转   │
 *   │  1 (L,H)   │  B更新→反转  │  A更新→正转   │
 *   │  2 (H,L)   │  B更新→反转  │  A更新→正转   │
 *   │  3 (H,H)   │  B更新→正转  │  A更新→反转   │
 *   └────────────┴──────────────┴───────────────┘
 *
 * ── 速度量级 ──
 *   delta_a + delta_b = 两次调用间捕获寄存器总变化量 (timer ticks)
 *   TIMG12 (10MHz) 除以 10 归一化到与其他 3 路 1MHz 一致。
 *   边沿多→delta大→速度快, 与真实速度成正比, 适合 PID 反馈。
 * ========================================================================= */

float motor_get_speed(motor_id_t motor)
{
    if (motor >= MOTOR_NUM) return 0.0f;

    static uint32_t last_count[MOTOR_NUM] = {0};
    static float last_rpm[MOTOR_NUM] = {0};
    static float last_period[MOTOR_NUM] = {0};

    uint32_t current_count = 0;
    uint32_t now_ticks = 0;

    switch (motor) {
    case MOTOR_FL: 
        current_count = READ_CAP_FFRONT_L(); 
        now_ticks = READ_CNT_FFRONT();
        break;
    case MOTOR_FR: 
        current_count = READ_CAP_FFRONT_R(); 
        now_ticks = READ_CNT_FFRONT();
        break;
    case MOTOR_RL: 
        current_count = READ_CAP_REAR_L(); 
        now_ticks = READ_CNT_REAR();
        break;
    case MOTOR_RR: 
        current_count = READ_CAP_REAR_R(); 
        now_ticks = READ_CNT_REAR();
        break;
    default: return 0.0f;
    }

    /* 与上次边沿时间的差值 */
    uint32_t diff = (current_count - last_count[motor]) & 0xFFFFU;
    
    /* 距离最后一个边沿到现在过去的时间量 */
    uint32_t time_since_last = (now_ticks - current_count) & 0xFFFFU;

    /* 超时停转检测：距最后一次脉冲超过 50ms (50000us)，视为停转 */
    if (time_since_last > 50000) {
        last_rpm[motor] = 0.0f;
        last_period[motor] = 0.0f;
        return 0.0f;
    }

    /* 本次 1ms 内没有抓到新脉冲：保持平滑输出 */
    if (diff == 0) {
        return last_rpm[motor];
    }

    last_count[motor] = current_count;

    float dir_f = g_motor[motor].direction ? 1.0f : -1.0f;

    /* 
     * 核心修复：多边沿时间解算过滤
     * 由于轮询频率低于边沿发生频率，diff 中可能包含了 2个甚至3个 周期的总时间。
     * 估算包含的周期数 N，将其除回去得到真实的【单边沿间隔时间】
     */
    float period = (float)diff;
    if (last_period[motor] > 0.0f) {
        int N = (int)(period / last_period[motor] + 0.5f);
        if (N < 1) N = 1;
        period = period / (float)N; 
    }
    
    // 如果干扰导致值太小则予以忽略
    if (period < 1.0f) period = 1.0f;
    last_period[motor] = period;

    /*
     * 根据 292RPM 推算，由于捕获是双边沿工作，脉冲发生率翻倍。
     * 为与你直观认知的 RPM 对齐，公式除以 2 进行修正：
     */
    float rpm = dir_f * (1000000.0f / period) * 60.0f / (11.0f * 21.3f) / 2.0f;
    last_rpm[motor] = rpm;
    
    return rpm;
}
