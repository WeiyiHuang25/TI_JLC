#include "bsp_UART.h"
#include "string.h"
#include "math.h"
#include "ti_msp_dl_config.h"


__IO uint16_t send_len = 0;
__IO uint16_t has_send_len = 0;
__IO uint8_t uart_tx_complete_flag = 1;  // 初始标志为已完成（允许新任务）
__IO uint8_t uart_tx_dma_complete_flag = 1; 
__IO uint8_t uart_rx_buff[UART_RX_BUFF_SIZE];
__IO uint8_t uart_rx_dma_complete_flag = 1;
__IO uint8_t uart_rx_length = 0;


void UART_Rx_DMA_ToIdle_Callback(uint16_t size);


void uart_send(uint8_t *buff, uint16_t size) {
    //当串口发送完毕后，才可再次发送
    if(uart_tx_complete_flag)
    {
        //设置源地址
        DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(buff));
    
        //设置目标地址
        DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(&UART_0_INST->TXDATA));
    
        //设置要搬运的字节数
        DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, size);
    
        //使能DMA通道
        DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
    
        uart_tx_complete_flag    = 0;
        uart_tx_dma_complete_flag = 0;
    }
}

void uart_receive_start() {
    if(uart_rx_dma_complete_flag)
    {
        //设置源地址
        DL_DMA_setSrcAddr(DMA, DMA_CH1_CHAN_ID, (uint32_t)(&UART_0_INST->RXDATA));
    
        //设置目标地址
        DL_DMA_setDestAddr(DMA, DMA_CH1_CHAN_ID, (uint32_t)(uart_rx_buff));
        //设置要搬运的字节数
        DL_DMA_setTransferSize(DMA, DMA_CH1_CHAN_ID, UART_RX_BUFF_SIZE);
    
        //使能DMA通道
        DL_DMA_enableChannel(DMA, DMA_CH1_CHAN_ID);
    
        uart_rx_dma_complete_flag = 0 ; 
    }
}



__IO uint16_t rx_size = 0;
void UART0_IRQHandler(){
    volatile uint32_t res = DL_UART_Main_getPendingInterrupt(UART_0_INST);
    switch (res) {
        case DL_UART_IIDX_EOT_DONE:{
            uart_tx_complete_flag = 1;
        }break;
        case DL_UART_MAIN_IIDX_DMA_DONE_TX:
            uart_tx_dma_complete_flag = 1;
        break;
        case DL_UART_MAIN_IIDX_DMA_DONE_RX:{
            rx_size = UART_RX_BUFF_SIZE - DL_DMA_getTransferSize(DMA,DMA_CH1_CHAN_ID);
            uart_rx_length = rx_size;
            uart_rx_dma_complete_flag = 1;
            UART_Rx_DMA_ToIdle_Callback(uart_rx_length);
        }break;
    }
}







void uart_isIDLE(){
    static uint8_t rx_last_size = 0;
    static uint8_t stable_count = 0;
    
    if(uart_rx_dma_complete_flag == 1) return;

    uart_rx_length = UART_RX_BUFF_SIZE - DL_DMA_getTransferSize(DMA, DMA_CH1_CHAN_ID);

    if (uart_rx_length == rx_last_size && uart_rx_length != 0) {
        stable_count++;
        if (stable_count >= 2) {  // 连续两次一致，确认接收完毕
            uart_rx_dma_complete_flag = 1;
            DL_DMA_disableChannel(DMA, DMA_CH1_CHAN_ID);
            stable_count = 0; // 重置计数器
            UART_Rx_DMA_ToIdle_Callback(uart_rx_length);
        }
    } else {
        stable_count = 0;
        rx_last_size = uart_rx_length;
    }
}

void UART_Rx_DMA_ToIdle_Callback(uint16_t size)
{
    uint8_t buffer[size];
    memcpy(buffer, (void *)uart_rx_buff, size);
    uart_send(buffer, size);
    uart_receive_start();
}
