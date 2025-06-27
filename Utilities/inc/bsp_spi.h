/*!
    \file    bsp_spi.h
    \brief   definitions SPI hardware resources

    \version 2025-06-05 V1.1.0, firmware for GD32G5x3
*/

#ifndef BSP_SPI_H
#define BSP_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32g5x3.h"
#include "gd32g5x3_libopt.h"

/* SPI定义 */
#define SPI2_PERIPH                      SPI2
#define SPI2_CLK                         RCU_SPI2
#define SPI2_DMA                         DMA1
#define SPI2_DMA_CLK                     RCU_DMA1

#define SPI2_RX_DMA_CH                   DMA_CH0
#define SPI2_RX_DMA_IRQn                 DMA1_Channel0_IRQn
#define SPI2_RX_DMA_REQUEST              DMA_REQUEST_SPI2_RX

#define SPI2_TX_DMA_CH                   DMA_CH1  
#define SPI2_TX_DMA_IRQn                 DMA1_Channel1_IRQn
#define SPI2_TX_DMA_REQUEST              DMA_REQUEST_SPI2_TX

/* SPI引脚 */
#define SPI2_GPIO_CLK                    RCU_GPIOC
#define SPI2_SCK_PORT                    GPIOC
#define SPI2_SCK_PIN                     GPIO_PIN_10
#define SPI2_MISO_PORT                   GPIOC
#define SPI2_MISO_PIN                    GPIO_PIN_11

/* 缓冲区 */
#define SPI_BUFFER_SIZE                  5
#define SSI_DATA_SIZE                    4  // 每次发送的数据长度

/* 全局变量声明  */
extern uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
extern uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
extern __IO FlagStatus spi_rx_complete_flag;   

/* 函数声明  */
void bsp_spi_init(void);
void bsp_spi_dma_init(void);
void bsp_spi_rcu_config(void);
void bsp_spi_gpio_config(void);
void bsp_spi_periph_config(void);
void bsp_spi_transfer(uint32_t size); 
void process_spi_data(uint8_t *data, uint32_t size);
	
#ifdef __cplusplus
}
#endif

#endif /* BSP_SPI_H */