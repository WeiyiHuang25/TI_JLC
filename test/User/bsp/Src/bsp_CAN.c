#include "bsp_CAN.h"
#include <string.h>
#include "bsp_PWM.h"
void handleCanRxMessage(void);
void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg);

bool CAN_send_std_frame(uint32_t id, uint8_t* data, uint16_t len)
{
    DL_MCAN_TxBufElement txMsg;
    DL_MCAN_TxFIFOStatus txfifoStatus;
    uint32_t timeout;

    /* Initialize message to transmit. */
    /* Identifier Value. */
    txMsg.id = ((uint32_t)id) << 18U;
    /* Transmit data frame. */
    txMsg.rtr = 0U;
    /* 11-bit standard identifier. */
    txMsg.xtd = 0U;
    /* ESI bit in CAN FD format depends only on error passive flag. */
    txMsg.esi = 0U;
    /* Transmitting 4 bytes. */
    txMsg.dlc = (uint32_t)len;
    /* CAN FD frames transmitted with bit rate switching. */
    txMsg.brs = 0U;
    /* Frame transmitted in CAN FD format. */
    txMsg.fdf = 0U;  //protocol_mode;
    /* Store Tx events. */
    txMsg.efc = 0U;
    /* Message Marker. */
    txMsg.mm = 0xAAU;
    /* Data bytes. */
    for (int i = 0; i < len; i++) txMsg.data[i] = data[i];

    /* Wait for CAN to enter NORMAL mode, with timeout */
    timeout = CAN_SEND_TIMEOUT;
    while (DL_MCAN_OPERATION_MODE_NORMAL != DL_MCAN_getOpMode(CANFD0))
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }

    /* Write Tx Message to the Message RAM (FIFO). */
    DL_MCAN_writeMsgRam(CANFD0, DL_MCAN_MEM_TYPE_FIFO, 0, &txMsg);

    /* Get put index and other TxFIFO details in txfifoStatus*/
    DL_MCAN_getTxFIFOQueStatus(CANFD0, &txfifoStatus);

    /* Enable Transmission interrupt.*/
    DL_MCAN_TXBufTransIntrEnable(CANFD0, txfifoStatus.putIdx, 1U);

    /* Add request for transmission. */
    DL_MCAN_TXBufAddReq(CANFD0, txfifoStatus.putIdx);

    return true;
}


bool CAN_send_ext_frame(uint32_t id, uint8_t* data, uint16_t len)
{
    DL_MCAN_TxBufElement txMsg;
    DL_MCAN_TxFIFOStatus txfifoStatus;
    uint32_t timeout;

    /* Initialize message to transmit. */
    /* Identifier Value. */
    txMsg.id = id;
    /* Transmit data frame. */
    txMsg.rtr = 0U;
    /* 11-bit standard identifier. */
    txMsg.xtd = 1U;
    /* ESI bit in CAN FD format depends only on error passive flag. */
    txMsg.esi = 0U;
    /* Transmitting 4 bytes. */
    txMsg.dlc = (uint32_t)len;
    /* CAN FD frames transmitted with bit rate switching. */
    txMsg.brs = 0U;
    /* Frame transmitted in CAN FD format. */
    txMsg.fdf = 0U;  //protocol_mode;
    /* Store Tx events. */
    txMsg.efc = 0U;
    /* Message Marker. */
    txMsg.mm = 0xAAU;
    /* Data bytes. */
    for (int i = 0; i < len; i++) txMsg.data[i] = data[i];

    /* Wait for CAN to enter NORMAL mode, with timeout */
    timeout = CAN_SEND_TIMEOUT;
    while (DL_MCAN_OPERATION_MODE_NORMAL != DL_MCAN_getOpMode(CANFD0))
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }

    /* Write Tx Message to the Message RAM (FIFO). */
    DL_MCAN_writeMsgRam(CANFD0, DL_MCAN_MEM_TYPE_FIFO, 0, &txMsg);

    /* Get put index and other TxFIFO details in txfifoStatus*/
    DL_MCAN_getTxFIFOQueStatus(CANFD0, &txfifoStatus);

    /* Enable Transmission interrupt.*/
    DL_MCAN_TXBufTransIntrEnable(CANFD0, txfifoStatus.putIdx, 1U);

    /* Add request for transmission. */
    DL_MCAN_TXBufAddReq(CANFD0, txfifoStatus.putIdx);

    return true;
}





void MCAN0_INST_IRQHandler(void)
{
    // 获取中断挂起源, 返回值是高优先级中断源的中断向量
    switch (DL_MCAN_getPendingInterrupt(MCAN0_INST))
    {
        // 处理LINE0中断, 如RF0F, RF0W或用户配置的其他中断源
        case DL_MCAN_IIDX_LINE0:
        {
            // (1) 获取LINE0上具体的中断状态, 并清除中断标志
            uint32_t status = DL_MCAN_getIntrStatus(MCAN0_INST);
            DL_MCAN_clearIntrStatus(MCAN0_INST, status, DL_MCAN_INTR_SRC_MCAN_LINE_0);

            // 新消息中断
            if (status & MCAN_IR_RF0N_MASK) {
                handleCanRxMessage();
            }
            break;
        }
        default:
            break;
    }
}


// void CAN_Rx_FIFO0_New_Message_Callback(DL_MCAN_RxBufElement rxMsg)
// {
//     uint8_t buffer[100];
//     size_t size = rxMsg.dlc;
//     uint32_t id;
//     memcpy(buffer, rxMsg.data, size);
//     if (rxMsg.xtd == 0)
//     {
//         id = ((uint32_t)rxMsg.id) >> 18U;
//         CAN_send_std_frame(id, buffer, size);

//     }
//     else
//     {
//         id = rxMsg.id;
//         CAN_send_ext_frame(id, buffer, size);
//     }
//     if (buffer[0] == 0x0U)
//     {
//         motor_set(MOTOR_FL, true, 2500);
//     }
//         if (buffer[0] == 0x1U)
//     {
//         motor_set(MOTOR_FL, true, 4999);
//     }
//     if (buffer[0] == 0x2U)
//     {
//         motor_brake(MOTOR_FL);
//     }
//     if (buffer[0] == 0x3U)
//     {
//         motor_set(MOTOR_FL, false, 2500);
//     }
//     if (buffer[0] == 0x4U)
//     {
//         motor_set(MOTOR_FL, false, 4999);
//     }
// }





void handleCanRxMessage(void)
{
    DL_MCAN_RxBufElement rxMsg;
    DL_MCAN_RxFIFOStatus rx_fs;

    rx_fs.num = DL_MCAN_RX_FIFO_NUM_0; // 指定操作的FIFO

    // 1. 循环读取, 确保一次性处理完FIFO中所有积压消息
    while (1)
    {
        // 2. 获取FIFO状态
        DL_MCAN_getRxFIFOStatus(MCAN0_INST, &rx_fs);

        // 3. 检查FIFO是否为空, 是则退出循环
        if (rx_fs.fillLvl == 0) break;

        // 4. 从FIFO中读取消息
        DL_MCAN_readMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_FIFO, 0U, rx_fs.num, &rxMsg);

        // 5. 告知硬件已经读取该位置的消息 (关键)
        DL_MCAN_writeRxFIFOAck(MCAN0_INST, rx_fs.num, rx_fs.getIdx);

        // 6. 处理收到的消息
        CAN_Rx_FIFO0_New_Message_Callback(rxMsg);
    }
}













