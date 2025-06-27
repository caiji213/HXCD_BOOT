/*!
    \file    bsp_adc.c
    \brief   firmware functions to manage ADC

    \version 2025-06-25 V1.0.0, firmware for GD32G5x3
*/      
#include "bsp_adc.h"
#include "systick.h"

// 注入采样：三相电流 + 母线电压
uint16_t iu_adc_value = 0;
uint16_t iv_adc_value = 0;
uint16_t iw_adc_value = 0;
uint16_t udc_adc_value = 0;

// 规则采样：Vref + 板卡类型 + 温度
uint16_t voltage_adc_values[VOLTAGE_CHANNEL_NUM] = {0};

void bsp_adc_init(void)
{
    adc_gpio_init();              // 所有 ADC 引脚
    adc_trigger_config();         // TIMER0_CH3 -> TRIGSEL -> ADCx 插入触发
    bsp_adc_injected_config();    // 三相电流 + Udc 注入采样配置
    bsp_adc_voltage_dma_config(); // 板卡信息规则采样配置（带 DMA）
}

void adc_gpio_init(void)
{
    // 打开 GPIO 时钟
    rcu_periph_clock_enable(IU_ADC_GPIO_CLK);
    rcu_periph_clock_enable(IV_ADC_GPIO_CLK);
    rcu_periph_clock_enable(IW_ADC_GPIO_CLK);
    rcu_periph_clock_enable(UDC_AD_GPIO_CLK);
    rcu_periph_clock_enable(REF1V65_GPIO_CLK);
    rcu_periph_clock_enable(BOARDTYPE_GPIO_CLK);
    rcu_periph_clock_enable(BOARDTEMP_GPIO_CLK);

    // 配置为模拟输入
    gpio_mode_set(IU_ADC_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, IU_ADC_GPIO_PIN);
    gpio_mode_set(IV_ADC_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, IV_ADC_GPIO_PIN);
    gpio_mode_set(IW_ADC_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, IW_ADC_GPIO_PIN);
    gpio_mode_set(UDC_AD_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, UDC_AD_GPIO_PIN);
    gpio_mode_set(REF1V65_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, REF1V65_GPIO_PIN);
    gpio_mode_set(BOARDTYPE_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BOARDTYPE_GPIO_PIN);
    gpio_mode_set(BOARDTEMP_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BOARDTEMP_GPIO_PIN);
}

void adc_trigger_config(void)
{
    rcu_periph_clock_enable(RCU_TRIGSEL);
    trigsel_init(IU_TRIGSEL_OUTPUT_INSTRG, ADC_TRIGGER_SOURCE);
    trigsel_init(IV_TRIGSEL_OUTPUT_INSTRG, ADC_TRIGGER_SOURCE);
    trigsel_init(IW_TRIGSEL_OUTPUT_INSTRG, ADC_TRIGGER_SOURCE);
}

void bsp_adc_injected_config(void)
{
    // 打开 ADC 时钟
    rcu_periph_clock_enable(IU_ADC_CLK);
    rcu_periph_clock_enable(IV_ADC_CLK);
    rcu_periph_clock_enable(IW_ADC_CLK);

    // 复位 ADC
    adc_deinit(IU_ADCx);
    adc_deinit(IV_ADCx);
    adc_deinit(IW_ADCx);

    // 配置 ADC 时钟分频
    adc_clock_config(IU_ADCx, ADC_CLK_SYNC_HCLK_DIV6);
    adc_clock_config(IV_ADCx, ADC_CLK_SYNC_HCLK_DIV6);
    adc_clock_config(IW_ADCx, ADC_CLK_SYNC_HCLK_DIV6);

    // 配置工作模式
    adc_special_function_config(IU_ADCx, ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(IV_ADCx, ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(IW_ADCx, ADC_CONTINUOUS_MODE, DISABLE);

    adc_special_function_config(IU_ADCx, ADC_SCAN_MODE, ENABLE);
    adc_special_function_config(IV_ADCx, ADC_SCAN_MODE, ENABLE);
    adc_special_function_config(IW_ADCx, ADC_SCAN_MODE, ENABLE);

    adc_resolution_config(IU_ADCx, ADC_RESOLUTION_12B);
    adc_resolution_config(IV_ADCx, ADC_RESOLUTION_12B);
    adc_resolution_config(IW_ADCx, ADC_RESOLUTION_12B);

    adc_data_alignment_config(IU_ADCx, ADC_DATAALIGN_RIGHT);
    adc_data_alignment_config(IV_ADCx, ADC_DATAALIGN_RIGHT);
    adc_data_alignment_config(IW_ADCx, ADC_DATAALIGN_RIGHT);

    // 配置注入通道数量
    adc_channel_length_config(IU_ADCx, ADC_INSERTED_CHANNEL, 1);
    adc_channel_length_config(IV_ADCx, ADC_INSERTED_CHANNEL, 2); // IV + UDC
    adc_channel_length_config(IW_ADCx, ADC_INSERTED_CHANNEL, 1);

    // 通道配置（通道编号、采样时间）
    adc_inserted_channel_config(IU_ADCx, 0, IU_ADC_CHANNEL, 24);
    adc_inserted_channel_config(IV_ADCx, 0, IV_ADC_CHANNEL, 24);
    adc_inserted_channel_config(IV_ADCx, 1, UDC_AD_ADC_CH, 24);
    adc_inserted_channel_config(IW_ADCx, 0, IW_ADC_CHANNEL, 24);

    // 配置注入通道外部触发（TIMER0_CH3）
    adc_external_trigger_config(IU_ADCx, ADC_INSERTED_CHANNEL, EXTERNAL_TRIGGER_RISING);
    adc_external_trigger_config(IV_ADCx, ADC_INSERTED_CHANNEL, EXTERNAL_TRIGGER_RISING);
    adc_external_trigger_config(IW_ADCx, ADC_INSERTED_CHANNEL, EXTERNAL_TRIGGER_RISING);

    // 清除并使能中断
    adc_interrupt_flag_clear(IU_ADCx, ADC_INT_FLAG_EOIC);
    adc_interrupt_flag_clear(IV_ADCx, ADC_INT_FLAG_EOIC);
    adc_interrupt_flag_clear(IW_ADCx, ADC_INT_FLAG_EOIC);

    adc_interrupt_enable(IU_ADCx, ADC_INT_EOIC);
    adc_interrupt_enable(IV_ADCx, ADC_INT_EOIC);
    adc_interrupt_enable(IW_ADCx, ADC_INT_EOIC);

    // 使能 ADC 并校准
    adc_enable(IU_ADCx);
    adc_enable(IV_ADCx);
    adc_enable(IW_ADCx);
    delay_1ms(1);

    adc_calibration_mode_config(IU_ADCx, ADC_CALIBRATION_OFFSET);
    adc_calibration_number(IU_ADCx, ADC_CALIBRATION_NUM1);
    adc_calibration_enable(IU_ADCx);

    adc_calibration_mode_config(IV_ADCx, ADC_CALIBRATION_OFFSET);
    adc_calibration_number(IV_ADCx, ADC_CALIBRATION_NUM1);
    adc_calibration_enable(IV_ADCx);

    adc_calibration_mode_config(IW_ADCx, ADC_CALIBRATION_OFFSET);
    adc_calibration_number(IW_ADCx, ADC_CALIBRATION_NUM1);
    adc_calibration_enable(IW_ADCx);
}

void bsp_adc_voltage_dma_config(void)
{
    // 打开 ADC + DMA 时钟
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(VOLTAGE_ADC_DMA_CLK);
    rcu_periph_clock_enable(RCU_DMAMUX);

    // ADC 配置
    adc_deinit(VOLTAGE_ADCx);
    adc_clock_config(VOLTAGE_ADCx, ADC_CLK_SYNC_HCLK_DIV6);
    adc_special_function_config(VOLTAGE_ADCx, ADC_CONTINUOUS_MODE, ENABLE);
    adc_special_function_config(VOLTAGE_ADCx, ADC_SCAN_MODE, ENABLE);
    adc_resolution_config(VOLTAGE_ADCx, ADC_RESOLUTION_12B);
    adc_data_alignment_config(VOLTAGE_ADCx, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(VOLTAGE_ADCx, ADC_ROUTINE_CHANNEL, VOLTAGE_CHANNEL_NUM);

    // 规则通道配置
    adc_routine_channel_config(VOLTAGE_ADCx, 0, REF1V65_ADC_CH, 24);
    adc_routine_channel_config(VOLTAGE_ADCx, 1, BOARDTYPE_ADC_CH, 24);
    adc_routine_channel_config(VOLTAGE_ADCx, 2, BOARDTEMP_ADC_CH, 24);
    adc_external_trigger_config(VOLTAGE_ADCx, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    // DMA 配置
    dma_parameter_struct dma_init_struct;
    dma_deinit(VOLTAGE_ADC_DMA, VOLTAGE_ADC_DMA_CH);
    dma_init_struct.request = VOLTAGE_ADC_DMA_REQUEST;
    dma_init_struct.periph_addr = (uint32_t)&ADC_RDATA(VOLTAGE_ADCx);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.memory_addr = (uint32_t)voltage_adc_values;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.number = VOLTAGE_CHANNEL_NUM;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;

    dma_init(VOLTAGE_ADC_DMA, VOLTAGE_ADC_DMA_CH, &dma_init_struct);
    dma_circulation_enable(VOLTAGE_ADC_DMA, VOLTAGE_ADC_DMA_CH);

    // 使能DMA中断
    dma_interrupt_enable(VOLTAGE_ADC_DMA, VOLTAGE_ADC_DMA_CH, DMA_CHXCTL_FTFIE);

    dma_channel_enable(VOLTAGE_ADC_DMA, VOLTAGE_ADC_DMA_CH);

    adc_dma_request_after_last_enable(VOLTAGE_ADCx);
    adc_dma_mode_enable(VOLTAGE_ADCx);

    adc_enable(VOLTAGE_ADCx);
    delay_1ms(1);

    adc_calibration_mode_config(VOLTAGE_ADCx, ADC_CALIBRATION_OFFSET);
    adc_calibration_number(VOLTAGE_ADCx, ADC_CALIBRATION_NUM1);
    adc_calibration_enable(VOLTAGE_ADCx);

    adc_software_trigger_enable(VOLTAGE_ADCx, ADC_ROUTINE_CHANNEL); // 启动规则采样
}
