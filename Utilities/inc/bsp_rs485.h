/*!
    \file    bsp_rs485.h
    \brief   definitions RS485 ports hardware resources

    \version 2025-06-05 V1.1.0, firmware for GD32G5x3
*/

#ifndef BSP_RS485_H
#define BSP_RS485_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32g5x3.h"
#include "gd32g5x3_libopt.h"

#define RS485_BUFFER_SIZE               256
#define RS485_DMA_SIZE                  RS485_BUFFER_SIZE + 1

/* USART1定义 */
#define RS485_COM                        USART1
#define RS485_COM_CLK                    RCU_USART1
#define RS485_TX_PIN                     GPIO_PIN_2
#define RS485_RX_PIN                     GPIO_PIN_3
#define RS485_GPIO_PORT                  GPIOA
#define RS485_GPIO_CLK                   RCU_GPIOA
#define RS485_AF                         GPIO_AF_7
#define RS485_RE_PIN                     GPIO_PIN_1
#define RS485_IRQn                       USART1_IRQn
#define RS485_RDATA_ADDRESS              (&USART_RDATA(USART1))
#define RS485_TDATA_ADDRESS              (&USART_TDATA(USART1))

/* DMA定义 */
#define RS485_RX_DMA_CLK                 RCU_DMA0
#define RS485_RX_DMA                     DMA0
#define RS485_RX_DMA_CH                  DMA_CH2
#define RS485_RX_DMA_IRQn                DMA0_Channel2_IRQn

#define RS485_TX_DMA_CLK                 RCU_DMA0
#define RS485_TX_DMA                     DMA0
#define RS485_TX_DMA_CH                  DMA_CH3
#define RS485_TX_DMA_IRQn                DMA0_Channel3_IRQn

/* 全局变量 */
extern uint8_t rs485_rx_count;
extern uint8_t rs485_rxbuffer[RS485_BUFFER_SIZE];
extern __IO FlagStatus rs485_idle_flag;
extern __IO FlagStatus rs485_tx_flag;

/* 函数声明  */
void bsp_rs485_init(void);
void bsp_rs485_init1(uint32_t Baudrate, uint16_t parity_bit, uint8_t DMA_enable);
void bsp_rs485_dma_init(void);
void bsp_rs485_dma_send(uint8_t *buffer, uint32_t length);
void bsp_rs485_send_mode(void);
void bsp_rs485_receive_mode(void);

#define UARTX_RX_BUF_LENGTH		16
#define UARTX_TX_BUF_LENGTH		16

#define UART_BAUD_9600			0
#define UART_BAUD_19200			1
#define UART_BAUD_38400			2
#define UART_BAUD_57600			3
#define UART_BAUD_115200		4
#define UART_BAUD_230400		5
#define UART_BAUD_921600		6
#define UART_BAUD_1843200		7

#define PTY_NONE				0
#define PTY_ODD					1
#define PTY_EVEN				2

#define DMA_ENABLE				1
#define DMA_DISENABLE			0

#ifdef __cplusplus
}
#endif

#endif /* BSP_RS485_H */