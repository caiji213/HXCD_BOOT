/*!
    \file    bsp_spi.c
    \brief   firmware functions to manage SPI

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp_spi.h"

uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];

__IO FlagStatus spi_rx_complete_flag = RESET;

void bsp_spi_rcu_config(void)
{
    rcu_periph_clock_enable(SPI2_GPIO_CLK); // 使能GPIOC时钟
    rcu_periph_clock_enable(SPI2_DMA_CLK);  // 使能DMA1时钟
    rcu_periph_clock_enable(RCU_DMAMUX);    // 使能DMAMUX时钟
    rcu_periph_clock_enable(SPI2_CLK);      // 使能SPI2时钟
}

void bsp_spi_gpio_config(void)
{
    /* SPI2引脚复用配置 */
    gpio_af_set(SPI2_SCK_PORT, GPIO_AF_6, SPI2_SCK_PIN);
    gpio_af_set(SPI2_MISO_PORT, GPIO_AF_6, SPI2_MISO_PIN);

    /* SCK配置 */
    gpio_mode_set(SPI2_SCK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_SCK_PIN);
    gpio_output_options_set(SPI2_SCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, SPI2_SCK_PIN);

    /* MISO配置 */
    gpio_mode_set(SPI2_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_MISO_PIN);
    gpio_output_options_set(SPI2_MISO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, SPI2_MISO_PIN);
}

void bsp_spi_periph_config(void)
{
    spi_parameter_struct spi_init_struct;
    spi_struct_para_init(&spi_init_struct);

    /* 配置SPI参数 */
    spi_init_struct.device_mode = SPI_MASTER;                       // 主机模式
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;          // 全双工模式
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;                // 8位数据帧
    spi_init_struct.nss = SPI_NSS_SOFT;                             // 软件控制NSS
    spi_init_struct.endian = SPI_ENDIAN_MSB;                        // MSB先行
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE; // 极性
    spi_init_struct.prescale = SPI_PSC_128;                         // 分频

    spi_init(SPI2_PERIPH, &spi_init_struct);

    // 注意：必须使用 SPI_BYTE_ACCESS，防止SPI在8bit帧宽下启用数据合并模式造成DMA错位
    spi_fifo_access_size_config(SPI2_PERIPH, SPI_BYTE_ACCESS);

    spi_enable(SPI2_PERIPH);
}

void bsp_spi_dma_init(void)
{
    dma_parameter_struct dma_init_struct;

    /********** 配置 SPI2 TX（发送）DMA：DMA1 通道1 **********/
    // 复位 DMA 发送通道
    dma_deinit(SPI2_DMA, SPI2_TX_DMA_CH);

    // 初始化结构体为默认值
    dma_struct_para_init(&dma_init_struct);

    // 设置 DMA 发送参数
    dma_init_struct.request = SPI2_TX_DMA_REQUEST;                  // DMA 请求源
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;           // 从内存到外设
    dma_init_struct.memory_addr = (uint32_t)spi_tx_buffer;          // 源地址：发送缓冲区
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;        // 启用内存地址自增
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;           // 内存宽度：8位
    dma_init_struct.number = SPI_BUFFER_SIZE;                       // 传输数据数量
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2_PERIPH); // 目标地址：SPI2 数据寄存器
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;       // 禁止外设地址自增
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;       // 外设宽度：8位
    dma_init_struct.priority = DMA_PRIORITY_HIGH;                   // 优先级：高

    // 初始化 DMA 通道
    dma_init(SPI2_DMA, SPI2_TX_DMA_CH, &dma_init_struct);

    // 禁用内存到内存模式
    dma_memory_to_memory_disable(SPI2_DMA, SPI2_TX_DMA_CH);

    // 禁用循环模式
    dma_circulation_disable(SPI2_DMA, SPI2_TX_DMA_CH);

    // 使能传输完成中断
    dma_interrupt_enable(SPI2_DMA, SPI2_TX_DMA_CH, DMA_INT_FTF);

    // 禁用 DMAMUX 同步模式（通道1）
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH1);

    /********** 配置 SPI2 RX（接收）DMA：DMA1 通道0 **********/
    // 复位 DMA 接收通道
    dma_deinit(SPI2_DMA, SPI2_RX_DMA_CH);

    // 初始化结构体为默认值
    dma_struct_para_init(&dma_init_struct);

    // 设置 DMA 接收参数
    dma_init_struct.request = SPI2_RX_DMA_REQUEST;                  // DMA 请求源
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;           // 从外设到内存
    dma_init_struct.memory_addr = (uint32_t)spi_rx_buffer;          // 目标地址：接收缓冲区
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;        // 启用内存地址自增
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;           // 内存宽度：8位
    dma_init_struct.number = SPI_BUFFER_SIZE;                       // 传输数据数量
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2_PERIPH); // 源地址：SPI2 数据寄存器
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;       // 禁止外设地址自增
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;       // 外设宽度：8位
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;             // 优先级：超高

    // 初始化 DMA 通道
    dma_init(SPI2_DMA, SPI2_RX_DMA_CH, &dma_init_struct);

    // 禁用内存到内存模式
    dma_memory_to_memory_disable(SPI2_DMA, SPI2_RX_DMA_CH);

    // 禁用循环模式
    dma_circulation_disable(SPI2_DMA, SPI2_RX_DMA_CH);

    // 使能传输完成中断
    dma_interrupt_enable(SPI2_DMA, SPI2_RX_DMA_CH, DMA_INT_FTF);

    // 禁用 DMAMUX 同步模式（通道0）
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH0);

    /********** 启用 SPI2 的 DMA 功能 **********/
    spi_dma_enable(SPI2_PERIPH, SPI_DMA_TRANSMIT); // 启用 SPI2 TX DMA
    spi_dma_enable(SPI2_PERIPH, SPI_DMA_RECEIVE);  // 启用 SPI2 RX DMA
}

void bsp_spi_init(void)
{
    bsp_spi_rcu_config();    // 配置时钟
    bsp_spi_gpio_config();   // 配置GPIO
    bsp_spi_periph_config(); // 配置SPI外设
    bsp_spi_dma_init();      // 配置DMA
}

void bsp_spi_transfer(uint32_t size)
{
    /* 确保传输长度不超过缓冲区大小 */
    if (size > SPI_BUFFER_SIZE)
    {
        size = SPI_BUFFER_SIZE;
    }

    /* 初始化缓冲区为0x00 */
    for (uint32_t i = 0; i < size; i++)
    {
        spi_tx_buffer[i] = 0x00;
        spi_rx_buffer[i] = 0x00;
    }

    /* 配置DMA传输长度 */
    dma_transfer_number_config(SPI2_DMA, SPI2_TX_DMA_CH, size);
    dma_transfer_number_config(SPI2_DMA, SPI2_RX_DMA_CH, size);

    /* 启动DMA传输 - 先启动接收再启动发送 */
    dma_channel_enable(SPI2_DMA, SPI2_RX_DMA_CH);
    dma_channel_enable(SPI2_DMA, SPI2_TX_DMA_CH);

    // 等待传输完成
    while (spi_rx_complete_flag == RESET)
        ;

    /* 清除接收完成标志 */
    spi_rx_complete_flag = RESET;
}

// 数据处理函数
void process_spi_data(uint8_t *data, uint32_t size)
{
    //    // 打印数据头
    //    printf("Received %u bytes of data:\n", size);
    //
    //    // 按行打印数据（每行16字节）
    //    for (uint32_t i = 0; i < size; i++) {
    //        // 每16字节打印一行
    //        if (i % 16 == 0) {
    //            // 打印当前行号
    //            if (i > 0) printf("\n");
    //            printf("%04X: ", i);
    //        }
    //
    //        // 打印当前字节的十六进制值
    //        printf("%02X ", data[i]);
    //    }
    //
    //    // 打印结束符
    //    printf("\n\n");
}