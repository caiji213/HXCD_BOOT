/*!
    \file    gd32g5x3_it.h
    \brief   the header file of the ISR

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#ifndef GD32G5X3_IT_H
#define GD32G5X3_IT_H

#include "main.h"
#include "systick.h"
#include "gd32g5x3.h"
#include "bsp.h"

extern volatile uint32_t g_sys_tick;

/* function declarations */
/* 此函数处理NMI异常 */
void NMI_Handler(void);
/* 此函数处理HardFault异常 */
void HardFault_Handler(void);
/* 此函数处理内存管理异常 */
void MemManage_Handler(void);
/* 此函数处理总线错误异常 */
void BusFault_Handler(void);
/* 此函数处理使用错误异常 */
void UsageFault_Handler(void);
/* 此函数处理SVC异常 */
void SVC_Handler(void);
/* 此函数处理调试监控异常 */
void DebugMon_Handler(void);
/* 此函数处理PendSV异常 */
void PendSV_Handler(void);
/* 此函数处理SysTick异常 */
void SysTick_Handler(void);
/* 此函数处理USART0中断 */
void USART0_IRQHandler(void);
/* 此函数处理DMA0_Channel0中断 */
void DMA0_Channel0_IRQHandler(void);
/* 此函数处理USART1中断 */
void USART1_IRQHandler(void);
/* 此函数处理DMA0_Channel3中断 */
void DMA0_Channel3_IRQHandler(void);
/* I²C DMA中断声明 */
void DMA0_Channel4_IRQHandler(void);
void DMA0_Channel5_IRQHandler(void);

#endif /* GD32G5X3_IT_H */
