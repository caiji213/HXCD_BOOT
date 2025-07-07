/*!
    \file    bsp_rs232.c
    \brief   firmware functions to manage RS232 COM ports

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp_rs232.h"

uint8_t rx_count = 0;
uint8_t tx_count = 0;
uint8_t rs232_rxbuffer[RS232_BUFFER_SIZE];

__IO FlagStatus rs232_idle_flag = RESET; // 初始：无空闲中断
__IO FlagStatus rs232_tx_flag = SET;     // 初始：USART可发送

void bsp_rs232_init(uint32_t Baudrate, uint16_t parity_bit, uint8_t DMA_enable)
{
    /* 启用GPIO时钟 */
    rcu_periph_clock_enable(RS232_GPIO_CLK);

    /* 启用USART时钟 */
    rcu_periph_clock_enable(RS232_COM_CLK);

    /* 配置端口复用为USART发送引脚 */
    gpio_af_set(RS232_GPIO_PORT, RS232_AF, RS232_TX_PIN);
    /* 配置端口复用为USART接收引脚 */
    gpio_af_set(RS232_GPIO_PORT, RS232_AF, RS232_RX_PIN);

    /* 配置USART_TX为复用推挽模式（上拉） */
    gpio_mode_set(RS232_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS232_TX_PIN);
    gpio_output_options_set(RS232_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, RS232_TX_PIN);

    /* 配置USART_RX为复用推挽模式（上拉） */
    gpio_mode_set(RS232_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS232_RX_PIN);
    gpio_output_options_set(RS232_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, RS232_RX_PIN);

    /* USART串口参数配置 */
    usart_deinit(RS232_COM);                 // 复位串口外设
    usart_baudrate_set(RS232_COM, Baudrate); // 设置波特率

    // 校验位配置
    if (parity_bit == PTY_NONE)
        usart_parity_config(RS232_COM, USART_PM_NONE);
    else if (parity_bit == PTY_ODD)
        usart_parity_config(RS232_COM, USART_PM_ODD);
    else if (parity_bit == PTY_EVEN)
        usart_parity_config(RS232_COM, USART_PM_EVEN);

    usart_receive_config(RS232_COM, USART_RECEIVE_ENABLE);   // 使能接收器
    usart_transmit_config(RS232_COM, USART_TRANSMIT_ENABLE); // 使能发送器

    /* 使能USART空闲中断 */
    usart_interrupt_enable(RS232_COM, USART_INT_IDLE);
    /* 使能USART发送完成中断 */
    usart_interrupt_enable(RS232_COM, USART_INT_TC);
    if (DMA_enable)
    {
        /* 初始化DMA */
        bsp_rs232_dma_init();
    }
    /* 使能USART外设 */
    usart_enable(RS232_COM);
}

void bsp_rs232_dma_init(void)
{
    /* 使能DMA时钟 */
    rcu_periph_clock_enable(RS232_TX_DMA_CLK);
    /* 使能DMAMUX时钟（DMA多路复用器时钟） */
    rcu_periph_clock_enable(RCU_DMAMUX);

    /* 定义DMA初始化参数结构体 */
    dma_parameter_struct dma_init_struct;

    /********************* 初始化发送通道（USART0 TX）*********************/
    dma_deinit(RS232_TX_DMA, RS232_TX_DMA_CH); // 复位DMA通道配置
    dma_struct_para_init(&dma_init_struct);    // 初始化DMA配置结构体为默认值

    /* 配置发送通道参数 */
    dma_init_struct.request = DMA_REQUEST_USART0_TX;             // DMA请求源为USART0发送
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;        // 传输方向：内存到外设
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;     // 内存地址自增
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;        // 内存数据宽度8位
    dma_init_struct.periph_addr = (uint32_t)RS232_TDATA_ADDRESS; // 外设地址为USART0数据寄存器
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;    // 外设地址不自增
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;    // 外设数据宽度8位
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;          // 最高优先级

    /* 应用配置初始化DMA通道 */
    dma_init(RS232_TX_DMA, RS232_TX_DMA_CH, &dma_init_struct);

    /* 禁用循环模式（单次传输） */
    dma_circulation_disable(RS232_TX_DMA, RS232_TX_DMA_CH);

    /* 配置DMA传输完成中断 */
    dma_interrupt_disable(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FTF); // 初始禁用
    dma_interrupt_enable(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FTF);  // 使能中断类型

    /* 清除DMA中断标志 */
    dma_interrupt_flag_clear(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FLAG_G);

    /* 使能USART0的DMA发送功能 */
    usart_dma_transmit_config(RS232_COM, USART_TRANSMIT_DMA_ENABLE);

    /********************* 初始化接收通道（USART0 RX）*********************/
    dma_deinit(RS232_RX_DMA, RS232_RX_DMA_CH); // 复位DMA通道配置
    dma_struct_para_init(&dma_init_struct);    // 初始化DMA配置结构体为默认值

    /* 配置DMA传输参数 */
    dma_init_struct.request = DMA_REQUEST_USART0_RX;             // DMA请求源设为USART0接收
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;        // 传输方向：外设到内存
    dma_init_struct.memory_addr = (uint32_t)rs232_rxbuffer;      // 内存接收缓冲区地址
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;     // 内存地址自增模式
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;        // 内存数据宽度8位
    dma_init_struct.number = RS232_DMA_SIZE;                     // 传输数据量=RS232_BUFFER_SIZE+1，防止产生DMA接收完成中断
    dma_init_struct.periph_addr = (uint32_t)RS232_RDATA_ADDRESS; // 外设数据寄存器地址
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;    // 外设地址固定
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;    // 外设数据宽度8位
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;          // 最高优先级

    /* 应用配置初始化DMA通道 */
    dma_init(RS232_RX_DMA, RS232_RX_DMA_CH, &dma_init_struct);

    /* 禁用循环模式（单次传输） */
    dma_circulation_disable(RS232_RX_DMA, RS232_RX_DMA_CH);

    /* 配置DMAMUX多路复用器，禁用通道1的同步触发模式 */
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH1);

    /* 使能USART0的DMA接收功能 */
    usart_dma_receive_config(RS232_COM, USART_RECEIVE_DMA_ENABLE);

    /* 启动DMA接收通道 */
    dma_channel_enable(RS232_RX_DMA, RS232_RX_DMA_CH);
}

void bsp_rs232_dma_send(uint8_t *buffer, uint32_t length)
{
    // 确保长度不超过缓冲区
    if (length > RS232_BUFFER_SIZE)
        length = RS232_BUFFER_SIZE;

    /* 等待上一次发送完成 */
    while (!rs232_tx_flag)
    {
        __NOP();
    }

    /* 重置标志 */
    rs232_tx_flag = RESET;

    /* 配置DMA传输参数 */
    dma_channel_disable(RS232_TX_DMA, RS232_TX_DMA_CH); // 先禁用通道
    dma_transfer_number_config(RS232_TX_DMA, RS232_TX_DMA_CH, length);
    dma_memory_address_config(RS232_TX_DMA, RS232_TX_DMA_CH, (uint32_t)buffer);

    /* 清除中断标志 */
    dma_interrupt_flag_clear(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FLAG_G);

    /* 使能DMA传输完成中断 */
    dma_interrupt_enable(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FTF);

    /* 启动DMA发送 */
    dma_channel_enable(RS232_TX_DMA, RS232_TX_DMA_CH);
}
