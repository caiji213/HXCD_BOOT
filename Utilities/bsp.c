/*!
    \file    bsp.c
    \brief   This is the main file of the underlying hardware driver.
             Each c file can be #include "bsp.h" to include all peripheral driver modules.

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp.h"

void bsp_deinit(void)
{
    dma_deinit_all();       // 复位所有 DMA 通道
    uart_deinit_all();      // 复位所有串口
    gpio_deinit_all();      // 复位所有 GPIO 端口
    timer_deinit_all();     // 复位所有定时器
    i2c_deinit_all();       // 复位所有 I2C 总线
}

void bsp_init(unsigned int baudrate)
{
    bsp_i2c_init();              // I2C初始化
	bsp_rs232_init();
	bsp_rs485_init(baudrate, PTY_NONE, DMA_ENABLE);
	
	nvic_config();              // 中断配置,有需删除的
	__enable_irq();             // 开中断
}	

void nvic_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);


    // RS232中断配置
    nvic_irq_enable(RS232_IRQn, 1, 1);        // USART0
    nvic_irq_enable(RS232_TX_DMA_IRQn, 1, 1); // RS232 TX

    // RS485中断配置
    nvic_irq_enable(RS485_IRQn, 1, 1);        // USART1
    nvic_irq_enable(RS485_TX_DMA_IRQn, 1, 1); // RS485 TX

    // I²C中断配置
    nvic_irq_enable(I2C_TX_DMA_IRQn, 1, 1); // I²C TX
    nvic_irq_enable(I2C_RX_DMA_IRQn, 1, 1); // I²C RX
}



