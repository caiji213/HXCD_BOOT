#include "Bootloader.h"
//#include "stm32f4xx.h"
//#include "stm32f4xx_crc.h"
//#include "User_DMA.h"
//#include "User_GPIO.h"
//#include "User_I2C.h"
//#include "User_uart.h"
//#include "User_tim.h"
#include "gd32g5x3.h"
#include "gd32g5x3_it.h"

//unsigned short FlashSectors_Row[12] = {FLASH_Sector_0,FLASH_Sector_1,FLASH_Sector_2,FLASH_Sector_3,FLASH_Sector_4,FLASH_Sector_5,FLASH_Sector_6,FLASH_Sector_7,FLASH_Sector_8,FLASH_Sector_9,FLASH_Sector_10,FLASH_Sector_11};

#define FlashSectors_APP_Begin 4 //App��ʼFlash��
#define FlashSectors_APP_End   9 //App��ֹFlash��

Memory_Info App_Flash_Info;
Memory_Info Boot_Flash_Info;
Memory_Info Ram_Info;

int Bootloader_Jump_Flag = 0; //��ת���

Info_t App_Info;
Info_t Boot_Info;

/*
Flash��Ϣ��GD32G553��512KB Flash��˫ Bank ģʽ��DBS=1��
ÿҳ��С��1KB
Bank0��0x08000000 - 0x0803FFFF����256ҳ
Bank1��0x08040000 - 0x0807FFFF����256ҳ

ҳ���	Page	Begin	      End	        Size(Dec)	Size(hex)
Bank0-000	0	0x08000000	0x080003FF	1024	    0x400    <- Boot ��ʼҳ
Bank0-001	1	0x08000400	0x080007FF	1024	    0x400
Bank0-002	2	0x08000800	0x08000BFF	1024	    0x400
...         ...	...	        ...	        ...	        ...
Bank0-047	47	0x0800BC00	0x0800BFFF	1024	    0x400    <- Boot ���һҳ
Bank0-048	48	0x0800C000	0x0800C3FF	1024	    0x400    <- Info ��ʼҳ
...         ...	...	        ...	        ...	        ...
Bank0-060	60	0x0800F000	0x0800F3FF	1024	    0x400    <- Info ���һҳ
Bank0-061	61	0x0800F400	0x0800F7FF	1024	    0x400    <- App ��ʼҳ
...         ...	...	        ...	        ...	        ...
Bank0-255	255	0x0803FC00	0x0803FFFF	1024	    0x400

Bank1-000	0	0x08040000	0x080403FF	1024	    0x400
...         ...	...	        ...	        ...	        ...
Bank1-192	192	0x0807C000	0x0807C3FF	1024	    0x400
...         ...	...	        ...	        ...	        ...
Bank1-252	252	0x0807FC00	0x0807FFFF	1024	    0x400    <- App ���һҳ
Bank1-253	253	0x08080000	0x080803FF	1024	    0x400
Bank1-254	254	0x08080400	0x080807FF	1024	    0x400
Bank1-255	255	0x08080800	0x08080BFF	1024	    0x400    <- Data ���һҳ

�����滮��
Boot ʹ�� Bank0 ҳ 0~47       ��48KB��          -> 0x08000000 ~ 0x0800BFFF
Info ʹ�� Bank0 ҳ 48~60      ��13KB��          -> 0x0800C000 ~ 0x0800F3FF
App  ʹ�� Bank0 ҳ 61~255 + Bank1 ҳ 0~252��448KB�� -> 0x0800F400 ~ 0x0807FFFF
Data ʹ�� Bank1 ҳ 253~255    ��3KB��           -> 0x08080000 ~ 0x08080BFF
*/


void Bootloader_Hal_Init(void)
{

	//Bootloader Flash��Ϣ
	Boot_Flash_Info.page_size = 0x4000;
	Boot_Flash_Info.start_addr = Boot_Vector;
	Boot_Flash_Info.size = App_Offset;
	Boot_Flash_Info.end_addr = Boot_Flash_Info.start_addr + Boot_Flash_Info.size - 1;
	Boot_Flash_Info.page_num = Boot_Flash_Info.size / Boot_Flash_Info.page_size;

	//App Flash��Ϣ������Ӧ��App������С��һ�£����ٰ���ͳһ��С����������������2-11ȫ������ΪApp����
	App_Flash_Info.page_size = 0;
	App_Flash_Info.start_addr = App_Vector;
	App_Flash_Info.size = 0xB0000UL;
	App_Flash_Info.end_addr = App_Flash_Info.start_addr + App_Flash_Info.size - 1;
	App_Flash_Info.page_num = 0;

	//RAM��Ϣ������SRAM��СΪ128kB
	Ram_Info.page_size = 4;
	Ram_Info.start_addr = Sram_UpdateFlag_Vector;			//���ô洢�������SRAM��ʼ��ַΪ0x2001FFE0
	Ram_Info.size = 0x10;									//���ô洢�������SRAM��СΪ16B
	Ram_Info.end_addr = Sram_UpdateFlag_Vector + Ram_Info.size - 1;
	Ram_Info.page_num = Ram_Info.size / Ram_Info.page_size;

	//APP��Ϣ����App����Ĵ�С��Ϣ��CRCУ����Ϣȫ������Appflash���һ��������ǰ8���ֽڣ�ÿ����Ϣ4�ֽ�

	App_Info.addr_size = App_Flash_Info.start_addr + App_Flash_Info.size - 8;
	App_Info.addr_crc = App_Flash_Info.start_addr + App_Flash_Info.size - 4;
	App_Info.size = *((unsigned long *)(App_Info.addr_size));
	App_Info.crc = *((unsigned long *)(App_Info.addr_crc));
	App_Info.hwId = 0x21;

	//Boot��Ϣ����bootloader����Ĵ�С��Ϣ��CRCУ����Ϣȫ������bootloader flash���������8���ֽڣ�ÿ����Ϣ4���ֽ�
	Boot_Info.addr_size = Boot_Flash_Info.start_addr + Boot_Flash_Info.size - 8;
	Boot_Info.addr_crc = Boot_Flash_Info.start_addr + Boot_Flash_Info.size - 4;
	Boot_Info.size = *((unsigned long *)(Boot_Info.addr_size));
	Boot_Info.crc = *((unsigned long *)(Boot_Info.addr_crc));
	Boot_Info.hwId = 0x21;
}


/* ���Boot����������*/
int Bootloader_CheckBoot(void)
{
	unsigned int boot_size,boot_crc;
	unsigned int crc;

	boot_size = Boot_Info.size;
	boot_crc = Boot_Info.crc;

	/* У��CRC */
	if(boot_size > Boot_Flash_Info.size)
	{
		//��Flash�������Դ��󣬲�����
		return 1;
	}
	else
	{
		crc = Bootloader_GetBootCRC();
		return (crc != boot_crc);
	}
}

/* ���Boot�����CRCУ����*/
unsigned long Bootloader_GetBootCRC(void)
{
	unsigned int boot_size;
	unsigned int crc;

	boot_size = Boot_Info.size;

	//CRC_ResetDR();										//��λCRC������
    crc_deinit();                                           // ��λCRC���㵥Ԫ
	
	if(Boot_Info.size > Boot_Flash_Info.size)
	{
		//��Flash�������Դ��󣬲�����
		crc = 0;
	}
	else
	{
		//ʹ��Ӳ������CRC
		//crc = CRC_CalcBlockCRC((uint32_t *)(Boot_Flash_Info.start_addr), boot_size / sizeof(uint32_t));
	    crc = crc_block_data_calculate((void *)Boot_Flash_Info.start_addr, boot_size, INPUT_FORMAT_WORD);
	}

	/* У��CRC */
	return crc;
}


/* ���APP����������*/
int Bootloader_CheckApp(void)
{
	unsigned int app_size,app_crc;
	unsigned int crc;

	app_size = App_Info.size;
	app_crc = App_Info.crc;

	/* У��CRC */
	if(app_size > App_Flash_Info.size)
	{
		//��Flash�������Դ��󣬲�����
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
/* ���APP�����CRC���*/
unsigned long Bootloader_GetAppCRC(void)
{
	unsigned int app_size;
	unsigned int crc;

	app_size = App_Info.size;

	//CRC_ResetDR();
    crc_deinit(); 
	
	if(App_Info.size > App_Flash_Info.size)
	{
		//��Flash�������Դ��󣬲�����
		crc = 0;
	}
	else
	{
		//ʹ��Ӳ������CRC
		//crc = CRC_CalcBlockCRC((uint32_t *)(App_Flash_Info.start_addr),app_size / sizeof(uint32_t));
		crc = crc_block_data_calculate((void *)App_Flash_Info.start_addr, app_size, INPUT_FORMAT_WORD);
	}

	/* У��CRC */
	return crc;
}

/*
* ����APP����,FLASH_Sector_2��FLASH_Sector_10
*/
int Bootloader_EraseApp(void)
{
	unsigned char index = 0;
	int ret = 0;
	__disable_irq(); //���ж�
	//���flash����
	fmc_unlock();
	//����ȫ��Appflash
	for(index = FlashSectors_APP_Begin;index <= FlashSectors_APP_End;index++)
	{
//		if(FLASH_EraseSector(FlashSectors_Row[index],VoltageRange_3) != FLASH_COMPLETE)
//		{
//			ret = 1;
//			break;
//		}
	}
	//��������flash
	fmc_lock();
	__enable_irq(); //���ж�
	return ret;
}


/*
* ����ȫ��flash����,FLASH_Sector_0��FLASH_Sector_11
*/
int Bootloader_EraseAllFlash(void)
{
	unsigned char index = 0;
	int ret = 0;
	__disable_irq(); //���ж�
	//���flash����
	fmc_unlock();
	//����ȫ��Appflash
	for(index = 0;index < 12;index++)
	{
//		if(FLASH_EraseSector(FlashSectors_Row[index],VoltageRange_3) != FLASH_COMPLETE)
//		{
//			ret = 1;
//			break;
//		}
	}
	//��������flash
	fmc_lock();
	__enable_irq(); //���ж�
	return ret;
}


/*У��flashд�������Ƿ���ȷ*/
//FLASH_Status FLASH_Varify_ProgramWord(unsigned long Address, unsigned long programword)
//{
//	FLASH_Status status;
//	unsigned long varification = 0;
//	varification = * (unsigned long *)(Address);
//	if(varification != programword)
//		status = FLASH_ERROR_PROGRAM;
//	else
//		status = FLASH_COMPLETE;
//	return status;
//}


/* ������¼��size����Ϊ4�ֽ������� */
int Bootloader_ProgramBlock(unsigned char * buf, unsigned long address, unsigned int size)
{
	unsigned long * words;
	int i,ret=0;
	if(size % 4)
	{
		return 1;
	}

	//ת��Ϊ32λ���ݵ�ָ��
	words = (unsigned long *)buf;

	__disable_irq(); //���ж�
	//���flash����
	fmc_unlock();
	//д��flash����
	for (i = 0; i < size; i+=4)
	{
//		if(FLASH_ProgramWord(App_Flash_Info.start_addr + address + i,words[i/4]) != FLASH_COMPLETE)
//		{
//			//flash����д��ʧ��
//			ret = 1;
//			break;
//		}
//		if(FLASH_Varify_ProgramWord(App_Flash_Info.start_addr + address + i,words[i/4]) != FLASH_COMPLETE)
//		{
//			//flash����У��ʧ��
//			ret = 1;
//			break;
//		}
	}
	//��������flash
	fmc_lock();
	__enable_irq(); //���ж�

	return ret;
}

/*��ȡ�洢��flash�е�App CRCУ����*/
unsigned long Bootloader_Read_Stored_CRC(void)
{
	return App_Info.crc;
}
/*App CRCУ����д��flash*/
int Bootloader_Write_App_CRC(unsigned long crc)
{
	int ret;
	__disable_irq(); //���ж�
	//���flash����
	fmc_unlock();
//	if(FLASH_ProgramWord(App_Info.addr_crc,crc) == FLASH_COMPLETE)
//		ret = 0;
//	else
//		ret = -1;
	//��������flash
	fmc_lock();
	__enable_irq(); //���ж�
	if (ret == 0)
	{
		App_Info.crc = crc;
	}
	return ret;
}
/*��ȡApp�����С*/
unsigned long Bootloader_Read_App_Size(void)
{
	return App_Info.size;
}
/*д��App�����С*/
int Bootloader_Write_App_Size(unsigned long size)
{
	int ret;
	__disable_irq(); //���ж�
	//���flash����
	fmc_unlock();
//	if(FLASH_ProgramWord(App_Info.addr_size,size) == FLASH_COMPLETE)
//		ret = 0;
//	else
//		ret = -1;
	//��������flash
	fmc_lock();
	__enable_irq(); //���ж�
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

/*��ת��Ӧ��App����ת֮ǰ�ȹر��Ѿ�ʹ�õ����裬ע�⣬�ú���ֻ�����ж���ִ�У�������ת���޷��ٽ����ж�*/
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

//	DMA_Deinit();
//	UART_Deinit();
//	GPIO_DeInitAll();
//	TIMx_Deinit();
//	I2C_DeInit_All();

    __set_MSP(vector_p->stack_addr);     	// 2. Configure stack pointer
    SCB->VTOR = App_Vector;             	// 3. Configure VTOR
    vector_p->func_p();                 	// 4. Jump to application
}


/*��ת��Bootloader����ת֮ǰ�ȹر��Ѿ�ʹ�õ����裬ע�⣬�ú���ֻ�����ж���ִ�У�������ת���޷��ٽ����ж�*/
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

//	DMA_Deinit();
//	UART_Deinit();
//	GPIO_DeInitAll();
//	TIMx_Deinit();

    __set_MSP(vector_p->stack_addr);     	// 2. Configure stack pointer
    SCB->VTOR = Boot_Vector;             	// 3. Configure VTOR
    vector_p->func_p();                 	// 4. Jump to application
}




