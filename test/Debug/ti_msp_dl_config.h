/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define GPIO_LFXT_PORT                                                     GPIOA
#define GPIO_LFXIN_PIN                                             DL_GPIO_PIN_3
#define GPIO_LFXIN_IOMUX                                          (IOMUX_PINCM8)
#define GPIO_LFXOUT_PIN                                            DL_GPIO_PIN_4
#define GPIO_LFXOUT_IOMUX                                         (IOMUX_PINCM9)
#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2000
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for FRONT_OUT */
#define FRONT_OUT_INST                                                     TIMA0
#define FRONT_OUT_INST_IRQHandler                               TIMA0_IRQHandler
#define FRONT_OUT_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define FRONT_OUT_INST_CLK_FREQ                                          1000000
/* GPIO defines for channel 0 */
#define GPIO_FRONT_OUT_C0_PORT                                             GPIOA
#define GPIO_FRONT_OUT_C0_PIN                                      DL_GPIO_PIN_0
#define GPIO_FRONT_OUT_C0_IOMUX                                   (IOMUX_PINCM1)
#define GPIO_FRONT_OUT_C0_IOMUX_FUNC                  IOMUX_PINCM1_PF_TIMA0_CCP0
#define GPIO_FRONT_OUT_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_FRONT_OUT_C1_PORT                                             GPIOA
#define GPIO_FRONT_OUT_C1_PIN                                      DL_GPIO_PIN_1
#define GPIO_FRONT_OUT_C1_IOMUX                                   (IOMUX_PINCM2)
#define GPIO_FRONT_OUT_C1_IOMUX_FUNC                  IOMUX_PINCM2_PF_TIMA0_CCP1
#define GPIO_FRONT_OUT_C1_IDX                                DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_FRONT_OUT_C2_PORT                                             GPIOA
#define GPIO_FRONT_OUT_C2_PIN                                     DL_GPIO_PIN_15
#define GPIO_FRONT_OUT_C2_IOMUX                                  (IOMUX_PINCM37)
#define GPIO_FRONT_OUT_C2_IOMUX_FUNC                 IOMUX_PINCM37_PF_TIMA0_CCP2
#define GPIO_FRONT_OUT_C2_IDX                                DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_FRONT_OUT_C3_PORT                                             GPIOA
#define GPIO_FRONT_OUT_C3_PIN                                     DL_GPIO_PIN_12
#define GPIO_FRONT_OUT_C3_IOMUX                                  (IOMUX_PINCM34)
#define GPIO_FRONT_OUT_C3_IOMUX_FUNC                 IOMUX_PINCM34_PF_TIMA0_CCP3
#define GPIO_FRONT_OUT_C3_IDX                                DL_TIMER_CC_3_INDEX

/* Defines for RL_OUT */
#define RL_OUT_INST                                                        TIMA1
#define RL_OUT_INST_IRQHandler                                  TIMA1_IRQHandler
#define RL_OUT_INST_INT_IRQN                                    (TIMA1_INT_IRQn)
#define RL_OUT_INST_CLK_FREQ                                             1000000
/* GPIO defines for channel 0 */
#define GPIO_RL_OUT_C0_PORT                                                GPIOA
#define GPIO_RL_OUT_C0_PIN                                        DL_GPIO_PIN_17
#define GPIO_RL_OUT_C0_IOMUX                                     (IOMUX_PINCM39)
#define GPIO_RL_OUT_C0_IOMUX_FUNC                    IOMUX_PINCM39_PF_TIMA1_CCP0
#define GPIO_RL_OUT_C0_IDX                                   DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_RL_OUT_C1_PORT                                                GPIOA
#define GPIO_RL_OUT_C1_PIN                                        DL_GPIO_PIN_16
#define GPIO_RL_OUT_C1_IOMUX                                     (IOMUX_PINCM38)
#define GPIO_RL_OUT_C1_IOMUX_FUNC                    IOMUX_PINCM38_PF_TIMA1_CCP1
#define GPIO_RL_OUT_C1_IDX                                   DL_TIMER_CC_1_INDEX

/* Defines for RR_OUT */
#define RR_OUT_INST                                                        TIMG6
#define RR_OUT_INST_IRQHandler                                  TIMG6_IRQHandler
#define RR_OUT_INST_INT_IRQN                                    (TIMG6_INT_IRQn)
#define RR_OUT_INST_CLK_FREQ                                             1000000
/* GPIO defines for channel 0 */
#define GPIO_RR_OUT_C0_PORT                                                GPIOA
#define GPIO_RR_OUT_C0_PIN                                        DL_GPIO_PIN_21
#define GPIO_RR_OUT_C0_IOMUX                                     (IOMUX_PINCM46)
#define GPIO_RR_OUT_C0_IOMUX_FUNC                    IOMUX_PINCM46_PF_TIMG6_CCP0
#define GPIO_RR_OUT_C0_IDX                                   DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_RR_OUT_C1_PORT                                                GPIOA
#define GPIO_RR_OUT_C1_PIN                                        DL_GPIO_PIN_22
#define GPIO_RR_OUT_C1_IOMUX                                     (IOMUX_PINCM47)
#define GPIO_RR_OUT_C1_IOMUX_FUNC                    IOMUX_PINCM47_PF_TIMG6_CCP1
#define GPIO_RR_OUT_C1_IDX                                   DL_TIMER_CC_1_INDEX



/* Defines for US_TIMER */
#define US_TIMER_INST                                                   (TIMG12)
#define US_TIMER_INST_IRQHandler                               TIMG12_IRQHandler
#define US_TIMER_INST_INT_IRQN                                 (TIMG12_INT_IRQn)
#define US_TIMER_INST_LOAD_VALUE                                            (0U)




/* Defines for I2C_OLED */
#define I2C_OLED_INST                                                       I2C1
#define I2C_OLED_INST_IRQHandler                                 I2C1_IRQHandler
#define I2C_OLED_INST_INT_IRQN                                     I2C1_INT_IRQn
#define I2C_OLED_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_OLED_SDA_PORT                                             GPIOB
#define GPIO_I2C_OLED_SDA_PIN                                      DL_GPIO_PIN_3
#define GPIO_I2C_OLED_IOMUX_SDA                                  (IOMUX_PINCM16)
#define GPIO_I2C_OLED_IOMUX_SDA_FUNC                   IOMUX_PINCM16_PF_I2C1_SDA
#define GPIO_I2C_OLED_SCL_PORT                                             GPIOB
#define GPIO_I2C_OLED_SCL_PIN                                      DL_GPIO_PIN_2
#define GPIO_I2C_OLED_IOMUX_SCL                                  (IOMUX_PINCM15)
#define GPIO_I2C_OLED_IOMUX_SCL_FUNC                   IOMUX_PINCM15_PF_I2C1_SCL


/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           40000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_40_MHZ_115200_BAUD                                      (21)
#define UART_0_FBRD_40_MHZ_115200_BAUD                                      (46)
/* Defines for UART_1 */
#define UART_1_INST                                                        UART1
#define UART_1_INST_FREQUENCY                                           40000000
#define UART_1_INST_IRQHandler                                  UART1_IRQHandler
#define UART_1_INST_INT_IRQN                                      UART1_INT_IRQn
#define GPIO_UART_1_RX_PORT                                                GPIOA
#define GPIO_UART_1_TX_PORT                                                GPIOA
#define GPIO_UART_1_RX_PIN                                         DL_GPIO_PIN_9
#define GPIO_UART_1_TX_PIN                                         DL_GPIO_PIN_8
#define GPIO_UART_1_IOMUX_RX                                     (IOMUX_PINCM20)
#define GPIO_UART_1_IOMUX_TX                                     (IOMUX_PINCM19)
#define GPIO_UART_1_IOMUX_RX_FUNC                      IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART_1_IOMUX_TX_FUNC                      IOMUX_PINCM19_PF_UART1_TX
#define UART_1_BAUD_RATE                                                  (9600)
#define UART_1_IBRD_40_MHZ_9600_BAUD                                       (260)
#define UART_1_FBRD_40_MHZ_9600_BAUD                                        (27)





/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (3)
#define UART_0_INST_DMA_TRIGGER_0                            (DMA_UART0_TX_TRIG)
/* Defines for DMA_CH1 */
#define DMA_CH1_CHAN_ID                                                      (2)
#define UART_0_INST_DMA_TRIGGER_1                            (DMA_UART0_RX_TRIG)
/* Defines for DMA_CH2 */
#define DMA_CH2_CHAN_ID                                                      (1)
#define UART_1_INST_DMA_TRIGGER_0                            (DMA_UART1_TX_TRIG)
/* Defines for DMA_CH3 */
#define DMA_CH3_CHAN_ID                                                      (0)
#define UART_1_INST_DMA_TRIGGER_1                            (DMA_UART1_RX_TRIG)


/* Port definition for Pin Group LED */
#define LED_PORT                                                         (GPIOB)

/* Defines for LED_PIN: GPIOB.22 with pinCMx 50 on package pin 21 */
#define LED_LED_PIN_PIN                                         (DL_GPIO_PIN_22)
#define LED_LED_PIN_IOMUX                                        (IOMUX_PINCM50)
/* Defines for A18: GPIOA.18 with pinCMx 40 on package pin 11 */
#define KEY_A18_PORT                                                     (GPIOA)
// pins affected by this interrupt request:["A18"]
#define KEY_GPIOA_INT_IRQN                                      (GPIOA_INT_IRQn)
#define KEY_GPIOA_INT_IIDX                      (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define KEY_A18_IIDX                                        (DL_GPIO_IIDX_DIO18)
#define KEY_A18_PIN                                             (DL_GPIO_PIN_18)
#define KEY_A18_IOMUX                                            (IOMUX_PINCM40)
/* Defines for B21: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_B21_PORT                                                     (GPIOB)
// groups represented: ["ULTRASONIC","KEY"]
// pins affected: ["ULTRASONIC_0","ULTRASONIC_1","B21"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define KEY_B21_IIDX                                        (DL_GPIO_IIDX_DIO21)
#define KEY_B21_PIN                                             (DL_GPIO_PIN_21)
#define KEY_B21_IOMUX                                            (IOMUX_PINCM49)
/* Port definition for Pin Group ULTRASONIC */
#define ULTRASONIC_PORT                                                  (GPIOB)

/* Defines for ULTRASONIC_0: GPIOB.13 with pinCMx 30 on package pin 1 */
#define ULTRASONIC_ULTRASONIC_0_IIDX                        (DL_GPIO_IIDX_DIO13)
#define ULTRASONIC_ULTRASONIC_0_PIN                             (DL_GPIO_PIN_13)
#define ULTRASONIC_ULTRASONIC_0_IOMUX                            (IOMUX_PINCM30)
/* Defines for ULTRASONIC_1: GPIOB.14 with pinCMx 31 on package pin 2 */
#define ULTRASONIC_ULTRASONIC_1_IIDX                        (DL_GPIO_IIDX_DIO14)
#define ULTRASONIC_ULTRASONIC_1_PIN                             (DL_GPIO_PIN_14)
#define ULTRASONIC_ULTRASONIC_1_IOMUX                            (IOMUX_PINCM31)
/* Defines for TRIG_0: GPIOB.15 with pinCMx 32 on package pin 3 */
#define ULTRASONIC_TRIG_0_PIN                                   (DL_GPIO_PIN_15)
#define ULTRASONIC_TRIG_0_IOMUX                                  (IOMUX_PINCM32)
/* Defines for TRIG_1: GPIOB.16 with pinCMx 33 on package pin 4 */
#define ULTRASONIC_TRIG_1_PIN                                   (DL_GPIO_PIN_16)
#define ULTRASONIC_TRIG_1_IOMUX                                  (IOMUX_PINCM33)




/* Defines for MCAN0 */
#define MCAN0_INST                                                        CANFD0
#define GPIO_MCAN0_CAN_TX_PORT                                             GPIOA
#define GPIO_MCAN0_CAN_TX_PIN                                     DL_GPIO_PIN_26
#define GPIO_MCAN0_IOMUX_CAN_TX                                  (IOMUX_PINCM59)
#define GPIO_MCAN0_IOMUX_CAN_TX_FUNC               IOMUX_PINCM59_PF_CANFD0_CANTX
#define GPIO_MCAN0_CAN_RX_PORT                                             GPIOA
#define GPIO_MCAN0_CAN_RX_PIN                                     DL_GPIO_PIN_27
#define GPIO_MCAN0_IOMUX_CAN_RX                                  (IOMUX_PINCM60)
#define GPIO_MCAN0_IOMUX_CAN_RX_FUNC               IOMUX_PINCM60_PF_CANFD0_CANRX
#define MCAN0_INST_IRQHandler                                 CANFD0_IRQHandler
#define MCAN0_INST_INT_IRQN                                     CANFD0_INT_IRQn


/* Defines for MCAN0 MCAN RAM configuration */
#define MCAN0_INST_MCAN_STD_ID_FILT_START_ADDR     (0)
#define MCAN0_INST_MCAN_STD_ID_FILTER_NUM          (1)
#define MCAN0_INST_MCAN_EXT_ID_FILT_START_ADDR     (48)
#define MCAN0_INST_MCAN_EXT_ID_FILTER_NUM          (1)
#define MCAN0_INST_MCAN_TX_BUFF_START_ADDR         (148)
#define MCAN0_INST_MCAN_TX_BUFF_SIZE               (8)
#define MCAN0_INST_MCAN_FIFO_1_START_ADDR          (192)
#define MCAN0_INST_MCAN_FIFO_1_NUM                 (2)
#define MCAN0_INST_MCAN_TX_EVENT_START_ADDR        (164)
#define MCAN0_INST_MCAN_TX_EVENT_SIZE              (2)
#define MCAN0_INST_MCAN_EXT_ID_AND_MASK            (0x1FFFFFFFU)
#define MCAN0_INST_MCAN_RX_BUFF_START_ADDR         (208)
#define MCAN0_INST_MCAN_FIFO_0_START_ADDR          (172)
#define MCAN0_INST_MCAN_FIFO_0_NUM                 (8)

#define MCAN0_INST_MCAN_INTERRUPTS (DL_MCAN_INTERRUPT_RF0N)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_FRONT_OUT_init(void);
void SYSCFG_DL_RL_OUT_init(void);
void SYSCFG_DL_RR_OUT_init(void);
void SYSCFG_DL_US_TIMER_init(void);
void SYSCFG_DL_I2C_OLED_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_1_init(void);
void SYSCFG_DL_DMA_init(void);

void SYSCFG_DL_SYSTICK_init(void);
void SYSCFG_DL_MCAN0_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
