/*!
    \file    bsp_deinit.c
    \brief   This file implements the peripheral deinitialization functions
             for GD32G5x3 series firmware.

    \version 2025-06-05, V1.0.0
*/

#include "bsp_deinit.h"
#include "gd32g5x3.h"

/* DMA所有通道复位 */
void dma_deinit_all(void)
{
    // DMA0所有通道
    dma_deinit(DMA0, DMA_CH0);
    dma_deinit(DMA0, DMA_CH1);
    dma_deinit(DMA0, DMA_CH2);
    dma_deinit(DMA0, DMA_CH3);
    dma_deinit(DMA0, DMA_CH4);
    dma_deinit(DMA0, DMA_CH5);
    dma_deinit(DMA0, DMA_CH6);

    // DMA1所有通道
    dma_deinit(DMA1, DMA_CH0);
    dma_deinit(DMA1, DMA_CH1);
    dma_deinit(DMA1, DMA_CH2);
    dma_deinit(DMA1, DMA_CH3);
    dma_deinit(DMA1, DMA_CH4);
    dma_deinit(DMA1, DMA_CH5);
    dma_deinit(DMA1, DMA_CH6);
}

/* 串口外设全部复位 */
void uart_deinit_all(void)
{
    usart_deinit(USART0);
    usart_deinit(USART1);
    usart_deinit(USART2);
    usart_deinit(UART3);
    usart_deinit(UART4);
}

/* GPIO全部复位 */
void gpio_deinit_all(void)
{
    gpio_deinit(GPIOA);
    gpio_deinit(GPIOB);
    gpio_deinit(GPIOC);
    gpio_deinit(GPIOD);
    gpio_deinit(GPIOE);
    gpio_deinit(GPIOF);
    gpio_deinit(GPIOG);
}

/* 定时器全部复位 */
void timer_deinit_all(void)
{
    timer_deinit(TIMER0);
    timer_deinit(TIMER1);
    timer_deinit(TIMER2);
    timer_deinit(TIMER3);
    timer_deinit(TIMER4);
    timer_deinit(TIMER5);
    timer_deinit(TIMER6);
    timer_deinit(TIMER7);
    timer_deinit(TIMER14);
    timer_deinit(TIMER15);
    timer_deinit(TIMER16);
    timer_deinit(TIMER19);
}


/* I2C全部复位 */
void i2c_deinit_all(void)
{
    i2c_deinit(I2C0);
    i2c_deinit(I2C1);
    i2c_deinit(I2C2);
	i2c_deinit(I2C3); 
}
