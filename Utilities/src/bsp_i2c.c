/*!
    \file    bsp_i2c.c
    \brief   firmware functions to manage I2C

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp_i2c.h"

#include <stdio.h>
#include "gd32g5x3.h"

void bsp_i2c_init(void)
{
    /* 1. 时钟使能 */
    rcu_periph_clock_enable(RCU_DMA0);     // DMA时钟
    rcu_periph_clock_enable(RCU_DMAMUX);   // DMAMUX时钟
    rcu_periph_clock_enable(RCU_GPIO_I2C); // GPIO时钟
    rcu_periph_clock_enable(RCU_I2C);      // I2C外设时钟

    /* 2. GPIO配置 */
    // SCL引脚配置
    gpio_af_set(I2C_SCL_PORT, I2C_GPIO_AF, I2C_SCL_PIN);
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SCL_PIN);

    // SDA引脚配置
    gpio_af_set(I2C_SDA_PORT, I2C_GPIO_AF, I2C_SDA_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SDA_PIN);

    /* 3. I2C外设配置 */
    // 时序配置 (标准模式400kHz)
    i2c_timing_config(I2CX, 0x1, 0xA, 0);      // 设置SCL高低电平时间
    i2c_master_clock_config(I2CX, 0x73, 0x6E); // 设置主时钟

    // 使能I2C外设
    i2c_enable(I2CX);

    // 初始化eeprom
    i2c_eeprom_init();
}

void i2c_gpio_reset(void)
{
    /* 复位I2C_SCL_PIN和I2C_SDA_PIN */
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, I2C_SCL_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, I2C_SDA_PIN);
}

void i2c_bus_reset()
{
    /* 将SDA/SCL配置为GPIO模式 */
    GPIO_BC(I2C_SCL_PORT) |= I2C_SCL_PIN;
    GPIO_BC(I2C_SDA_PORT) |= I2C_SDA_PIN;
    /* 复位I2C_SCL_PIN和I2C_SDA_PIN */
    i2c_gpio_reset();
    // 插入空操作确保时序
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    /* 产生停止信号 */
    GPIO_BOP(I2C_SCL_PORT) |= I2C_SCL_PIN;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIO_BOP(I2C_SDA_PORT) |= I2C_SDA_PIN;
    /* 重新连接I2C_SCL_PIN到I2C_SCL */
    /* 重新连接I2C_SDA_PIN到I2C_SDA */
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SCL_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SDA_PIN);
}

void i2c_dma_tx_config(uint32_t mem_addr, uint32_t size)
{
    dma_parameter_struct dma_init_struct; // DMA配置结构体

    /* 初始化DMA通道 */
    dma_deinit(I2C_DMAx, I2C_DMA_TX_CH);    // 复位DMA通道寄存器
    dma_struct_para_init(&dma_init_struct); // 初始化结构体为默认值

    /* 配置DMA传输参数 */
    dma_init_struct.request = I2C_DMA_TX_REQUEST;             // DMA请求源选择(I2C发送)
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;     // 传输方向：内存→外设
    dma_init_struct.memory_addr = mem_addr;                   // 内存源地址
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;     // 内存数据宽度8位(匹配I2C)
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;  // 内存地址自增(连续数据块)
    dma_init_struct.number = size;                            // 传输数据量
    dma_init_struct.periph_addr = (uint32_t)&I2C_TDATA(I2CX); // 外设目标地址(I2C数据寄存器)
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT; // 外设数据宽度8位
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE; // 外设地址固定(始终写入同一寄存器)
    dma_init_struct.priority = DMA_PRIORITY_HIGH;             // 高传输优先级
    dma_init(I2C_DMAx, I2C_DMA_TX_CH, &dma_init_struct);      // 应用配置

    /* 高级模式设置 */
    dma_circulation_disable(I2C_DMAx, I2C_DMA_TX_CH);  // 禁用循环模式(单次传输)
    dmamux_synchronization_disable(I2C_DMA_MUX_TX_CH); // 禁用DMAMUX同步触发

    /* 使能DMA通道 */
    dma_channel_enable(I2C_DMAx, I2C_DMA_TX_CH); //

    /* 使能DMA传输完成中断 */
    dma_interrupt_enable(I2C_DMAx, I2C_DMA_TX_CH, DMA_INT_FTF);

    /* 启动DMA传输 */
    i2c_dma_enable(I2CX, I2C_DMA_TRANSMIT);
}

void i2c_dma_rx_config(uint32_t mem_addr, uint32_t size)
{
    dma_parameter_struct dma_init_struct;

    /* 复位DMA通道 */
    dma_deinit(I2C_DMAx, I2C_DMA_RX_CH);    // 复位DMA通道寄存器
    dma_struct_para_init(&dma_init_struct); // 初始化结构体

    /* 配置DMA传输参数 */
    dma_init_struct.request = I2C_DMA_RX_REQUEST;             // DMA请求源选择(I2C接收)
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;     // 传输方向：外设→内存
    dma_init_struct.memory_addr = mem_addr;                   // 内存目标地址
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;     // 内存数据宽度8位
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;  // 内存地址自增
    dma_init_struct.number = size;                            // 传输数据量
    dma_init_struct.periph_addr = (uint32_t)&I2C_RDATA(I2CX); // 外设源地址(I2C数据寄存器)
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT; // 外设数据宽度8位
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE; // 外设地址固定
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;       // 超高优先级(确保接收实时性)
    dma_init(I2C_DMAx, I2C_DMA_RX_CH, &dma_init_struct);      // 应用配置

    /* 高级模式设置 */
    dma_circulation_disable(I2C_DMAx, I2C_DMA_RX_CH);  // 禁用循环模式
    dmamux_synchronization_disable(I2C_DMA_MUX_RX_CH); // 禁用DMAMUX同步

    /* 启动DMA通道 */
    dma_channel_enable(I2C_DMAx, I2C_DMA_RX_CH); // 启动DMA通道

    /* 使能DMA传输完成中断 */
    dma_interrupt_enable(I2C_DMAx, I2C_DMA_RX_CH, DMA_INT_FTF);

    /* 启动DMA传输 */
    i2c_dma_enable(I2CX, I2C_DMA_RECEIVE);
}
