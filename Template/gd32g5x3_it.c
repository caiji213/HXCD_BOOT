/*!
    \file    gd32g5x3_it.c
    \brief   interrupt service routines

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/
#include "gd32g5x3_it.h"
#include "User_modbus.h"

volatile uint32_t g_sys_tick = 0; // 全局系统滴答计数器

#define SRAM_ECC_ERROR_HANDLE(s) \
    do                           \
    {                            \
    } while (1)

#define FLASH_ECC_ERROR_HANDLE(s) \
    do                            \
    {                             \
    } while (1)

/*!
    \brief      此函数处理NMI异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void NMI_Handler(void)
{
    if (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_TCMSRAMECCME))
    {
        SRAM_ECC_ERROR_HANDLE("TCMSRAM multi-bits non-correction ECC error\r\n");
    }
    else if (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_SRAM1ECCME))
    {
        SRAM_ECC_ERROR_HANDLE("SRAM1 multi-bits non-correction ECC error\r\n");
    }
    else if (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_SRAM0ECCME))
    {
        SRAM_ECC_ERROR_HANDLE("SRAM0 multi-bits non-correction ECC error\r\n");
    }
    else if (SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_FLASHECC))
    {
        FLASH_ECC_ERROR_HANDLE("FLASH ECC error\r\n");
    }
    else
    {
        /* 如果发生NMI异常，进入无限循环 */
        /* HXTAL时钟监控NMI错误或NMI引脚错误 */
        while (1)
        {
        }
    }
}

/*!
    \brief      此函数处理HardFault异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void HardFault_Handler(void)
{
    /* 如果发生Hard Fault异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理内存管理异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void MemManage_Handler(void)
{
    /* 如果发生内存管理异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理总线错误异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void BusFault_Handler(void)
{
    /* 如果发生总线错误异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理使用错误异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void UsageFault_Handler(void)
{
    /* 如果发生使用错误异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理SVC异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void SVC_Handler(void)
{
    /* 如果发生SVC异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理调试监控异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void DebugMon_Handler(void)
{
    /* 如果发生调试监控异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理PendSV异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void PendSV_Handler(void)
{
    /* 如果发生PendSV异常，进入无限循环 */
    while (1)
    {
    }
}

/*!
    \brief      此函数处理SysTick异常
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void SysTick_Handler(void)
{
    delay_decrement();
    g_sys_tick++; // 增加系统滴答计数
}

/* USART0中断服务函数 */
void USART0_IRQHandler(void)
{
    /* 空闲中断处理 */
    if (RESET != usart_interrupt_flag_get(RS232_COM, USART_INT_FLAG_IDLE))
    {
        /* 清除空闲中断标志 */
        usart_interrupt_flag_clear(RS232_COM, USART_INT_FLAG_IDLE);

        /* 计算接收到的数据数量 */
        rx_count = RS232_DMA_SIZE - (dma_transfer_number_get(RS232_RX_DMA, RS232_RX_DMA_CH));
        rs232_idle_flag = 1;

        /* 禁用DMA并重新配置 */
        dma_channel_disable(RS232_RX_DMA, RS232_RX_DMA_CH);
        dma_transfer_number_config(RS232_RX_DMA, RS232_RX_DMA_CH, RS232_DMA_SIZE);
        dma_flag_clear(RS232_RX_DMA, RS232_RX_DMA_CH, DMA_FLAG_FTF);
        dma_channel_enable(RS232_RX_DMA, RS232_RX_DMA_CH);
    }
    /* 发送完成中断处理 */
    if (RESET != usart_interrupt_flag_get(RS232_COM, USART_INT_FLAG_TC))
    {
        /* 清除发送完成中断标志 */
        usart_interrupt_flag_clear(RS232_COM, USART_INT_FLAG_TC);

        /* 禁用UART发送完成中断 */
        usart_interrupt_disable(RS232_COM, USART_INT_TC);

        /* 设置发送完成标志 */
        rs232_tx_flag = SET;
    }
    // 处理溢出错误
    if (usart_flag_get(RS232_COM, USART_FLAG_ORERR))
    {
        usart_flag_clear(RS232_COM, USART_FLAG_ORERR);
    }
    // 处理噪声错误
    if (usart_flag_get(RS232_COM, USART_FLAG_NERR))
    {
        usart_flag_clear(RS232_COM, USART_FLAG_NERR);
    }
    // 处理帧错误
    if (usart_flag_get(RS232_COM, USART_FLAG_FERR))
    {
        usart_flag_clear(RS232_COM, USART_FLAG_FERR);
    }
    // 处理校验错误
    if (usart_flag_get(RS232_COM, USART_FLAG_PERR))
    {
        usart_flag_clear(RS232_COM, USART_FLAG_PERR);
    }
}

// RS232_TX_DMA DMA中断服务函数
void DMA0_Channel0_IRQHandler(void)
{
    /* 检查DMA传输完成标志 */
    if (dma_interrupt_flag_get(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FLAG_FTF))
    {
        /* 清除中断标志 */
        dma_interrupt_flag_clear(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FLAG_FTF);

        /* 禁用DMA通道 */
        dma_channel_disable(RS232_TX_DMA, RS232_TX_DMA_CH);

        /* 禁用DMA中断 */
        dma_interrupt_disable(RS232_TX_DMA, RS232_TX_DMA_CH, DMA_INT_FTF);

        /* 开启UART发送完成中断 */
        usart_interrupt_enable(RS232_COM, USART_INT_TC);
    }
}

/* USART1中断服务函数 */
void USART1_IRQHandler(void)
{
    /* 空闲中断处理 */
    if (RESET != usart_interrupt_flag_get(RS485_COM, USART_INT_FLAG_IDLE))
    {
        /* 清除空闲中断标志 */
        usart_interrupt_flag_clear(RS485_COM, USART_INT_FLAG_IDLE);

        /* 计算接收到的数据数量 */
        rs485_rx_count = RS485_DMA_SIZE - dma_transfer_number_get(RS485_RX_DMA, RS485_RX_DMA_CH);
        rs485_idle_flag = SET;

        /* 禁用DMA并重新配置 */
        dma_channel_disable(RS485_RX_DMA, RS485_RX_DMA_CH);
        dma_transfer_number_config(RS485_RX_DMA, RS485_RX_DMA_CH, RS485_DMA_SIZE);
        dma_flag_clear(RS485_RX_DMA, RS485_RX_DMA_CH, DMA_FLAG_FTF);
        dma_channel_enable(RS485_RX_DMA, RS485_RX_DMA_CH);
		
		 /* ========== Modbus 数据处理入口 ========== */
        ModBus_Slave_Received_Data(rs485_rxbuffer, rs485_rx_count, CMD_FROM_USART1);
    }

    /* 发送完成中断处理 */
    if (RESET != usart_interrupt_flag_get(RS485_COM, USART_INT_FLAG_TC))
    {
        /* 清除发送完成中断标志 */
        usart_interrupt_flag_clear(RS485_COM, USART_INT_FLAG_TC);

        /* 禁用UART发送完成中断 */
        usart_interrupt_disable(RS485_COM, USART_INT_TC);
        /* 切换回接收模式 */
        bsp_rs485_receive_mode();
        /* 更新标志 */
        rs485_tx_flag = SET;
		
		/* ========== Modbus 发送完成回调 ========== */
        ModBus_Slave_Send_Finished();
    }

    // 处理溢出错误
    if (usart_flag_get(RS485_COM, USART_FLAG_ORERR))
    {
        usart_flag_clear(RS485_COM, USART_FLAG_ORERR);
    }
    // 处理噪声错误
    if (usart_flag_get(RS485_COM, USART_FLAG_NERR))
    {
        usart_flag_clear(RS485_COM, USART_FLAG_NERR);
    }
    // 处理帧错误
    if (usart_flag_get(RS485_COM, USART_FLAG_FERR))
    {
        usart_flag_clear(RS485_COM, USART_FLAG_FERR);
    }
    // 处理校验错误
    if (usart_flag_get(RS485_COM, USART_FLAG_PERR))
    {
        usart_flag_clear(RS485_COM, USART_FLAG_PERR);
    }
}

/* RS485_TX_DMA DMA0通道3中断服务函数 */
void DMA0_Channel3_IRQHandler(void)
{
    if (dma_interrupt_flag_get(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FLAG_FTF))
    {
        /* 清除中断标志 */
        dma_interrupt_flag_clear(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FLAG_FTF);

        /* 禁用DMA通道 */
        dma_channel_disable(RS485_TX_DMA, RS485_TX_DMA_CH);

        /* 禁用DMA中断 */
        dma_interrupt_disable(RS485_TX_DMA, RS485_TX_DMA_CH, DMA_INT_FTF);

        /* 启用串口接收完成中断用于状态更新 */
        usart_interrupt_enable(RS485_COM, USART_INT_TC);
    }
}

/* I²C TX DMA0通道4中断服务函数 */
void DMA0_Channel4_IRQHandler(void)
{
    if (dma_interrupt_flag_get(I2C_DMAx, I2C_DMA_TX_CH, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(I2C_DMAx, I2C_DMA_TX_CH, DMA_INT_FLAG_FTF);
        i2c_dma_tx_callback(); // 调用TX完成回调
    }
}

/* I²C TX DMA0通道5中断服务函数 */
void DMA0_Channel5_IRQHandler(void)
{
    if (dma_interrupt_flag_get(I2C_DMAx, I2C_DMA_RX_CH, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(I2C_DMAx, I2C_DMA_RX_CH, DMA_INT_FLAG_FTF);
        i2c_dma_rx_callback(); // 调用RX完成回调
    }
}
