/*!
    \file    bsp_adc.h
    \brief   definitions ADC hardware resources

    \version 2025-06-05 V1.0.0, firmware for GD32G5x3
*/
#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "gd32g5x3.h"

/* 注入通道 ADC（用于三相电流 + 母线电压） */
/* 通道分配：IU -> ADC1_IN11 (PB2), IV -> ADC2_IN4 (PB13), IW -> ADC3_IN3 (PB14) */
#define IU_ADCx                    ADC1
#define IV_ADCx                    ADC2
#define IW_ADCx                    ADC3

#define IU_ADC_CHANNEL             ADC_CHANNEL_11   // PB2
#define IV_ADC_CHANNEL             ADC_CHANNEL_4    // PB13
#define IW_ADC_CHANNEL             ADC_CHANNEL_3    // PB14
#define UDC_AD_ADC_CH              ADC_CHANNEL_3    // PE7（UDC 注入采样）

#define IU_ADC_CLK                 RCU_ADC1
#define IV_ADC_CLK                 RCU_ADC2
#define IW_ADC_CLK                 RCU_ADC3

#define IU_ADC_GPIO_PORT           GPIOB
#define IU_ADC_GPIO_PIN            GPIO_PIN_2
#define IU_ADC_GPIO_CLK            RCU_GPIOB

#define IV_ADC_GPIO_PORT           GPIOB
#define IV_ADC_GPIO_PIN            GPIO_PIN_13
#define IV_ADC_GPIO_CLK            RCU_GPIOB

#define IW_ADC_GPIO_PORT           GPIOB
#define IW_ADC_GPIO_PIN            GPIO_PIN_14
#define IW_ADC_GPIO_CLK            RCU_GPIOB

#define UDC_AD_GPIO_PORT           GPIOE
#define UDC_AD_GPIO_PIN            GPIO_PIN_7
#define UDC_AD_GPIO_CLK            RCU_GPIOE

/* 板卡电压信息（规则通道 + DMA）：REF、BoardType、BoardTemp，使用 ADC0 */
#define VOLTAGE_ADCx               ADC0
#define VOLTAGE_CHANNEL_NUM        3

#define VOLTAGE_ADC_DMA            DMA0
#define VOLTAGE_ADC_DMA_CLK        RCU_DMA0
#define VOLTAGE_ADC_DMA_CH         DMA_CH6
#define VOLTAGE_ADC_DMA_REQUEST    DMA_REQUEST_ADC0
#define VOLTAGE_ADC_DMA_IRQn       DMA0_Channel6_IRQn

#define REF1V65_ADC_CH             ADC_CHANNEL_10   // PB12
#define BOARDTYPE_ADC_CH           ADC_CHANNEL_11   // PB1
#define BOARDTEMP_ADC_CH           ADC_CHANNEL_12   // PB0

#define REF1V65_GPIO_PORT          GPIOB
#define REF1V65_GPIO_PIN           GPIO_PIN_12
#define REF1V65_GPIO_CLK           RCU_GPIOB

#define BOARDTYPE_GPIO_PORT        GPIOB
#define BOARDTYPE_GPIO_PIN         GPIO_PIN_1
#define BOARDTYPE_GPIO_CLK         RCU_GPIOB

#define BOARDTEMP_GPIO_PORT        GPIOB
#define BOARDTEMP_GPIO_PIN         GPIO_PIN_0
#define BOARDTEMP_GPIO_CLK         RCU_GPIOB

/* 注入采样触发配置（TIMER0_CH3 -> TRIGSEL -> ADCx） */
#define ADC_TRIGGER_TIMER          TIMER0
#define ADC_TRIGGER_CHANNEL        TIMER_CH_3
#define ADC_TRIGGER_SOURCE         TRIGSEL_INPUT_TIMER0_CH3

#define IU_TRIGSEL_OUTPUT_INSTRG   TRIGSEL_OUTPUT_ADC1_INSTRG
#define IV_TRIGSEL_OUTPUT_INSTRG   TRIGSEL_OUTPUT_ADC2_INSTRG
#define IW_TRIGSEL_OUTPUT_INSTRG   TRIGSEL_OUTPUT_ADC3_INSTRG

/* ADC 中断向量 */
#define IU_ADC_IRQn                ADC0_1_IRQn
#define IV_ADC_IRQn                ADC2_IRQn
#define IW_ADC_IRQn                ADC3_IRQn

/*  全局变量声明  */
extern uint16_t iu_adc_value;
extern uint16_t iv_adc_value;
extern uint16_t iw_adc_value;
extern uint16_t udc_adc_value;
extern uint16_t voltage_adc_values[VOLTAGE_CHANNEL_NUM];

/* 函数声明  */
void bsp_adc_init(void);
void adc_gpio_init(void);
void adc_trigger_config(void);
void bsp_adc_voltage_dma_config(void);
void bsp_adc_injected_config(void);

#endif // __BSP_ADC_H__
