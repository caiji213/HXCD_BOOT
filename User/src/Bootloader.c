#include "Bootloader.h"
//#include "stm32f4xx.h"
//#include "stm32f4xx_crc.h"
//#include "User_DMA.h"
//#include "User_GPIO.h"
//#include "User_I2C.h"
//#include "User_uart.h"
//#include "User_tim.h"

unsigned short FlashSectors_Row[12] = {FLASH_Sector_0,FLASH_Sector_1,FLASH_Sector_2,FLASH_Sector_3,FLASH_Sector_4,FLASH_Sector_5,FLASH_Sector_6,FLASH_Sector_7,FLASH_Sector_8,FLASH_Sector_9,FLASH_Sector_10,FLASH_Sector_11};

#define FlashSectors_APP_Begin 4 //App起始Flash号
#define FlashSectors_APP_End   9 //App终止Flash号

Memory_Info App_Flash_Info;
Memory_Info Boot_Flash_Info;
Memory_Info Ram_Info;

int Bootloader_Jump_Flag = 0; //跳转标记

Info_t App_Info;
Info_t Boot_Info;

/*
Flash信息，STM32F407，1MB flash
Sector	Begin	End	Size(Dec)	Size(hex)
0	0x08000000	0x08003FFF	16384	0x4000
1	0x08004000	0x08007FFF	16384	0x4000
2	0x08008000	0x0800BFFF	16384	0x4000
3	0x0800C000	0x0800FFFF	16384	0x4000
4	0x08010000	0x0801FFFF	65536	0x10000
5	0x08020000	0x0803FFFF	131072	0x20000
6	0x08040000	0x0805FFFF	131072	0x20000
7	0x08060000	0x0807FFFF	131072	0x20000
8	0x08080000	0x0809FFFF	131072	0x20000
9	0x080A0000	0x080BFFFF	131072	0x20000
10	0x080C0000	0x080DFFFF	131072	0x20000
11	0x080E0000	0x080FFFFF	131072	0x20000

Boot 使用 0~2
App 加密信息使用 3
App 使用 4~9
App 数据信息使用 10~11
*/

void Bootloader_Hal_Init(void)
{

	//Bootloader Flash信息
	Boot_Flash_Info.page_size = 0x4000;
	Boot_Flash_Info.start_addr = Boot_Vector;
	Boot_Flash_Info.size = App_Offset;
	Boot_Flash_Info.end_addr = Boot_Flash_Info.start_addr + Boot_Flash_Info.size - 1;
	Boot_Flash_Info.page_num = Boot_Flash_Info.size / Boot_Flash_Info.page_size;

	//App Flash信息，由于应用App扇区大小不一致，不再按照统一大小区分扇区，将扇区2-11全部划分为App扇区
	App_Flash_Info.page_size = 0;
	App_Flash_Info.start_addr = App_Vector;
	App_Flash_Info.size = 0xB0000UL;
	App_Flash_Info.end_addr = App_Flash_Info.start_addr + App_Flash_Info.size - 1;
	App_Flash_Info.page_num = 0;

	//RAM信息，设置SRAM大小为128kB
	Ram_Info.page_size = 4;
	Ram_Info.start_addr = Sram_UpdateFlag_Vector;			//设置存储升级标记SRAM起始地址为0x2001FFE0
	Ram_Info.size = 0x10;									//设置存储升级标记SRAM大小为16B
	Ram_Info.end_addr = Sram_UpdateFlag_Vector + Ram_Info.size - 1;
	Ram_Info.page_num = Ram_Info.size / Ram_Info.page_size;

	//APP信息，将App程序的大小信息、CRC校验信息全部放在Appflash最后一个扇区的前8个字节，每个信息4字节

	App_Info.addr_size = App_Flash_Info.start_addr + App_Flash_Info.size - 8;
	App_Info.addr_crc = App_Flash_Info.start_addr + App_Flash_Info.size - 4;
	App_Info.size = *((unsigned long *)(App_Info.addr_size));
	App_Info.crc = *((unsigned long *)(App_Info.addr_crc));
	App_Info.hwId = 0x21;

	//Boot信息，将bootloader程序的大小信息、CRC校验信息全部放在bootloader flash扇区的最后8个字节，每个信息4个字节
	Boot_Info.addr_size = Boot_Flash_Info.start_addr + Boot_Flash_Info.size - 8;
	Boot_Info.addr_crc = Boot_Flash_Info.start_addr + Boot_Flash_Info.size - 4;
	Boot_Info.size = *((unsigned long *)(Boot_Info.addr_size));
	Boot_Info.crc = *((unsigned long *)(Boot_Info.addr_crc));
	Boot_Info.hwId = 0x21;
}


/* 检查Boot区域完整性*/
int Bootloader_CheckBoot(void)
{
	unsigned int boot_size,boot_crc;
	unsigned int crc;

	boot_size = Boot_Info.size;
	boot_crc = Boot_Info.crc;

	/* 校验CRC */
	if(boot_size > Boot_Flash_Info.size)
	{
		//比Flash还大，明显错误，不计算
		return 1;
	}
	else
	{
		crc = Bootloader_GetBootCRC();
		return (crc != boot_crc);
	}
}

/* 检查Boot程序的CRC校验结果*/
unsigned long Bootloader_GetBootCRC(void)
{
	unsigned int boot_size;
	unsigned int crc;

	boot_size = Boot_Info.size;

	CRC_ResetDR();										//复位CRC计算结果

	if(Boot_Info.size > Boot_Flash_Info.size)
	{
		//比Flash还大，明显错误，不计算
		crc = 0;
	}
	else
	{
		//使用硬件计算CRC
		crc = CRC_CalcBlockCRC((uint32_t *)(Boot_Flash_Info.start_addr), boot_size / sizeof(uint32_t));
	}

	/* 校验CRC */
	return crc;
}


/* 检查APP区域完整性*/
int Bootloader_CheckApp(void)
{
	unsigned int app_size,app_crc;
	unsigned int crc;

	app_size = App_Info.size;
	app_crc = App_Info.crc;

	/* 校验CRC */
	if(app_size > App_Flash_Info.size)
	{
		//比Flash还大，明显错误，不计算
		return 1;
	}
	else
	{
		__disable_irq();
		crc = Bootloader_GetAppCRC();
		__enable_irq();
		return (crc != app_crc);
	}
}
/* 检查APP程序的CRC结果*/
unsigned long Bootloader_GetAppCRC(void)
{
	unsigned int app_size;
	unsigned int crc;

	app_size = App_Info.size;

	CRC_ResetDR();

	if(App_Info.size > App_Flash_Info.size)
	{
		//比Flash还大，明显错误，不计算
		crc = 0;
	}
	else
	{
		//使用硬件计算CRC
		crc = CRC_CalcBlockCRC((uint32_t *)(App_Flash_Info.start_addr),app_size / sizeof(uint32_t));
	}

	/* 校验CRC */
	return crc;
}

/*
* 擦除APP区域,FLASH_Sector_2到FLASH_Sector_10
*/
int Bootloader_EraseApp(void)
{
	unsigned char index = 0;
	int ret = 0;
	__disable_irq(); //关中断
	//解除flash锁定
	FLASH_Unlock();
	//擦除全部Appflash
	for(index = FlashSectors_APP_Begin;index <= FlashSectors_APP_End;index++)
	{
		if(FLASH_EraseSector(FlashSectors_Row[index],VoltageRange_3) != FLASH_COMPLETE)
		{
			ret = 1;
			break;
		}
	}
	//重新锁定flash
	FLASH_Lock();
	__enable_irq(); //开中断
	return ret;
}


/*
* 擦除全部flash区域,FLASH_Sector_0到FLASH_Sector_11
*/
int Bootloader_EraseAllFlash(void)
{
	unsigned char index = 0;
	int ret = 0;
	__disable_irq(); //关中断
	//解除flash锁定
	FLASH_Unlock();
	//擦除全部Appflash
	for(index = 0;index < 12;index++)
	{
		if(FLASH_EraseSector(FlashSectors_Row[index],VoltageRange_3) != FLASH_COMPLETE)
		{
			ret = 1;
			break;
		}
	}
	//重新锁定flash
	FLASH_Lock();
	__enable_irq(); //开中断
	return ret;
}


/*校验flash写入数据是否正确*/
FLASH_Status FLASH_Varify_ProgramWord(unsigned long Address, unsigned long programword)
{
	FLASH_Status status;
	unsigned long varification = 0;
	varification = * (unsigned long *)(Address);
	if(varification != programword)
		status = FLASH_ERROR_PROGRAM;
	else
		status = FLASH_COMPLETE;
	return status;
}


/* 整块烧录，size必须为4字节整数倍 */
int Bootloader_ProgramBlock(unsigned char * buf, unsigned long address, unsigned int size)
{
	unsigned long * words;
	int i,ret=0;
	if(size % 4)
	{
		return 1;
	}

	//转换为32位数据的指针
	words = (unsigned long *)buf;

	__disable_irq(); //关中断
	//解除flash锁定
	FLASH_Unlock();
	//写入flash数据
	for (i = 0; i < size; i+=4)
	{
		if(FLASH_ProgramWord(App_Flash_Info.start_addr + address + i,words[i/4]) != FLASH_COMPLETE)
		{
			//flash数据写入失败
			ret = 1;
			break;
		}
//		if(FLASH_Varify_ProgramWord(App_Flash_Info.start_addr + address + i,words[i/4]) != FLASH_COMPLETE)
//		{
//			//flash数据校验失败
//			ret = 1;
//			break;
//		}
	}
	//重新锁定flash
	FLASH_Lock();
	__enable_irq(); //开中断

	return ret;
}

/*获取存储在flash中的App CRC校验结果*/
unsigned long Bootloader_Read_Stored_CRC(void)
{
	return App_Info.crc;
}
/*App CRC校验结果写入flash*/
int Bootloader_Write_App_CRC(unsigned long crc)
{
	int ret;
	__disable_irq(); //关中断
	//解除flash锁定
	FLASH_Unlock();
	if(FLASH_ProgramWord(App_Info.addr_crc,crc) == FLASH_COMPLETE)
		ret = 0;
	else
		ret = -1;
	//重新锁定flash
	FLASH_Lock();
	__enable_irq(); //开中断
	if (ret == 0)
	{
		App_Info.crc = crc;
	}
	return ret;
}
/*读取App程序大小*/
unsigned long Bootloader_Read_App_Size(void)
{
	return App_Info.size;
}
/*写入App程序大小*/
int Bootloader_Write_App_Size(unsigned long size)
{
	int ret;
	__disable_irq(); //关中断
	//解除flash锁定
	FLASH_Unlock();
	if(FLASH_ProgramWord(App_Info.addr_size,size) == FLASH_COMPLETE)
		ret = 0;
	else
		ret = -1;
	//重新锁定flash
	FLASH_Lock();
	__enable_irq(); //开中断
	if (ret == 0)
	{
		App_Info.size = size;
	}
	return ret;
}

int Bootloader_Get_Jump_Flag(void)
{
	return Bootloader_Jump_Flag;
}

void Bootloader_Set_Jump_Flag(int value)
{
	Bootloader_Jump_Flag = value;
}

unsigned int Bootloader_GethwID(void)
{
	return 0x21;
}

/*跳转到应用App，跳转之前先关闭已经使用的外设，注意，该函数只能在中断外执行，否则跳转后无法再进入中断*/
void Bootloader_RunAPP(void)
{
	const vector_t *vector_p            = (vector_t*) App_Vector;
	volatile uint32_t stack_arr[100]    = {0}; // Allocate some stack
                                               // just to show that
                                               // the SP should be reset
                                               // before the jump - or the
                                               // stack won't be configured
                                               // correctly.

	__disable_irq();              		// 1. Disable interrupts

	DMA_Deinit();
	UART_Deinit();
	GPIO_DeInitAll();
	TIMx_Deinit();
	I2C_DeInit_All();

    __set_MSP(vector_p->stack_addr);     	// 2. Configure stack pointer
    SCB->VTOR = App_Vector;             	// 3. Configure VTOR
    vector_p->func_p();                 	// 4. Jump to application
}


/*跳转到Bootloader，跳转之前先关闭已经使用的外设，注意，该函数只能在中断外执行，否则跳转后无法再进入中断*/
void Bootloader_RunBootloader(void)
{
	const vector_t *vector_p            = (vector_t*) Boot_Vector;
	volatile uint32_t stack_arr[100]    = {0}; // Allocate some stack
                                               // just to show that
                                               // the SP should be reset
                                               // before the jump - or the
                                               // stack won't be configured
                                               // correctly.

	__disable_irq();              		// 1. Disable interrupts

	DMA_Deinit();
	UART_Deinit();
	GPIO_DeInitAll();
	TIMx_Deinit();

    __set_MSP(vector_p->stack_addr);     	// 2. Configure stack pointer
    SCB->VTOR = Boot_Vector;             	// 3. Configure VTOR
    vector_p->func_p();                 	// 4. Jump to application
}




