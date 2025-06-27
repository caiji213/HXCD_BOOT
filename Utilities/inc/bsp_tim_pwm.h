/*!
    \file    bsp_tim_pwm.h
    \brief   definitions PWM hardware resources

    \version 2025-06-05 V1.3.0, with dynamic control functions
*/

#ifndef BSP_TIM_PWM_H
#define BSP_TIM_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32g5x3.h"
#include "gd32g5x3_libopt.h"

/* TIMER 资源  */
#define PWM_TIMER                        TIMER0
#define PWM_TIMER_CLK                    RCU_TIMER0

/* GPIO 时钟  */
#define PWM_GPIO_PORT                    GPIOE
#define PWM_GPIO_CLK                     RCU_GPIOE

/*  PWM 通道定义  */
#define PWM_CH0_PIN                      GPIO_PIN_9
#define PWM_CH0_AF                       GPIO_AF_2
#define PWM_MCH0_PIN                     GPIO_PIN_8
#define PWM_MCH0_AF                      GPIO_AF_2

#define PWM_CH1_PIN                      GPIO_PIN_11
#define PWM_CH1_AF                       GPIO_AF_2
#define PWM_MCH1_PIN                     GPIO_PIN_10
#define PWM_MCH1_AF                      GPIO_AF_2

#define PWM_CH2_PIN                      GPIO_PIN_13
#define PWM_CH2_AF                       GPIO_AF_2
#define PWM_MCH2_PIN                     GPIO_PIN_12
#define PWM_MCH2_AF                      GPIO_AF_2

#define PWM_ADC_TRIG_CH                  TIMER_CH_3
#define PWM_ADC_TRIG_PULSE              (PWM_TIMER_PERIOD / 2 - 1) //(PWM_TIMER_PERIOD - 1) 比较值

/*  PWM_EN 控制引脚  */
#define PWM_EN_GPIO_PORT                 GPIOA
#define PWM_EN_GPIO_PIN                  GPIO_PIN_12
#define PWM_EN_GPIO_CLK                  RCU_GPIOA

/*  PWM 默认参数 */
#define PWM_TIMER_PRESCALER              (216 - 1)     // 216MHz/216 = 1MHz
#define PWM_TIMER_PERIOD                 999           // 
#define PWM_DUTY_CH0                     500           // 占空比50%
#define PWM_DUTY_CH1                     500           // 占空比50%
#define PWM_DUTY_CH2                     500           // 占空比50%
#define PWM_DEADTIME                     100U          // 死区时间

/* 函数声明  */
void bsp_tim_pwm_init(void);                      // 初始化PWM定时器和GPIO
void bsp_pwm_set_duty(uint8_t channel, uint16_t pulse); // 设置占空比
void bsp_pwm_enable(void);                        // 启用PWM输出（MOE）
void bsp_pwm_disable(void);                       // 禁用PWM输出（MOE）
void bsp_pwm_init(void);                          // PWM初始化
void bsp_pwm_en_init(void);                       // 初始化PWM_EN控制引脚
void bsp_pwm_en_set(uint8_t enable);              // 控制PWM_EN使能状态
void bsp_pwm_test_once(void);                     // PWM测试

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIM_PWM_H */
