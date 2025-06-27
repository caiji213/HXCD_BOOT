/*!
    \file    bsp_tim_pwm.c
    \brief   firmware functions to manage TIMER0 PWM

    \version 2025-06-05 V1.0.0, firmware for GD32G5x3
*/

#include <stdio.h>
#include "bsp_tim_pwm.h"

void bsp_pwm_test_once(void)
{

    static int16_t duty = 0; // 当前占空比
    static int8_t step = 10; // 每次增加/减少的步长

    // 更新占空比值
    duty += step;
    if (duty >= PWM_TIMER_PERIOD)
    {
        duty = PWM_TIMER_PERIOD;
        step = -step; // 反向下降
    }
    else if (duty <= 0)
    {
        duty = 0;
        step = -step; // 反向上升
    }

    // 设置三个通道一致占空比
    bsp_pwm_set_duty(TIMER_CH_0, duty);
    bsp_pwm_set_duty(TIMER_CH_1, duty);
    bsp_pwm_set_duty(TIMER_CH_2, duty);

    // printf("PWM smooth duty: %.1f%%\r\n", (float)duty * 100 / PWM_TIMER_PERIOD);
}

void bsp_pwm_en_init(void)
{
    rcu_periph_clock_enable(PWM_EN_GPIO_CLK);
    gpio_mode_set(PWM_EN_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PWM_EN_GPIO_PIN);
    gpio_output_options_set(PWM_EN_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_EN_GPIO_PIN);
    // 上电默认禁止输出
    gpio_bit_reset(PWM_EN_GPIO_PORT, PWM_EN_GPIO_PIN);
}

void bsp_pwm_en_set(uint8_t enable)
{
    if (enable)
    {
        gpio_bit_set(PWM_EN_GPIO_PORT, PWM_EN_GPIO_PIN);
    }
    else
    {
        gpio_bit_reset(PWM_EN_GPIO_PORT, PWM_EN_GPIO_PIN);
    }
}

static void pwm_gpio_config(void)
{
    // 启用 PWM GPIO 所在端口时钟
    rcu_periph_clock_enable(PWM_GPIO_CLK);

    // 配置主PWM通道 CH0：PE9
    gpio_mode_set(PWM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, PWM_CH0_PIN);               // 复用功能
    gpio_output_options_set(PWM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_CH0_PIN); // 推挽，60MHz
    gpio_af_set(PWM_GPIO_PORT, PWM_CH0_AF, PWM_CH0_PIN);                                   // 设置为 AF2 → TIMER0_CH0

    // 配置互补PWM通道 MCH0：PE8
    gpio_mode_set(PWM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, PWM_MCH0_PIN);
    gpio_output_options_set(PWM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_MCH0_PIN);
    gpio_af_set(PWM_GPIO_PORT, PWM_MCH0_AF, PWM_MCH0_PIN);

    // 配置主PWM通道 CH1：PE11
    gpio_mode_set(PWM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, PWM_CH1_PIN);
    gpio_output_options_set(PWM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_CH1_PIN);
    gpio_af_set(PWM_GPIO_PORT, PWM_CH1_AF, PWM_CH1_PIN);

    // 配置互补PWM通道 MCH1：PE10
    gpio_mode_set(PWM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, PWM_MCH1_PIN);
    gpio_output_options_set(PWM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_MCH1_PIN);
    gpio_af_set(PWM_GPIO_PORT, PWM_MCH1_AF, PWM_MCH1_PIN);

    // 配置主PWM通道 CH2：PE13
    gpio_mode_set(PWM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, PWM_CH2_PIN);
    gpio_output_options_set(PWM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_CH2_PIN);
    gpio_af_set(PWM_GPIO_PORT, PWM_CH2_AF, PWM_CH2_PIN);

    // 配置互补PWM通道 MCH2：PE12
    gpio_mode_set(PWM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, PWM_MCH2_PIN);
    gpio_output_options_set(PWM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, PWM_MCH2_PIN);
    gpio_af_set(PWM_GPIO_PORT, PWM_MCH2_AF, PWM_MCH2_PIN);
}

void bsp_tim_pwm_init(void)
{
    /* GPIO初始化配置 */
    pwm_gpio_config();

    /* 定时器时钟使能 */
    rcu_periph_clock_enable(PWM_TIMER_CLK); // 使能定时器时钟
    timer_deinit(PWM_TIMER);                // 复位定时器到默认状态

    /* 声明配置结构体 */
    timer_parameter_struct timer_initpara;        // 定时器基础配置结构体
    timer_oc_parameter_struct timer_ocinitpara;   // 输出比较配置结构体
    timer_break_parameter_struct timer_breakpara; // 刹车和死区配置结构体

    /* 定时器基础配置 */
    timer_struct_para_init(&timer_initpara);                // 初始化结构体为默认值
    timer_initpara.prescaler = PWM_TIMER_PRESCALER;         // 预分频器值
    timer_initpara.alignedmode = TIMER_COUNTER_CENTER_BOTH; // 中心对齐模式
    timer_initpara.counterdirection = TIMER_COUNTER_UP;     // 向上计数模式
    timer_initpara.period = PWM_TIMER_PERIOD;               // 自动重装载值
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;        // 时钟分频
    timer_initpara.repetitioncounter = 1;                   // 重复计数器
    timer_init(PWM_TIMER, &timer_initpara);                 // 应用配置到定时器

    /* 输出通道通用配置 */
    timer_channel_output_struct_para_init(&timer_ocinitpara); // 初始化结构体
    timer_ocinitpara.outputstate = TIMER_CCX_ENABLE;          // 使能主通道输出
    timer_ocinitpara.outputnstate = TIMER_CCXN_ENABLE;        // 使能互补通道输出
    timer_ocinitpara.ocpolarity = TIMER_OC_POLARITY_HIGH;     // 主通道有效电平为高
    timer_ocinitpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;   // 互补通道有效电平为高
    timer_ocinitpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;   // 空闲时主通道输出低电平
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW; // 空闲时互补通道输出低电平

    /* 通道0配置 */
    timer_channel_output_config(PWM_TIMER, TIMER_CH_0, &timer_ocinitpara);                     // 应用通道配置
    timer_multi_mode_channel_mode_config(PWM_TIMER, TIMER_CH_0, TIMER_MCH_MODE_COMPLEMENTARY); // 设为互补模式
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_0, PWM_DUTY_CH0);              // 设置比较值(占空比=比较值/周期)
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_0, TIMER_OC_MODE_PWM0);               // PWM模式0（计数<比较值时有效）
    timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);        // 禁止影子寄存器(立即更新)

    /* 通道1配置 */
    timer_channel_output_config(PWM_TIMER, TIMER_CH_1, &timer_ocinitpara);
    timer_multi_mode_channel_mode_config(PWM_TIMER, TIMER_CH_1, TIMER_MCH_MODE_COMPLEMENTARY);
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_1, PWM_DUTY_CH1);
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    /* 通道2配置 */
    timer_channel_output_config(PWM_TIMER, TIMER_CH_2, &timer_ocinitpara);
    timer_multi_mode_channel_mode_config(PWM_TIMER, TIMER_CH_2, TIMER_MCH_MODE_COMPLEMENTARY);
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_2, PWM_DUTY_CH2);
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);

    /* 通道3：用于 ADC 触发 */
    timer_oc_parameter_struct trig_ocinit;
    timer_channel_output_struct_para_init(&trig_ocinit);

    trig_ocinit.outputstate = TIMER_CCX_ENABLE;    // 启用主通道输出
    trig_ocinit.outputnstate = TIMER_CCXN_DISABLE; // 禁用互补通道
    trig_ocinit.ocpolarity = TIMER_OC_POLARITY_HIGH;
    trig_ocinit.ocidlestate = TIMER_OC_IDLE_STATE_LOW;

    timer_channel_output_config(PWM_TIMER, TIMER_CH_3, &trig_ocinit);
    timer_channel_output_pulse_value_config(PWM_TIMER, TIMER_CH_3, PWM_ADC_TRIG_PULSE);
    timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_3, TIMER_OC_MODE_PWM0);
    timer_interrupt_enable(TIMER0, TIMER_INT_CH3);

    /* 刹车和死区配置（安全特性） */
    timer_break_struct_para_init(&timer_breakpara);         // 初始化结构体
    timer_breakpara.runoffstate = TIMER_ROS_STATE_ENABLE;   // 运行模式下关闭输出有效
    timer_breakpara.ideloffstate = TIMER_IOS_STATE_ENABLE;  // 空闲模式下关闭输出有效
    timer_breakpara.deadtime = PWM_DEADTIME;                // 死区时间值（防止上下管直通）
    timer_breakpara.outputautostate = TIMER_OUTAUTO_ENABLE; // 自动输出使能
    timer_breakpara.protectmode = TIMER_CCHP0_PROT_OFF;     // 保护模式关闭
    timer_breakpara.break0state = TIMER_BREAK0_DISABLE;     // 禁用刹车输入0
    timer_breakpara.break1state = TIMER_BREAK1_DISABLE;     // 禁用刹车输入1
    timer_break_config(PWM_TIMER, &timer_breakpara);        // 应用刹车配置

    /* 高级控制配置 */
    timer_auto_reload_shadow_enable(PWM_TIMER);     // 使能自动重装载影子寄存器
    timer_primary_output_config(PWM_TIMER, ENABLE); // 主输出使能

    /* 启动定时器 */
    timer_enable(PWM_TIMER);
}

void bsp_pwm_init(void)
{
    bsp_pwm_en_init();  // 初始化输出控制
    bsp_tim_pwm_init(); // 初始化PWM
}

void bsp_pwm_set_duty(uint8_t channel, uint16_t pulse)
{
    if (channel > TIMER_CH_2 || pulse > PWM_TIMER_PERIOD)
    {
        return;
    }
    timer_channel_output_pulse_value_config(PWM_TIMER, channel, pulse);
}

void bsp_pwm_enable(void)
{
    timer_primary_output_config(PWM_TIMER, ENABLE);
    timer_enable(PWM_TIMER);
}

void bsp_pwm_disable(void)
{
    timer_primary_output_config(PWM_TIMER, DISABLE);
    timer_disable(PWM_TIMER);
}
