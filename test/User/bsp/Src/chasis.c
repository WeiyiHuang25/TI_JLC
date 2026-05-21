
#include "chasis.h"


float test_FL, test_FR, test_RL, test_RR;
extern void chasis_move_done_callback(void);

chasis user_chasis = {0};

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
    brake_hold = false;  /* 退出刹车模式 */

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
    brake_hold = true;  /* 保持刹车, 防止 chasis_cal 覆盖 */

    user_chasis.exp_vx = 0.0f;
    user_chasis.exp_vy = 0.0f;
    user_chasis.exp_wz = 0.0f;

    motor_brake(MOTOR_FL);
    motor_brake(MOTOR_FR);
    motor_brake(MOTOR_RL);
    motor_brake(MOTOR_RR);
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

