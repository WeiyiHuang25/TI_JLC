#ifndef __ZDT_CAN_H
#define __ZDT_CAN_H


#include "stdbool.h"
#include "ti_msp_dl_config.h"
#include "bsp_CAN.h"



/**
	* @brief   CAN发送多个字节
	* @param   无
	* @retval  无
	*/
void can_SendCmd(uint8_t *cmd, uint8_t len);

#endif
