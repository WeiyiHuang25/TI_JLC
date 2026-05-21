
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

/* 里程计位�?(全局坐标�? */
static float mec_odom_x     = 0.0f;
static float mec_odom_y     = 0.0f;
static float mec_odom_theta = 0.0f;

/* ── 电机方向反转标志 ── */
static bool motor_invert[MOTOR_NUM] = {false, false, false, false};

/* ── 方向测试状态 ── */
#define TEST_RUN_MS   100000.0f          /* 每个电机转动时长 (ms) */
#define TEST_PAUSE_MS 500.0f            /* 电机间暂停时长 (ms) */
#define TEST_RPM      282.0f            /* 测试转速 (RPM, 满速) */
static bool  motor_test_mode = false;
static float motor_test_target_rpm[4] = {0};
static int   motor_test_step = 0;
static float motor_test_timer = 0.0f;

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

/* =========================================================================
 * 正向运动�?+ 里程计积�? 各轮 rpm �?chasis.{vx,vy,wz} + 全局位姿
 * ========================================================================= */
static void mec_forward_and_odom(void)
{
    float w[4];
    w[0] = user_chasis.FL.rpm * MEC_RPM2RADS;
    w[1] = user_chasis.FR.rpm * MEC_RPM2RADS;
    w[2] = user_chasis.RL.rpm * MEC_RPM2RADS;
    w[3] = user_chasis.RR.rpm * MEC_RPM2RADS;

    float sum_vx =  w[0] + w[1] + w[2] + w[3];
    float sum_vy = -w[0] + w[1] + w[2] - w[3];
    float sum_w  = -w[0] + w[1] - w[2] + w[3];

    user_chasis.vx = MEC_KV * sum_vx;
    user_chasis.vy = MEC_KV * sum_vy;
    user_chasis.wz = MEC_KW * sum_w;

    /* ── 里程计积�?(使用 arm_math DSP �? ── */
    float c = cosf(mec_odom_theta);
    float s = sinf(mec_odom_theta);

    float dx = user_chasis.vx * c - user_chasis.vy * s;
    float dy = user_chasis.vx * s + user_chasis.vy * c;

    mec_odom_x     += dx * MEC_DT;
    mec_odom_y     += dy * MEC_DT;
    mec_odom_theta += user_chasis.wz * MEC_DT;

    /* 角度归一化到 [-PI, PI] */
    if      (mec_odom_theta >  3.14159265358979f) mec_odom_theta -= 6.28318530717958f;
    else if (mec_odom_theta < -3.14159265358979f) mec_odom_theta += 6.28318530717958f;
}


/* =========================================================================
 * 电机方向配置 (0=正常, 1=反转)
 * ========================================================================= */

void motor_set_invert(motor_id_t motor, bool invert)
{
    if (motor < MOTOR_NUM) motor_invert[motor] = invert;
}

bool motor_get_invert(motor_id_t motor)
{
    return (motor < MOTOR_NUM) ? motor_invert[motor] : false;
}

/** 启动方向测试: FL→FR→RL→RR 依次正转 60RPM/2s */
void motor_test_direction_start(void)
{
    motor_test_mode  = true;
    motor_test_step  = 0;
    motor_test_timer = TEST_RUN_MS;
    /* 第一步: FL 转 */
    motor_test_target_rpm[0] = TEST_RPM;
    motor_test_target_rpm[1] = 0;
    motor_test_target_rpm[2] = 0;
    motor_test_target_rpm[3] = 0;
}

/** 停止方向测试 */
void motor_test_direction_stop(void)
{
    motor_test_mode = false;
    motor_test_step = 0;
    for (int i = 0; i < 4; i++) motor_test_target_rpm[i] = 0;
    chasis_stop();
}

/** 方向测试 tick (100ms 回调内部调用) */
void motor_test_tick(void)
{
    if (!motor_test_mode) return;

    motor_test_timer -= 100.0f;
    if (motor_test_timer > 0.0f) return;

    /* 当前阶段结束, 进入下一阶段 */
    motor_test_step++;

    if (motor_test_step < 7) {
        int motor_idx = motor_test_step / 2;  /* 0→FL, 1→FR, 2→RL, 3→RR */
        bool is_run   = (motor_test_step % 2 == 0);

        for (int i = 0; i < 4; i++) motor_test_target_rpm[i] = 0;
        if (is_run) {
            motor_test_target_rpm[motor_idx] = TEST_RPM;
            motor_test_timer = TEST_RUN_MS;
        } else {
            motor_test_timer = TEST_PAUSE_MS;
        }
    } else {
        /* 全部测试完成 */
        motor_test_direction_stop();
    }
}


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
static float move_speed_mps  = 0.5f;   /* 默认移动速度 (m/s) */

/* ── 速度控制 ── */

/** 设置底盘期望速度 (m/s, m/s, rad/s), 持续生效直到下次调用 */
inline void chasis_set_velocity(float vx, float vy, float wz)
{
    user_chasis.exp_vx = vx;
    user_chasis.exp_vy = vy;
    user_chasis.exp_wz = wz;
    move_state = MOVE_IDLE;  /* 手动调速时退出位置模式 */
}

/** 停止 */
inline void chasis_stop(void)
{
    chasis_set_velocity(0.0f, 0.0f, 0.0f);
}

/* ── 位置控制 (开环, 无传感器) ── */

/**
 * @brief 相对位移移动 (开环, 阻塞式检查)
 * @param dx      前进距离 (m), 负=后退
 * @param dy      左移距离 (m), 负=右移
 * @param dtheta  旋转角度 (rad), 正=逆时针
 * @param speed   移动线速度 (m/s), 旋转时自动折算
 *
 * 调用后每 100ms 自动推进, 用 chasis_move_is_done() 检查是否完成.
 *
 * 示例:
 *   chasis_move(1.0f, 0.0f, 0.0f, 0.5f);  // 前进 1m, 0.5m/s
 *   while (!chasis_move_is_done());         // 等待完成
 *   chasis_move(0.0f, 0.0f, 3.14f, 0.5f); // 旋转 180°
 */
void chasis_move(float dx, float dy, float dtheta, float speed)
{
    /* 计算各轴所需时间, 取最长者作为总时间 */
    float tx = (dx  != 0.0f) ? fabsf(dx)  / speed : 0.0f;
    float ty = (dy  != 0.0f) ? fabsf(dy)  / speed : 0.0f;
    float tw = (dtheta != 0.0f) ? fabsf(dtheta) / speed : 0.0f;
    float t_max = tx;
    if (ty > t_max) t_max = ty;
    if (tw > t_max) t_max = tw;

    if (t_max <= 0.0f) { chasis_stop(); return; }

    /* 按最长轴时间重新折算各轴速度, 保证同时到达 */
    float vx = dx  / t_max;
    float vy = dy  / t_max;
    float wz = dtheta / t_max;

    chasis_set_velocity(vx, vy, wz);
    move_speed_mps = speed;
    move_timer_ms  = t_max * 1000.0f;
    move_state     = MOVE_RUNNING;
}

/** 检查位置移动是否完成 */
bool chasis_move_is_done(void)
{
    return (move_state == MOVE_DONE);
}

/** 中断正在执行的移动 */
void chasis_move_abort(void)
{
    chasis_stop();
    move_state = MOVE_IDLE;
    move_timer_ms = 0.0f;
}

/* ── 里程计读取 ── */

float chasis_get_odom_x(void)     { return mec_odom_x; }
float chasis_get_odom_y(void)     { return mec_odom_y; }
float chasis_get_odom_theta(void) { return mec_odom_theta; }
void  chasis_reset_odom(void)     { mec_odom_x = mec_odom_y = mec_odom_theta = 0.0f; }







/* =========================================================================
 * 单电机驱动封�? PID速度�?�?刹车/惰行/驱动 三�?
 *
 * 刹车策略:
 *   �?exp_rpm == 0 && |speed| < BRAKE_SPEED_THR �?motor_brake (短路刹车锁电�?
 *   �?exp_rpm == 0 && |speed| �?BRAKE_SPEED_THR �?PID 主动减速至 0
 *   �?exp_rpm != 0                            �?PID 正常调�?
 * ========================================================================= */
static void motor_drive_brake(motor_id_t id, pid_t *pid, float target, float abs_speed)
{
    int32_t out;

    if (target == 0.0f && abs_speed < BRAKE_SPEED_THR) {
        pid_reset(pid);
        motor_brake(id);
        return;
    }

    out = pid_speed_drive(pid, (int32_t)target, (uint32_t)abs_speed);

    if (out == 0) {
        motor_coast(id);
    } else {
        motor_set(id, out >= 0, (uint16_t)abs(out));
    }
}

void chasis_cal(void)
{
        /* ── 位置移动倒计时 ── */
    if (move_state == MOVE_RUNNING) {
        move_timer_ms -= 100.0f;
        if (move_timer_ms <= 0.0f) {
            chasis_stop();
            move_state     = MOVE_DONE;
            move_timer_ms  = 0.0f;
        }
    }

    /* 1. 目标转速来源: 测试模式直接指定, 正常模式逆运动学解算 */
    if (motor_test_mode) {
        user_chasis.FL.exp_rpm = motor_test_target_rpm[0] * (motor_invert[MOTOR_FL] ? -1.0f : 1.0f);
        user_chasis.FR.exp_rpm = motor_test_target_rpm[1] * (motor_invert[MOTOR_FR] ? -1.0f : 1.0f);
        user_chasis.RL.exp_rpm = motor_test_target_rpm[2] * (motor_invert[MOTOR_RL] ? -1.0f : 1.0f);
        user_chasis.RR.exp_rpm = motor_test_target_rpm[3] * (motor_invert[MOTOR_RR] ? -1.0f : 1.0f);
    } else {
        mec_inverse(&user_chasis);
    }

    /* 2. PID 速度环 + 电机驱动 */
    motor_drive_brake(MOTOR_FL, &user_chasis.FL.pid,
                      user_chasis.FL.exp_rpm, motor_get_speed(MOTOR_FL));

    motor_drive_brake(MOTOR_FR, &user_chasis.FR.pid,
                      user_chasis.FR.exp_rpm, motor_get_speed(MOTOR_FR));

    motor_drive_brake(MOTOR_RL, &user_chasis.RL.pid,
                      user_chasis.RL.exp_rpm, motor_get_speed(MOTOR_RL));

    motor_drive_brake(MOTOR_RR, &user_chasis.RR.pid,
                      user_chasis.RR.exp_rpm, motor_get_speed(MOTOR_RR));
}

void chasis_update(void)
{
        /* 1. 传感器读�?(带符�?rpm 供日�?显示) */
    user_chasis.FL.rpm = motor_get_speed(MOTOR_FL);
    user_chasis.FR.rpm = motor_get_speed(MOTOR_FR);   /* 硬件反向 */
    user_chasis.RL.rpm = motor_get_speed(MOTOR_RL);   /* 硬件反向 */
    user_chasis.RR.rpm = motor_get_speed(MOTOR_RR);
    test_FL = user_chasis.FL.rpm;
    test_FR = user_chasis.FR.rpm;
    test_RL = user_chasis.RL.rpm;
    test_RR = user_chasis.RR.rpm;
    /* 2. 正向运动�?+ 里程计积�?*/
    mec_forward_and_odom();
    if(chasis_move_is_done())
    {
        chasis_move_done_callback();
    }
}
