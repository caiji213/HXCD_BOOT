/*!
    \file    systick.h
    \brief   the header file of systick

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* 配置系统滴答定时器 */
void systick_config(void);
/* 延迟指定毫秒时间 */
void delay_1ms(uint32_t count);
/* 延迟递减计数 */
void delay_decrement(void);

#endif /* SYSTICK_H */
