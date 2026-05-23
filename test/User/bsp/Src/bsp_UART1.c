#include "bsp_UART1.h"
#include "string.h"
#include "math.h"
#include "ti_msp_dl_config.h"


__IO uint16_t uart1_send_len = 0;
__IO uint16_t uart1_has_send_len = 0;
__IO uint8_t uart1_tx_complete_flag = 1;
__IO uint8_t uart1_tx_dma_complete_flag = 1;
__IO uint8_t uart1_rx_buff[UART1_RX_BUFF_SIZE];
__IO uint8_t uart1_rx_dma_complete_flag = 1;
__IO uint8_t uart1_rx_length = 0;


__attribute__((weak)) void UART1_Rx_DMA_ToIdle_Callback(uint16_t size) {}


void uart1_send(uint8_t *buff, uint16_t size) {
    if (uart1_tx_complete_flag) {
        DL_DMA_setSrcAddr(DMA, DMA_CH2_CHAN_ID, (uint32_t)(buff));
        DL_DMA_setDestAddr(DMA, DMA_CH2_CHAN_ID, (uint32_t)(&UART_1_INST->TXDATA));
        DL_DMA_setTransferSize(DMA, DMA_CH2_CHAN_ID, size);
        DL_DMA_enableChannel(DMA, DMA_CH2_CHAN_ID);
        uart1_tx_complete_flag = 0;
        uart1_tx_dma_complete_flag = 0;
    }
}

void uart1_receive_start(void) {
    if (uart1_rx_dma_complete_flag) {
        DL_DMA_setSrcAddr(DMA, DMA_CH3_CHAN_ID, (uint32_t)(&UART_1_INST->RXDATA));
        DL_DMA_setDestAddr(DMA, DMA_CH3_CHAN_ID, (uint32_t)(uart1_rx_buff));
        DL_DMA_setTransferSize(DMA, DMA_CH3_CHAN_ID, UART1_RX_BUFF_SIZE);
        DL_DMA_enableChannel(DMA, DMA_CH3_CHAN_ID);
        uart1_rx_dma_complete_flag = 0;
    }
}


__IO uint16_t uart1_rx_size = 0;
void UART1_IRQHandler(void) {
    volatile uint32_t res = DL_UART_Main_getPendingInterrupt(UART_1_INST);
    switch (res) {
        case DL_UART_IIDX_EOT_DONE: {
            uart1_tx_complete_flag = 1;
        } break;
        case DL_UART_MAIN_IIDX_DMA_DONE_TX:
            uart1_tx_dma_complete_flag = 1;
            break;
        case DL_UART_MAIN_IIDX_DMA_DONE_RX: {
            uart1_rx_size = UART1_RX_BUFF_SIZE - DL_DMA_getTransferSize(DMA, DMA_CH3_CHAN_ID);
            uart1_rx_length = uart1_rx_size;
            uart1_rx_dma_complete_flag = 1;
            UART1_Rx_DMA_ToIdle_Callback(uart1_rx_length);
        } break;
    }
}


void uart1_isIDLE(void) {
    static uint8_t rx_last_size = 0;
    static uint8_t stable_count = 0;

    if (uart1_rx_dma_complete_flag == 1) return;

    uart1_rx_length = UART1_RX_BUFF_SIZE - DL_DMA_getTransferSize(DMA, DMA_CH3_CHAN_ID);

    if (uart1_rx_length == rx_last_size && uart1_rx_length != 0) {
        stable_count++;
        if (stable_count >= 2) {
            uart1_rx_dma_complete_flag = 1;
            DL_DMA_disableChannel(DMA, DMA_CH3_CHAN_ID);
            stable_count = 0;
            UART1_Rx_DMA_ToIdle_Callback(uart1_rx_length);
        }
    } else {
        stable_count = 0;
        rx_last_size = uart1_rx_length;
    }
}
