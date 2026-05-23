#include "JY901.h"
#include "wit_c_sdk.h"
#include "bsp_UART1.h"
#include <string.h>

float jy901_yaw;

static void _jy_write(uint8_t *data, uint32_t len)
{
    uart1_send(data, (uint16_t)len);
}

static void _jy_delay(uint16_t ms) { (void)ms; }
static void _jy_reg_cb(uint32_t reg, uint32_t num) { (void)reg; (void)num; }

void JY901_Init(void)
{
    jy901_yaw = 0.0f;
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitSerialWriteRegister(_jy_write);
    WitDelayMsRegister(_jy_delay);
    WitRegisterCallBack(_jy_reg_cb);
    WitWriteReg(0x24, 0x0001);   /* AXIS6=1: 六轴融合(关磁力计) */
}

void JY901_Update(void)
{
    jy901_yaw = (float)sReg[Yaw] * 0.01f;
}