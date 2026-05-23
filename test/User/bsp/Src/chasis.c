
#include "chasis.h"


float test_FL, test_FR, test_RL, test_RR;
extern void chasis_move_done_callback(void);

chasis user_chasis = {0};
bool    chasis_braked = false;

/* =========================================================================
 * 麦轮运动学常�?(Mecanum Kinematics)
 * 车身坐标�? X=前进, Y=左移, ω=逆时针为�?
 * ========================================================================= */
#define MEC_WHEEL_RADIUS   0.029f           /* 轮半�?(m) */
#define MEC_HALF_LENGTH    0.067f           /* a = 半轴�?(m) */
#define MEC_HALF_WIDTH     0.105f           /* b = 半轮�?(m) */
#define MEC_EFF_ARM        (MEC_HALF_LENGTH + MEC_HALF_WIDTH)  /* a+b = 0.172m */

#define MEC_DT             0.001f           /* 里程计积分步�?(s) */
#define MEC_RPM2RADS       (2.0f * 3.14159265358979f / 60.0f)  /* rpm �?rad/s */
#define MEC_RADS2RPM       (60.0f / (2.0f * 3.14159265358979f))  /* rad/s �?rpm */

/* 正向运动学系�?*/
#define MEC_KV             (MEC_WHEEL_RADIUS * 0.25f)
#define MEC_KW             (MEC_WHEEL_RADIUS / (4.0f * MEC_EFF_ARM))



/* ── 电机方向反转标志 ── */
static bool motor_invert[MOTOR_NUM] = {false, false, false, false};

/* ── 方向测试状态 ── */
#define TEST_RUN_MS   100000.0f          /* 每个电机转动时长 (ms) */
#define TEST_PAUSE_MS 500.0f            /* 电机间暂停时长 (ms) */
#define TEST_RPM      282.0f            /* 测试转速 (RPM, 满速) */
static bool  motor_test_mode = false;
static float motor_test_target_rpm[4] = {0};

void motor_openloop_drive(motor_id_t id, float exp_rpm);
/* =========================================================================
 * 逆向运动�? chasis.exp_{vx,vy,wz} �?各轮 exp_rpm
 * ========================================================================= */
static void mec_inverse(const chasis *cs)
{
    float vx = cs->exp_vx;
    float vy = cs->exp_vy;
    float wz = cs->exp_wz;

    user_chasis.FL.exp_rpm = MEC_RADS2RPM * (vx - vy - MEC_EFF_ARM * wz) / MEC_WHEEL_RADIUS;
    user_chasis.FR.exp_rpm = MEC_RADS2RPM * (vx + vy + MEC_EFF_ARM * wz) / MEC_WHEEL_RADIUS;
    user_chasis.RL.exp_rpm = MEC_RADS2RPM * (vx + vy - MEC_EFF_ARM * wz) / MEC_WHEEL_RADIUS;
    user_chasis.RR.exp_rpm = MEC_RADS2RPM * (vx - vy + MEC_EFF_ARM * wz) / MEC_WHEEL_RADIUS;

    /* 应用方向反转 */
    if (motor_invert[MOTOR_FL]) user_chasis.FL.exp_rpm = -user_chasis.FL.exp_rpm;
    if (motor_invert[MOTOR_FR]) user_chasis.FR.exp_rpm = -user_chasis.FR.exp_rpm;
    if (motor_invert[MOTOR_RL]) user_chasis.RL.exp_rpm = -user_chasis.RL.exp_rpm;
    if (motor_invert[MOTOR_RR]) user_chasis.RR.exp_rpm = -user_chasis.RR.exp_rpm;

    /* 各轮方向修正系数 (EzTuner.h) */
    if (user_chasis.FL.exp_rpm > 0.0f) user_chasis.FL.exp_rpm *= CHASIS_FL_POS_SCALE;
    else                               user_chasis.FL.exp_rpm *= CHASIS_FL_NEG_SCALE;
    if (user_chasis.FR.exp_rpm > 0.0f) user_chasis.FR.exp_rpm *= CHASIS_FR_POS_SCALE;
    else                               user_chasis.FR.exp_rpm *= CHASIS_FR_NEG_SCALE;
    if (user_chasis.RL.exp_rpm > 0.0f) user_chasis.RL.exp_rpm *= CHASIS_RL_POS_SCALE;
    else                               user_chasis.RL.exp_rpm *= CHASIS_RL_NEG_SCALE;
    if (user_chasis.RR.exp_rpm > 0.0f) user_chasis.RR.exp_rpm *= CHASIS_RR_POS_SCALE;
    else                               user_chasis.RR.exp_rpm *= CHASIS_RR_NEG_SCALE;

    /* ── 运动方向级修正 (EzTuner.h: MEC_*_SCALE) ── */
    {
        const float fwd[4]   = MEC_FWD_SCALE;
        const float bwd[4]   = MEC_BWD_SCALE;
        const float left[4]  = MEC_LEFT_SCALE;
        const float right[4] = MEC_RIGHT_SCALE;
        const float *corr;

        if      (vx >  0.001f) corr = fwd;
        else if (vx < -0.001f) corr = bwd;
        else if (vy >  0.001f) corr = left;
        else if (vy < -0.001f) corr = right;
        else                   corr = NULL;

        if (corr) {
            user_chasis.FL.exp_rpm *= corr[0];
            user_chasis.FR.exp_rpm *= corr[1];
            user_chasis.RL.exp_rpm *= corr[2];
            user_chasis.RR.exp_rpm *= corr[3];
        }
    }
}




// /* =========================================================================
//  * 电机方向配置 (0=正常, 1=反转)
//  * ========================================================================= */

// void motor_set_invert(motor_id_t motor, bool invert)
// {
//     if (motor < MOTOR_NUM) motor_invert[motor] = invert;
// }

// bool motor_get_invert(motor_id_t motor)
// {
//     return (motor < MOTOR_NUM) ? motor_invert[motor] : false;
// }

// /** 启动方向测试: FL→FR→RL→RR 依次正转 60RPM/2s */
// void motor_test_direction_start(void)
// {
//     motor_test_mode  = true;
//     motor_test_step  = 0;
//     motor_test_timer = TEST_RUN_MS;
//     /* 第一步: FL 转 */
//     motor_test_target_rpm[0] = TEST_RPM;
//     motor_test_target_rpm[1] = 0;
//     motor_test_target_rpm[2] = 0;
//     motor_test_target_rpm[3] = 0;
// }

// /** 停止方向测试 */
// void motor_test_direction_stop(void)
// {
//     motor_test_mode = false;
//     motor_test_step = 0;
//     for (int i = 0; i < 4; i++) motor_test_target_rpm[i] = 0;
//     chasis_stop();
// }

// /** 方向测试 tick (100ms 回调内部调用) */
// void motor_test_tick(void)
// {
//     if (!motor_test_mode) return;

//     motor_test_timer -= 100.0f;
//     if (motor_test_timer > 0.0f) return;

//     /* 当前阶段结束, 进入下一阶段 */
//     motor_test_step++;

//     if (motor_test_step < 7) {
//         int motor_idx = motor_test_step / 2;  /* 0→FL, 1→FR, 2→RL, 3→RR */
//         bool is_run   = (motor_test_step % 2 == 0);

//         for (int i = 0; i < 4; i++) motor_test_target_rpm[i] = 0;
//         if (is_run) {
//             motor_test_target_rpm[motor_idx] = TEST_RPM;
//             motor_test_timer = TEST_RUN_MS;
//         } else {
//             motor_test_timer = TEST_PAUSE_MS;
//         }
//     } else {
//         /* 全部测试完成 */
//         motor_test_direction_stop();
//     }
// }


/* =========================================================================
 * 底盘控制 API
 * ========================================================================= */

/* ── 开环位置移动状态机 ── */
typedef enum {
    MOVE_IDLE = 0,
    MOVE_RUNNING,
    MOVE_DONE
} move_state_t;

static move_state_t move_state = MOVE_IDLE;
static float move_timer_ms   = 0.0f;   /* 剩余时间 (ms) */
static bool  brake_hold       = false;  /* 刹车保持: 禁止 chasis_cal 覆盖 */


/* =========================================================================
 * chasis_set_velocity — 核心控制函数
 * 输入: vx(前进 m/s), vy(左移 m/s), wz(逆时针 rad/s)
 * 内部: 麦轮逆运动学 → 各轮 RPM → 开环 PWM 直接驱动
 * ========================================================================= */
void chasis_set_velocity(float vx, float vy, float wz)
{
    brake_hold    = false;  /* 退出刹车模式 */
    chasis_braked = false;

    user_chasis.exp_vx = vx;
    user_chasis.exp_vy = vy;
    user_chasis.exp_wz = wz;

    /* 麦轮逆运动学: 车身速度 → 各轮 RPM */
    mec_inverse(&user_chasis);

    /* 开环驱动四轮 */
    motor_openloop_drive(MOTOR_FL, user_chasis.FL.exp_rpm);
    motor_openloop_drive(MOTOR_FR, user_chasis.FR.exp_rpm);
    motor_openloop_drive(MOTOR_RL, user_chasis.RL.exp_rpm);
    motor_openloop_drive(MOTOR_RR, user_chasis.RR.exp_rpm);
}

void chasis_stop(void)
{
    chasis_set_velocity(0.0f, 0.0f, 0.0f);
}

/** 急刹车: 四轮 H 桥两通道同时输出 HIGH, 电机短路制动 */
void chasis_brake(void)
{
    brake_hold    = true;  /* 保持刹车, 防止 chasis_cal 覆盖 */
    chasis_braked = true;

    user_chasis.exp_vx = 0.0f;
    user_chasis.exp_vy = 0.0f;
    user_chasis.exp_wz = 0.0f;

    motor_brake(MOTOR_FL);
    motor_brake(MOTOR_FR);
    motor_brake(MOTOR_RL);
    motor_brake(MOTOR_RR);
}

/* ── 线性刹车状态 ── */
static bool     brake_linear_active = false;
static float    brake_linear_dvx, brake_linear_dvy, brake_linear_dwz;
static uint32_t brake_linear_time_ms, brake_linear_elapsed;

void chasis_linear_brake(uint32_t time_ms)
{
    if (time_ms == 0) { chasis_brake(); return; }
    brake_linear_active  = true;
    brake_linear_time_ms = time_ms;
    brake_linear_elapsed = 0;
    /* 预计算每 100ms 步进减量, 避免循环内除法 */
    float steps = (float)time_ms / 100.0f;
    brake_linear_dvx = user_chasis.exp_vx / steps;
    brake_linear_dvy = user_chasis.exp_vy / steps;
    brake_linear_dwz = user_chasis.exp_wz / steps;
}

/* ── 梯形运动状态 ── */
typedef struct {
    bool     active;
    float    vx_s, vy_s, wz_s;    /* 起始速度 */
    float    vx_t, vy_t, wz_t;    /* 目标速度 */
    float    dv;                   /* 每 100ms 速度增量 */
    uint32_t accel_ticks;          /* 加速段 tick 数 */
    uint32_t cruise_ticks;         /* 匀速段 tick 数 */
    uint32_t decel_ticks;          /* 减速段 tick 数 */
    uint32_t tick;                 /* 当前 tick */
} trapezoid_t;

static trapezoid_t trap;
bool chasis_trapezoid_done = true;

void chasis_trapezoid_move(float vx, float vy, float wz, float accel, uint32_t time_ms)
{
    /*
     * chasis_cal() 在 one_hundured_ms_callback() 中以 100Hz 调用，
     * trap.tick 每 10ms 自增 1 次，因此 1 tick = 10ms。
     * total_ticks = time_ms / 10, adv = accel * 0.01 (每 tick 速度变化量)
     */
    uint32_t total_ticks = time_ms / 10;
    if (total_ticks < 2) total_ticks = 2;

    /* 记录当前速度为起始速度（支持运动中重写） */
    trap.vx_s = user_chasis.exp_vx;
    trap.vy_s = user_chasis.exp_vy;
    trap.wz_s = user_chasis.exp_wz;
    trap.vx_t = vx;
    trap.vy_t = vy;
    trap.wz_t = wz;
    trap.tick = 0;

    /* 计算各轴所需加减速 tick 数，取最大 */
    float adv = accel * 0.01f;       /* 每 tick (10ms) 速度变化量, chasis_cal @100Hz */
    uint32_t need_accel = 0, need_decel = 0;

    float dvx_a = (vx - trap.vx_s > 0) ? (vx - trap.vx_s) : (trap.vx_s - vx);
    float dvy_a = (vy - trap.vy_s > 0) ? (vy - trap.vy_s) : (trap.vy_s - vy);
    float dwz_a = (wz - trap.wz_s > 0) ? (wz - trap.wz_s) : (trap.wz_s - wz);
    float dv_max_a = dvx_a;
    if (dvy_a > dv_max_a) dv_max_a = dvy_a;
    if (dwz_a > dv_max_a) dv_max_a = dwz_a;
    if (dv_max_a > 0.0f) need_accel = (uint32_t)(dv_max_a / adv) + 1;

    float dvx_d = (vx > 0) ? vx : -vx;
    float dvy_d = (vy > 0) ? vy : -vy;
    float dwz_d = (wz > 0) ? wz : -wz;
    float dv_max_d = dvx_d;
    if (dvy_d > dv_max_d) dv_max_d = dvy_d;
    if (dwz_d > dv_max_d) dv_max_d = dwz_d;
    if (dv_max_d > 0.0f) need_decel = (uint32_t)(dv_max_d / adv) + 1;

    /* 梯形 → 三角判断 */
    if (need_accel + need_decel > total_ticks) {
        /* 时间不够到目标速度，三角波 */
        need_accel = total_ticks / 2;
        need_decel = total_ticks - need_accel;
        trap.cruise_ticks = 0;
    } else {
        trap.cruise_ticks = total_ticks - need_accel - need_decel;
    }
    trap.accel_ticks = need_accel;
    trap.decel_ticks = need_decel;
    trap.dv = adv;
    trap.active = true;
    chasis_trapezoid_done = false;
}
/* ── 位置控制 (开环, 无传感器) ── */






/* =========================================================================
 * 开环电机驱动: exp_rpm → PWM duty, 无传感器反馈
 *
 * duty = |exp_rpm| / OPENLOOP_MAX_RPM * ratio * PWM_PERIOD
 * exp_rpm == 0 → motor_coast (惰行)
 * ========================================================================= */
static const float openloop_ratio[MOTOR_NUM] = {
    OPENLOOP_FL_RATIO,
    OPENLOOP_FR_RATIO,
    OPENLOOP_RL_RATIO,
    OPENLOOP_RR_RATIO,
};

void motor_openloop_drive(motor_id_t id, float exp_rpm)
{
    if (id >= MOTOR_NUM) return;

    if (exp_rpm == 0.0f) {
        motor_coast(id);
        return;
    }

    float abs_rpm = fabsf(exp_rpm);
    float ratio   = openloop_ratio[id];
    uint16_t duty = (uint16_t)(abs_rpm / OPENLOOP_MAX_RPM * ratio * (float)PWM_PERIOD);

    if (duty > PWM_PERIOD) duty = PWM_PERIOD;
    if (duty > 0 && duty < OPENLOOP_MIN_DUTY) duty = OPENLOOP_MIN_DUTY;

    motor_set(id, exp_rpm > 0.0f, duty);
}

void chasis_cal(void)
{
    /* ── 线性刹车中 ── */
    if (brake_linear_active) {
        brake_linear_elapsed += 100;
        if (brake_linear_elapsed >= brake_linear_time_ms) {
            chasis_brake();
            brake_linear_active = false;
        } else {
            user_chasis.exp_vx -= brake_linear_dvx;
            user_chasis.exp_vy -= brake_linear_dvy;
            user_chasis.exp_wz -= brake_linear_dwz;
            mec_inverse(&user_chasis);
            motor_openloop_drive(MOTOR_FL, user_chasis.FL.exp_rpm);
            motor_openloop_drive(MOTOR_FR, user_chasis.FR.exp_rpm);
            motor_openloop_drive(MOTOR_RL, user_chasis.RL.exp_rpm);
            motor_openloop_drive(MOTOR_RR, user_chasis.RR.exp_rpm);
        }
        return;
    }

    /* ── 梯形运动中 ── */
    if (trap.active) {
        trap.tick++;
        uint32_t total = trap.accel_ticks + trap.cruise_ticks + trap.decel_ticks;
        float t, vx, vy, wz;

        if (trap.tick <= trap.accel_ticks) {
            /* 加速段 */
            t = (float)trap.tick / (float)trap.accel_ticks;
            vx = trap.vx_s + (trap.vx_t - trap.vx_s) * t;
            vy = trap.vy_s + (trap.vy_t - trap.vy_s) * t;
            wz = trap.wz_s + (trap.wz_t - trap.wz_s) * t;
        } else if (trap.tick <= trap.accel_ticks + trap.cruise_ticks) {
            /* 匀速段 */
            vx = trap.vx_t;
            vy = trap.vy_t;
            wz = trap.wz_t;
        } else if (trap.tick < total) {
            /* 减速段 */
            t = (float)(trap.tick - trap.accel_ticks - trap.cruise_ticks) / (float)trap.decel_ticks;
            vx = trap.vx_t * (1.0f - t);
            vy = trap.vy_t * (1.0f - t);
            wz = trap.wz_t * (1.0f - t);
        } else {
            /* 完成 */
            chasis_brake();
            trap.active = false;
            chasis_trapezoid_done = true;
            return;
        }

        user_chasis.exp_vx = vx;
        user_chasis.exp_vy = vy;
        user_chasis.exp_wz = wz;
        mec_inverse(&user_chasis);
        motor_openloop_drive(MOTOR_FL, user_chasis.FL.exp_rpm);
        motor_openloop_drive(MOTOR_FR, user_chasis.FR.exp_rpm);
        motor_openloop_drive(MOTOR_RL, user_chasis.RL.exp_rpm);
        motor_openloop_drive(MOTOR_RR, user_chasis.RR.exp_rpm);
        return;
    }

    /* 刹车保持模式: 不覆盖电机状态 */
    if (brake_hold) return;

    /* ── 位置移动倒计时 ── */
    if (move_state == MOVE_RUNNING) {
        move_timer_ms -= 100.0f;
        if (move_timer_ms <= 0.0f) {
            chasis_stop();
            move_state     = MOVE_DONE;
            move_timer_ms  = 0.0f;
        }
    }

    /* 测试模式: 直接指定各轮 RPM */
    if (motor_test_mode) {
        user_chasis.FL.exp_rpm = motor_test_target_rpm[0] * (motor_invert[MOTOR_FL] ? -1.0f : 1.0f);
        user_chasis.FR.exp_rpm = motor_test_target_rpm[1] * (motor_invert[MOTOR_FR] ? -1.0f : 1.0f);
        user_chasis.RL.exp_rpm = motor_test_target_rpm[2] * (motor_invert[MOTOR_RL] ? -1.0f : 1.0f);
        user_chasis.RR.exp_rpm = motor_test_target_rpm[3] * (motor_invert[MOTOR_RR] ? -1.0f : 1.0f);
    } else {
        /* 正常模式: 用当前 exp_vx/vy/wz 重算 RPM (应对持续运动) */
        mec_inverse(&user_chasis);
    }

    /* 开环驱动四轮 */
    motor_openloop_drive(MOTOR_FL, user_chasis.FL.exp_rpm);
    motor_openloop_drive(MOTOR_FR, user_chasis.FR.exp_rpm);
    motor_openloop_drive(MOTOR_RL, user_chasis.RL.exp_rpm);
    motor_openloop_drive(MOTOR_RR, user_chasis.RR.exp_rpm);
}

