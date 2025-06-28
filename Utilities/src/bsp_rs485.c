/*!
    \file    bsp_rs485.c
    \brief   firmware functions to manage RS485 ports
	
    \version 2025-06-05 V1.0.0, firmware for GD32G5x3
*/

#include "bsp_rs485.h"

uint8_t rs485_rx_count = 0;
uint8_t rs485_rxbuffer[RS485_BUFFER_SIZE];
__IO FlagStatus rs485_idle_flag = RESET;
__IO FlagStatus rs485_tx_flag = SET;

void bsp_rs485_init(void)
{
    /* 使能GPIO和USART时钟 */
    rcu_periph_clock_enable(RS485_GPIO_CLK);
    rcu_periph_clock_enable(RS485_COM_CLK);

    /* 配置TX引脚 */
    gpio_af_set(RS485_GPIO_PORT, RS485_AF, RS485_TX_PIN);
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS485_TX_PIN);
    gpio_output_options_set(RS485_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, RS485_TX_PIN);

    /* 配置RX引脚 */
    gpio_af_set(RS485_GPIO_PORT, RS485_AF, RS485_RX_PIN);
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS485_RX_PIN);
    gpio_output_options_set(RS485_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, RS485_RX_PIN);

    /* 配置RS485方向控制引脚 */
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS485_RE_PIN);
    gpio_output_options_set(RS485_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, RS485_RE_PIN);

    /* 默认设置为接收模式 */
    bsp_rs485_receive_mode();

    /* USART配置 */
    usart_deinit(RS485_COM);
    usart_baudrate_set(RS485_COM, 115200U);
    usart_receive_config(RS485_COM, USART_RECEIVE_ENABLE);
    usart_transmit_config(RS485_COM, USART_TRANSMIT_ENABLE);

    /* 使能USART空闲中断 */
    usart_interrupt_enable(RS485_COM, USART_INT_IDLE);
    /* 使能USART发送完成中断 */
    usart_interrupt_enable(RS485_COM, USART_INT_TC);

    /* 初始化DMA */
    bsp_rs485_dma_init();

    /* 使能USART外设 */
    usart_enable(RS485_COM);
}

void bsp_rs485_init1(uint32_t Baudrate, uint16_t parity_bit, uint8_t DMA_enable)
{
    /* 使能GPIO和USART时钟 */
    rcu_periph_clock_enable(RS485_GPIO_CLK);
    rcu_periph_clock_enable(RS485_COM_CLK);

    /* 配置TX引脚 */
    gpio_af_set(RS485_GPIO_PORT, RS485_AF, RS485_TX_PIN);
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS485_TX_PIN);
    gpio_output_options_set(RS485_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, RS485_TX_PIN);

    /* 配置RX引脚 */
    gpio_af_set(RS485_GPIO_PORT, RS485_AF, RS485_RX_PIN);
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS485_RX_PIN);
    gpio_output_options_set(RS485_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, RS485_RX_PIN);

    /* 配置RS485方向控制引脚 */
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS485_RE_PIN);
    gpio_output_options_set(RS485_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, RS485_RE_PIN);

    /* 默认设置为接收模式 */
    bsp_rs485_receive_mode();

    /* USART配置 */
    usart_deinit(RS485_COM);
    usart_baudrate_set(RS485_COM, Baudrate);
    usart_receive_config(RS485_COM, USART_RECEIVE_ENABLE);
    usart_transmit_config(RS485_COM, USART_TRANSMIT_ENABLE);
	// 校验位配置
	if(parity_bit == PTY_NONE)
		usart_parity_config(RS485_COM,USART_PM_NONE);
	else if(parity_bit == PTY_ODD)
		usart_parity_config(RS485_COM,USART_PM_ODD);
	else if(parity_bit == PTY_EVEN)
		usart_parity_config(RS485_COM,USART_PM_EVEN);
	
  
    /* 使能USART空闲中断 */
    usart_interrupt_enable(RS485_COM, USART_INT_IDLE);
    /* 使能USART发送完成中断 */
    usart_interrupt_enable(RS485_COM, USART_INT_TC);
	if(DMA_enable)
	{
		/* 初始化DMA */
		bsp_rs485_dma_init();
	}
    /* 使能USART外设 */
    usart_enable(RS485_COM);
}

void bsp_rs485_dma_init(void)
{
    /* 使能DMA时钟 */
    rcu_periph_clock_enable(RS485_RX_DMA_CLK);
    rcu_periph_clock_enable(RS485_TX_DMA_CLK);
    rcu_periph_clock_enable(RCU_DMAMUX); /* 使能DMAMUX时钟 */

    dma_parameter_struct dma_init_struct;

    /********************* 初始化接收通道（USART1 RX）*********************/
    dma_deinit(RS485_RX_DMA, RS485_RX_DMA_CH);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.request = DMA_REQUEST_USART1_RX;
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rs485_rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = RS485_DMA_SIZE;
    dma_init_struct.periph_addr = (uint32_t)RS485_RDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_init(RS485_RX_DMA, RS485_RX_DMA_CH, &dma_init_struct);
    dma_circulation_disable(RS485_RX_DMA, RS485_RX_DMA_CH);

    /* 配置DMAMUX多路复用器 */
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH2); /* 禁用通道2的同步触发模式 */

    /* 使能USART1的DMA接收功能 */
    usart_dma_receive_config(RS485_COM, USART_RECEIVE_DMA_ENABLE);

    /* 启动DMA接收通道 */
    dma_channel_enable(RS485_RX_DMA, RS485_RX_DMA_CH);

    /********************* 初始化发送通道（USART1 TX）*********************/
    dma_deinit(RS485_TX_DMA, RS485_TX_DMA_CH);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.request = DMA_REQUEST_USART1_TX;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.periph_addr = (uint32_t)RS485_TDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_init(RS485_TX_DMA, RS485_TX_DMA_CH, &dma_init_struct);
    dma_circulation_disable(RS485_TX_DMA, RS485_TX_DMA_CH);

    /* 配置DMAMUX多路复用器 */
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH3); /* 禁用通道3的同步触发模式 */

    /* 配置DMA传输完成中断 */
    dma_interrupt_disable(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FTF); // 初始禁用
    dma_interrupt_enable(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FTF);  // 使能中断类型

    /* 清除DMA中断标志 */
    dma_interrupt_flag_clear(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FLAG_G);

    /* 使能USART1的DMA发送功能 */
    usart_dma_transmit_config(RS485_COM, USART_TRANSMIT_DMA_ENABLE);
}

void bsp_rs485_dma_send(uint8_t *buffer, uint32_t length)
{
    // 确保长度不超过缓冲区
    if (length > RS485_BUFFER_SIZE)
        length = RS485_BUFFER_SIZE;

    /* 等待上一次发送完成 */
    while (rs485_tx_flag == RESET)
    {
        __NOP();
    }

    rs485_tx_flag = RESET;

    /* 切换到发送模式 */
    bsp_rs485_send_mode();

    /* 配置DMA传输参数 */
    dma_channel_disable(RS485_TX_DMA, RS485_TX_DMA_CH); // 先禁用通道
    dma_transfer_number_config(RS485_TX_DMA, RS485_TX_DMA_CH, length);
    dma_memory_address_config(RS485_TX_DMA, RS485_TX_DMA_CH, (uint32_t)buffer);

    /* 清除中断标志 */
    dma_interrupt_flag_clear(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FLAG_G);

    /* 使能DMA传输完成中断 */
    dma_interrupt_enable(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FTF);

    /* 启动DMA发送 */
    dma_channel_enable(RS485_TX_DMA, RS485_TX_DMA_CH);
}

void bsp_rs485_send_mode(void)
{
    /* 设置方向控制引脚为发送模式 */
    gpio_bit_set(RS485_GPIO_PORT, RS485_RE_PIN);
}

void bsp_rs485_receive_mode(void)
{
    /* 设置方向控制引脚为接收模式 */
    gpio_bit_reset(RS485_GPIO_PORT, RS485_RE_PIN);
}