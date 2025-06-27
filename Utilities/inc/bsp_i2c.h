/*!
    \file    bsp_i2c.h
    \brief   definitions I2C hardware resources

    \version 2025-06-05 V1.1.0, firmware for GD32G5x3
*/

#ifndef BSP_I2C_H
#define BSP_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "systick.h"
#include "at24cxx.h"
#include "gd32g5x3.h"
#include "gd32g5x3_libopt.h"

/* I²C0定义 */
#define I2C_SPEED       		400000
#define I2CX           		    I2C0
#define RCU_GPIO_I2C   		    RCU_GPIOB
#define RCU_I2C       		    RCU_I2C0
#define I2C_SCL_PORT    		GPIOB
#define I2C_SDA_PORT    		GPIOB
#define I2C_SCL_PIN     		GPIO_PIN_6
#define I2C_SDA_PIN     	    GPIO_PIN_7
#define I2C_GPIO_AF    			GPIO_AF_4

#define I2C_DMAx                DMA0
#define I2C_DMA_TX_CH           DMA_CH4
#define I2C_DMA_RX_CH           DMA_CH5
#define I2C_DMA_TX_REQUEST      DMA_REQUEST_I2C0_TX
#define I2C_DMA_RX_REQUEST      DMA_REQUEST_I2C0_RX
#define I2C_DMA_MUX_TX_CH       DMAMUX_MULTIPLEXER_CH4
#define I2C_DMA_MUX_RX_CH       DMAMUX_MULTIPLEXER_CH5
#define I2C_TX_DMA_IRQn         DMA0_Channel4_IRQn
#define I2C_RX_DMA_IRQn         DMA0_Channel5_IRQn

/* 函数声明 */
void bsp_i2c_init(void);
void i2c_gpio_reset(void);
void i2c_bus_reset(void);
void i2c_dma_tx_config(uint32_t mem_addr, uint32_t size);
void i2c_dma_rx_config(uint32_t mem_addr, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* BSP_I2C_H */
