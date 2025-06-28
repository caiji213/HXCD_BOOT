/*!
    \file    main.c
    \brief   HXCD Servo motor magnetic braiding

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include <stdio.h>

#include "main.h"
#include "bsp.h"
#include "systick.h"
#include "gd32g5x3.h"
#include "gd32g5x3_it.h"

char str_DeviceInfo[64] = "ARM_Boot ";

#define ARRAYNUM(arr_nanme) (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))

/*!
    \brief 打印固件版本号与系统时钟频率
*/
void print_system_info(void)
{
    uint32_t fw_ver = gd32g5x3_firmware_version_get();

    printf("\r\n----------------------------------------");
    printf("\r\nHXCD_GD32G5X3 firmware version: V%d.%d.%d",
           (uint8_t)(fw_ver >> 24),
           (uint8_t)(fw_ver >> 16),
           (uint8_t)(fw_ver >> 8));

    printf("\r\nSystem Clocks:");
    printf("\r\n  CK_SYS  = %d Hz", rcu_clock_freq_get(CK_SYS));
    printf("\r\n  CK_AHB  = %d Hz", rcu_clock_freq_get(CK_AHB));
    printf("\r\n  CK_APB1 = %d Hz", rcu_clock_freq_get(CK_APB1));
    printf("\r\n  CK_APB2 = %d Hz", rcu_clock_freq_get(CK_APB2));
    printf("\r\n----------------------------------------\r\n");
}

int main(void)
{
	// 变量定义
	uint32_t last_tick = g_sys_tick;  // 时间计数
    const uint8_t rs232_txbuffer[] = "RS232 DMA with IDLE interrupt\r\n";
	const uint8_t rs485_txbuffer[] = "RS485 DMA with IDLE interrupt\r\n";
	
	// 初始化
    systick_config();
    bsp_init();
	
	// 打印固件版本号与系统时钟频率
    print_system_info();

	/* I2C测试 */
    if(I2C_OK == i2c_24c64_test()) 
	{  
       printf("\r\ni2c_24c64_test succeed %d", I2C_SPEED);
    }
	
    while (1)
    {
        /* RS232测试 */
        if (rs232_idle_flag == SET)
        {
            if(rx_count > 0)
            {
                // 使用DMA发送接收到的数据
                bsp_rs232_dma_send(rs232_rxbuffer, rx_count);
            }
            rs232_idle_flag = RESET;
        }
        
        /* RS485测试 */
        if (rs485_idle_flag == SET)
        {
            if (rs485_rx_count > 0)
            {
                // 使用DMA发送接收到的数据
                bsp_rs485_dma_send(rs485_rxbuffer, rs485_rx_count);
            }
            rs485_idle_flag = RESET;
        }
        /* 主动发送串口数据 */
//        if ((g_sys_tick - last_tick) >= 1000)
//        {
//          last_tick = g_sys_tick;
//			bsp_rs232_dma_send((uint8_t*)rs232_txbuffer, sizeof(rs232_txbuffer)-1);
//			bsp_rs485_dma_send((uint8_t*)rs485_txbuffer, sizeof(rs485_txbuffer)-1);

//        }
        /* SPI测试 */
        if ((g_sys_tick - last_tick) >= 10)
        {
            last_tick = g_sys_tick;
			
			// 启动SSI接收传输
            bsp_spi_transfer(SSI_DATA_SIZE);

           // 处理接收到的数据
           process_spi_data(spi_rx_buffer, SPI_BUFFER_SIZE);

        }
		
		/* PWM测试 + ADC测试*/
//        if ((g_sys_tick - last_tick) >= 10)
//        {
//            last_tick = g_sys_tick;
//            bsp_pwm_test_once();
//        }
    }
}

/* 将C库printf函数重定向到USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(RS232_COM, (uint8_t)ch);
    while (RESET == usart_flag_get(RS232_COM, USART_FLAG_TBE));

    return ch;
}
