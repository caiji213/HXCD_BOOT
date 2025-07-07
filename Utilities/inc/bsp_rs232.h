/*!
    \file    bsp_rs232.h
    \brief   definitions RS232 ports hardware resources

    \version 2025-06-05 V1.1.0, firmware for GD32G5x3
*/

#ifndef BSP_RS232_H
#define BSP_RS232_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32g5x3.h"
#include "gd32g5x3_libopt.h"

#define RS232_BUFFER_SIZE               256
#define RS232_DMA_SIZE                  RS232_BUFFER_SIZE + 1

/* USART0定义 */
#define RS232_COM                        USART0
#define RS232_COM_CLK                    RCU_USART0
#define RS232_TX_PIN                     GPIO_PIN_9
#define RS232_RX_PIN                     GPIO_PIN_10
#define RS232_GPIO_PORT                  GPIOA
#define RS232_GPIO_CLK                   RCU_GPIOA
#define RS232_AF                         GPIO_AF_7
#define RS232_IRQn                       USART0_IRQn
#define RS232_RDATA_ADDRESS              (&USART_RDATA(USART0))
#define RS232_TDATA_ADDRESS              (&USART_TDATA(USART0))

/* DMA定义 */
#define RS232_TX_DMA_CLK                 RCU_DMA0  
#define RS232_TX_DMA                     DMA0
#define RS232_TX_DMA_CH                  DMA_CH0
#define RS232_TX_DMA_IRQn                DMA0_Channel0_IRQn

#define RS232_RX_DMA_CLK                 RCU_DMA0  
#define RS232_RX_DMA                     DMA0
#define RS232_RX_DMA_CH                  DMA_CH1  
#define RS232_RX_DMA_IRQn                DMA0_Channel1_IRQn

#define PTY_NONE			             0
#define PTY_ODD					         1
#define PTY_EVEN				         2

#define DMA_ENABLE				          1
#define DMA_DISENABLE			          0

/* 全局变量定义 */
extern uint8_t rs232_rx_count;                             // 接收数据计数器
extern uint8_t tx_count;                             // 发送数据计数器
extern uint8_t rs232_rxbuffer[RS232_BUFFER_SIZE];    // 接收缓冲区
extern __IO FlagStatus rs232_idle_flag;              // USART总线空闲标志
extern __IO FlagStatus rs232_tx_flag;                // UART发送完成标志

/* 函数声明  */
void bsp_rs232_init(uint32_t Baudrate, uint16_t parity_bit, uint8_t DMA_enable);
void bsp_rs232_dma_init(void);
void bsp_rs232_dma_send(uint8_t *buffer, uint32_t length);


#ifdef __cplusplus
}
#endif

#endif /* BSP_RS232_H */
