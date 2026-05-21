#include "ZDT_CAN.h"

/**
	* @brief   CAN发送多个字节
	* @param   无
	* @retval  无
    * @note 记得替换hcan
	*/
void can_SendCmd(uint8_t *cmd, uint8_t len)
{
	uint8_t i = 0, j = 0, k = 0, l = 0, packNum = 0;
	uint8_t txData[32]; uint32_t ExtId; size_t length;

	// 除去ID地址和功能码后的数据长度
	j = len - 2;

	// 发送数据
	while(i < j)
	{
		// 数据个数
		k = j - i;

		// 填充缓存
		ExtId = ((uint32_t)cmd[0] << 8) | (uint32_t)packNum;
		txData[0] = cmd[1];

		// 小于8字节命令
		if(k < 8)
		{
			for(l=0; l < k; l++,i++) { txData[l + 1] = cmd[i + 2]; } length = k + 1;
		}
		// 大于8字节命令，分包发送，每包数据最多发送8个字节
		else
		{
			for(l=0; l < 7; l++,i++) { txData[l + 1] = cmd[i + 2]; } length = 8;
		}

		// 发送数据
		while(!CAN_send_ext_frame(ExtId, txData, length));

		// 记录发送的第几包的数据
		++packNum;
	}
}
