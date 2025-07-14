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
	ccr_deinit_all();       // 复位硬件CCR
}

void bsp_init()
{
	bsp_rs232_init(115200, PTY_NONE, DMA_ENABLE);
	bsp_rs485_init(115200, PTY_NONE, DMA_ENABLE);
	bsp_i2c_init();              // I2C初始化
	nvic_config();               // 中断配置
	__enable_irq();              // 开中断 
}	

void bsp_crc_init()
{
	/* enable CRC clock */
    rcu_periph_clock_enable(RCU_CRC);// 开硬件crc的时钟
	/* deinit CRC calculation unit */
    crc_deinit();
    
    // 配置CRC模块（仅在初始化时配置一次）
    crc_polynomial_size_set(CRC_CTL_PS_32);           // 32位多项式
    crc_input_data_reverse_config(CRC_INPUT_DATA_NOT); // 无输入反转
    crc_reverse_output_data_disable();                // 无输出反转
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



