#include "bsp_CAN.h"



void MCAN_Send_Example(void)
{
    DL_MCAN_TxBufElement txMsg;

    // 1. 初始化发送报文结构体
    txMsg.id = (0x123 << 18);        // 设置标准帧ID (注意：标准ID必须左移18位放入T0中)
    txMsg.esi = 0;                   // 错误状态指示 (0: 否)
    txMsg.xtd = 0;                   // 标准帧(0) / 扩展帧(1)
    txMsg.rtr = 0;                   // 数据帧(0) / 远程帧(1)
    txMsg.brs = 1;                   // CAN FD 开启波特率切换(1) / 不切换(0)
    txMsg.dlc = 4;                   // 数据长度代码 (DLC=4 代表 4字节)
    txMsg.fdf = 1;                   // CAN FD 格式(1) / 经典 CAN 格式(0)
    txMsg.efc = 0;                   // 不保存发送事件 FIFO
    txMsg.mm  = 0;                   // 消息标记 (可选)

    // 2. 填充 payload 数据数据
    txMsg.data[0] = 0x11223344;      // 因为是 uint32_t 数组，注意字节序
    // 如果 DLC > 4，继续填充 txMsg.data[1], data[2] ...

    // 3. 将报文写入 Message RAM 的 Dedicated TX Buffer 中
    // 参数3: 0 代表写入第0号 TX Buffer
    DL_MCAN_writeMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_BUF, 0, &txMsg);

    // 4. 请求发送该 Buffer
    DL_MCAN_TXBufAddReq(MCAN0_INST, 0); 
}

void MCAN_Receive_Example(void)
{
    DL_MCAN_RxBufElement rxMsg;
    DL_MCAN_RxFIFOStatus rxFS;

    // 1. 获取 RX FIFO 0 的状态
    DL_MCAN_getRxFIFOStatus(MCAN0_INST, &rxFS);

    // 2. 判断 FIFO 0 里面有没有数据 (fill level > 0)
    if ((rxFS.num % 128) > 0) 
    {
        // 3. 从 Message RAM 读出这帧数据
        // 参数3: getIndex 是当前 FIFO 的读取索引
        // 参数4: 告诉驱动你要读的是 FIFO 0
        DL_MCAN_readMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_FIFO, rxFS.getIdx, 
                           DL_MCAN_RX_FIFO_NUM_0, &rxMsg);

        // 4. 解析数据
        // 标准帧 ID 保存在 rxMsg.id 的高14位，需要右移 18 位提取
        uint32_t receivedID = (rxMsg.xtd == 0) ? (rxMsg.id >> 18) : (rxMsg.id);
        uint32_t lengthCode = rxMsg.dlc;     // 获取数据长度
        uint32_t dataWord0  = rxMsg.data[0]; // 获取 payload 第一格

        // 5. 非常重要：告诉硬件 FIFO 这个元素我已经拿走了，腾出空间 (Acknowledge)
        DL_MCAN_writeRxFIFOAck(MCAN0_INST, DL_MCAN_RX_FIFO_NUM_0, rxFS.getIdx);
    }
}

