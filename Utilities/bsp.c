/*!
    \file    bsp.c
    \brief   This is the main file of the underlying hardware driver.
             Each c file can be #include "bsp.h" to include all peripheral driver modules.

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp.h"

void bsp_init(void)
{
    bsp_pwm_init();   // PWM初始化
    bsp_adc_init();   // ADC初始化
    bsp_spi_init();   // SPI初始化
    bsp_i2c_init();   // I2C初始化
    bsp_rs232_init(); // RS232初始化
    bsp_rs485_init(); // RS485初始化
    nvic_config();    // 中断配置
}

void init_periheral(unsigned int baudrate)
{
    bsp_i2c_init();              // I2C初始化
	bsp_rs485_init1(baudrate, PTY_NONE, DMA_ENABLE);
	
	nvic_config();              // 中断配置,有需删除的
	__enable_irq();             // 开中断
}	



void nvic_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);

    // ADC 中断
    nvic_irq_enable(IU_ADC_IRQn, 0, 1);
    nvic_irq_enable(IV_ADC_IRQn, 0, 1);
    nvic_irq_enable(IW_ADC_IRQn, 0, 1);
    nvic_irq_enable(VOLTAGE_ADC_DMA_IRQn, 0, 1);

    // 定时器0的比较中断
    nvic_irq_enable(TIMER0_Channel_IRQn, 1, 1);

    // RS232中断配置
    nvic_irq_enable(RS232_IRQn, 1, 1);        // USART0
    nvic_irq_enable(RS232_TX_DMA_IRQn, 1, 1); // RS232 TX

    // RS485中断配置
    nvic_irq_enable(RS485_IRQn, 1, 1);        // USART1
    nvic_irq_enable(RS485_TX_DMA_IRQn, 1, 1); // RS485 TX

    // I²C中断配置
    nvic_irq_enable(I2C_TX_DMA_IRQn, 1, 1); // I²C TX
    nvic_irq_enable(I2C_RX_DMA_IRQn, 1, 1); // I²C RX

    // SPI2 DMA中断配置
    nvic_irq_enable(SPI2_TX_DMA_IRQn, 1, 1); // SPI2 TX 
    nvic_irq_enable(SPI2_RX_DMA_IRQn, 1, 1); // SPI2 RX 
}
