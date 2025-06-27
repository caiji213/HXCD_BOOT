/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "gd32g5x3.h"
#include "systick.h"

volatile static uint32_t delay;

/*!
    \brief      配置系统滴答定时器
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void systick_config(void)
{
    /* 配置系统滴答定时器以产生1000Hz中断 */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        /* 捕获错误 */
        while (1)
        {
        }
    }
    /* 配置系统滴答中断处理函数优先级 */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      延迟指定毫秒时间
    \param[in]  count: 延迟的毫秒数
    \param[out] 无
    \retval     无
*/
void delay_1ms(uint32_t count)
{
    delay = count;

    while (0U != delay)
    {
    }
}

/*!
    \brief      延迟递减计数
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void delay_decrement(void)
{
    if (0U != delay)
    {
        delay--;
    }
}
