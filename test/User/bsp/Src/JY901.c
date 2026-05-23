#include "JY901.h"
#include "wit_c_sdk.h"
#include "bsp_UART1.h"
#include <string.h>

jy901_t jy901;

/* DT: 100ms = 0.1s（与 JY901_Update 调用周期一致） */
#define JY901_DT  0.1f
/* 加速度量纲: mg → m/s²   (9.8 / 1000) */
#define MG2MS2    0.0098f
/* 角度量纲: 0.01° → ° */
#define DEG2DEG   0.01f

/* =========================================================================
 * 串口发送注册
 * ========================================================================= */
static void _jy_write(uint8_t *data, uint32_t len)
{
    uart1_send(data, (uint16_t)len);
}

static void _jy_delay(uint16_t ms)
{
    (void)ms;   /* 不阻塞，校准不需要 */
}

/* =========================================================================
 * 数据回调（SDK 每解析完一帧自动调用）→ 无需额外操作
 * ========================================================================= */
static void _jy_reg_cb(uint32_t reg, uint32_t num)
{
    (void)reg; (void)num;
}

/* =========================================================================
 * 初始化
 * ========================================================================= */
void JY901_Init(void)
{
    memset(&jy901, 0, sizeof(jy901));
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitSerialWriteRegister(_jy_write);
    WitDelayMsRegister(_jy_delay);
    WitRegisterCallBack(_jy_reg_cb);
}

/* =========================================================================
 * 100ms 周期更新: 读 sReg → 积分速度/位置
 * ========================================================================= */
void JY901_Update(void)
{
    /* 加速度 (mg → m/s²) */
    jy901.acc_x = (float)sReg[AX] * MG2MS2;
    jy901.acc_y = (float)sReg[AY] * MG2MS2;
    jy901.acc_z = (float)sReg[AZ] * MG2MS2;

    /* 角速度 (0.01°/s → °/s) */
    jy901.gyro_x = (float)sReg[GX] * DEG2DEG;
    jy901.gyro_y = (float)sReg[GY] * DEG2DEG;
    jy901.gyro_z = (float)sReg[GZ] * DEG2DEG;

    /* 姿态角 (0.01° → °) */
    jy901.roll  = (float)sReg[Roll]  * DEG2DEG;
    jy901.pitch = (float)sReg[Pitch] * DEG2DEG;
    jy901.yaw   = (float)sReg[Yaw]   * DEG2DEG;

    /* 水平速度/位置积分 */
    jy901.vel_x += jy901.acc_x * JY901_DT;
    jy901.vel_y += jy901.acc_y * JY901_DT;
    jy901.pos_x += jy901.vel_x * JY901_DT;
    jy901.pos_y += jy901.vel_y * JY901_DT;
}