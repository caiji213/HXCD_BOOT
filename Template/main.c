/*!
    \file    main.c
    \brief   HXCD Servo motor magnetic braiding bootlaoder

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "bsp.h"
#include "systick.h"
#include "gd32g5x3.h"
#include "gd32g5x3_it.h"
#include "Bootloader.h"
#include "Bootloader_Check_Force.h"
#include "User_modbus.h"
#include "User_modbus_command_decode.h"

char str_DeviceInfo[64] = "ARM_Boot ";
uint8_t buf[2]={0,0};

#define ARRAYNUM(arr_nanme) (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
	
__attribute__((section(".ARM.__at_0x20000000"))) uint32_t RunAPP_Flag;
__attribute__((section(".ARM.__at_0x20000004"))) uint32_t Boot_Para;

int main(void)
{
	// 变量定义
	uint32_t last_tick = g_sys_tick;  // 时间计数
	int DoNotCheckTxRxShort = 0;
	
	
	__disable_irq();			//关闭系统总中断
	SCB->VTOR = Boot_Vector; 	//更改中断向量地址
	
	// 初始化
    systick_config();    // 滴答定时器初始化
	bsp_init(115200);    //外设初始化+rs485波特率设置
	ModBus_Init(1);      // modbus初始化
	Bootloader_Hal_Init(); 

    while (1)
    {
		if ((g_sys_tick - last_tick) >= 1)
        {
          last_tick = g_sys_tick;
		  //modbus数据处理函数
		  ModBus_Slave_Process();

        }
    }
}

/* 将C库printf函数重定向到USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(RS232_COM, (uint8_t)ch);
    while (RESET == usart_flag_get(RS232_COM, USART_FLAG_TBE));

    return ch;
}
