/*
 * HEE_board.h
 *
 *  Created on: Jul 27, 2024
 *      Author: duong
 */

#ifndef BOARD_H_
#define BOARD_H_

/**************CURRENT MONITOR*****************/
#define ADC_I_SENSE_HANDLE      ADC1
#define ADC_I_SENSE_CHANNEL     LL_ADC_CHANNEL_2
#define ADC_I_SENSE_IRQ         ADC1_IRQn

#define OCP_ALERT_PORT          GPIOC
#define OCP_ALERT_PIN           LL_GPIO_PIN_13

#define I_SENSE_RESET_PORT      GPIOC
#define I_SENSE_RESET_PIN       LL_GPIO_PIN_14
#define ADC_CHANNEL_COUNT		1
/***********************************************/

/*********************H BRIDGE******************/
#define H_BRIDGE_SD1_HANDLE     TIM14
#define H_BRIDGE_SD1_CHANNEL    LL_TIM_CHANNEL_CH1

#define H_BRIDGE_HIN1_PORT      GPIOA
#define H_BRIDGE_HIN1_PIN       LL_GPIO_PIN_6

#define H_BRIDGE_SD2_HANDLE     TIM15
#define H_BRIDGE_SD2_CHANNEL    LL_TIM_CHANNEL_CH2

#define H_BRIDGE_HIN2_PORT      GPIOA
#define H_BRIDGE_HIN2_PIN       LL_GPIO_PIN_5
/***********************************************/

/*****************Voltage Switching*************/
#define V_SWITCH_LIN1_HANDLE    TIM3
#define V_SWITCH_LIN1_CHANNEL   LL_TIM_CHANNEL_CH2

#define V_SWITCH_HIN1_PORT      GPIOB
#define V_SWITCH_HIN1_PIN       LL_GPIO_PIN_0

#define V_SWITCH_LIN2_HANDLE    TIM16
#define V_SWITCH_LIN2_CHANNEL   LL_TIM_CHANNEL_CH1

#define V_SWITCH_HIN2_PORT      GPIOB
#define V_SWITCH_HIN2_PIN       LL_GPIO_PIN_9
/***********************************************/

/***********************RELAY*******************/
#define RELAY_SW_PORT           GPIOB
#define RELAY_SW_PIN            LL_GPIO_PIN_1
/***********************************************/

/*********************UART**********************/
#define RS232_UART_HANDLE       USART3
#define RS232_UART_IRQ          USART3_6_IRQn

#define GPC_UART_HANDLE         USART1
#define GPC_UART_IRQ            USART1_IRQn
/***********************************************/

/**********************SPI**********************/
#define DAC_SPI_HANDLE          SPI1

#define DAC_SPI_CS_PORT         GPIOA
#define DAC_SPI_CS_PIN          LL_GPIO_PIN_15

#define DAC_nLDAC_PORT          GPIOB
#define DAC_nLDAC_PIN           LL_GPIO_PIN_4
/***********************************************/

/**********************I2C**********************/
#define I2C_HANDLE              I2C2
/***********************************************/

/*******************DEBUG LED*******************/
#define DEBUG_LED_PORT          GPIOB
#define DEBUG_LED_PIN           LL_GPIO_PIN_12
/***********************************************/

/********************DECODER********************/
#define DECOD_LS0_PORT          GPIOA
#define DECOD_LS0_PIN           LL_GPIO_PIN_9

#define DECOD_LS1_PORT          GPIOA
#define DECOD_LS1_PIN           LL_GPIO_PIN_10

#define DECOD_LS2_PORT          GPIOA
#define DECOD_LS2_PIN           LL_GPIO_PIN_11

#define DECOD_LS_EN_PORT        GPIOA
#define DECOD_LS_EN_PIN         LL_GPIO_PIN_12

#define DECOD_HS0_PORT          GPIOA
#define DECOD_HS0_PIN           LL_GPIO_PIN_1

#define DECOD_HS1_PORT          GPIOA
#define DECOD_HS1_PIN           LL_GPIO_PIN_0

#define DECOD_HS2_PORT          GPIOB
#define DECOD_HS2_PIN           LL_GPIO_PIN_15

#define DECOD_HS_EN_PORT        GPIOA
#define DECOD_HS_EN_PIN         LL_GPIO_PIN_8
/***********************************************/

#define GPC_TX_BUF_LEN			64
#define	GPC_RX_BUF_LEN			64
#define FSP_BUF_LEN				64

#endif /* BOARD_H_ */
