/*!
	\file    main.c
	\brief   HXCD Servo motor magnetic braiding bootlaoder

	\version 2025-06-05, V1.1.0, firmware for GD32G5x3d

*/

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "bsp.h"
#include "systick.h"
#include "gd32g5x3.h"
#include "gd32g5x3_it.h"
#include "User_modbus.h"
#include "Bootloader.h"
#include "Bootloader_Check_Force.h"
#include "User_modbus_command_decode.h"

char str_DeviceInfo[64] = "HXCD_BOOT ";
uint8_t buf[2] = {0, 0};

#define ARRAYNUM(arr_nanme) (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))

volatile uint32_t RunAPP_Flag;
volatile uint32_t Boot_Para;
/* 备份寄存器处理函数 */
static void update_Boot_Para(uint32_t value)
{
    pmu_backup_write_enable();
    RTC_BKP4 = value;
    Boot_Para = value;
	pmu_backup_write_disable();
}
static void update_RunAPP_Flag(uint32_t value)
{
    pmu_backup_write_enable();
    RTC_BKP3 = value;
    RunAPP_Flag = value;
	pmu_backup_write_disable();
}

int main(void)
{
	uint32_t last_tick = g_sys_tick; // 时间计数
	int DoNotCheckTxRxShort = 0;

	__disable_irq();			     // 关闭系统总中断
	SCB->VTOR = BOOT_START_ADDR;     // 更改中断向量地址
    bkp_init();

    // 读 BKP 寄存器
	RunAPP_Flag = RTC_BKP3;
	Boot_Para= RTC_BKP4;
	// 滴答定时器初始化
	systick_config();

	Bootloader_Hal_Init();

	if (RunAPP_Flag == FLAG_RUNAPP_FORCE)
	{
		// 强制进入App，用于参数初始化重启
		Bootloader_RunAPP();
	}
	else if (RunAPP_Flag == FLAG_RUNBOOT)
	{
		// 有升级标记，需要留在Boot里
		// 初始化外设
		bsp_init(Boot_Para & 0xFFFFFF);
		ModBus_Init(Boot_Para >> 24);
		bsp_crc_init();
		// 回复已跳转到Bootloader
		ModBus_Command_Decode_Feedback_JumpBootloader(rs485_rxbuffer, Boot_Para >> 24, bsp_rs485_dma_send); // 升级跳入后第一次485回复
		ModBus_Command_Decode_Feedback_JumpBootloader(rs232_rxbuffer, Boot_Para >> 24, bsp_rs232_dma_send); // 升级跳入后第一次232回复
		DoNotCheckTxRxShort = 1;
	}
	else
	{
		bsp_rs232_init(115200, PTY_NONE, DMA_ENABLE);
		bsp_rs485_init(115200, PTY_NONE, DMA_ENABLE);
		bsp_crc_init(); // 硬件crc计算
		bsp_i2c_init(); // I2C初始化
		nvic_config();	// 中断配置
		__enable_irq(); // 开中断
		ModBus_Init(1);
		//printf("[boot] bsp_rs232_init,bsp_rs485_init\r\n");
	}

	if (DoNotCheckTxRxShort)
	{
		// 进入这里理论上应该是 FLAG_RUNBOOT == RunAPP_Flag，停留在Boot中
	}

	// 检查是否强制进入Bootloader
	else if (Bootloader_Check_Force())
	{
		update_RunAPP_Flag(0xFFFFFFFF);   // 设置运行标记为-1
		eeprom_buffer_write(buf, 0xFFE, 2); // 读原值，主要为调试用
		// 写IIC的初始化参数标志，再重启由APP进行初始化参数
		delay_1ms(5);
		eeprom_byte_write(0xFF, 0xFFE);
		delay_1ms(5);
		eeprom_byte_write(0xFF, 0xFFF);
		delay_1ms(5);
		// 停留在Bootloader中，可以进行升级
	}

	else if ((Bootloader_CheckApp() == 0))
	{
		// 检查app完整性正确，运行App
		update_RunAPP_Flag(FLAG_RUNAPP);
		Bootloader_RunAPP();
	}
	else
	{
		// 检查app失败进入BootLoader
		//RunAPP_Flag = FLAG_CRC_ERROR;
		update_RunAPP_Flag(FLAG_CRC_ERROR);
	}

	// 把设备信息加上编译日期
	strcat(str_DeviceInfo, __DATE__);
	strcat(str_DeviceInfo, " ");
	strcat(str_DeviceInfo, __TIME__);

	while (1)
	{
		// modbus数据处理函数
		ModBus_Slave_Process();
		// 检查跳转APP的标志
		if (Bootloader_Get_Jump_Flag())
		{
			//RunAPP_Flag = FLAG_GO_APP;
			update_RunAPP_Flag(FLAG_GO_APP);
			Bootloader_Set_Jump_Flag(0);
			Bootloader_RunAPP();
		}
	}
}

/* 将C库printf函数重定向到USART */
int fputc(int ch, FILE *f)
{
	usart_data_transmit(RS232_COM, (uint8_t)ch);
	while (RESET == usart_flag_get(RS232_COM, USART_FLAG_TBE))
		;

	return ch;
}
