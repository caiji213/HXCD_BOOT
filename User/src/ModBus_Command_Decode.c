#include <stdint.h>
#include "ModBus_Command_Decode.h"
#include "User_Convert_Int_BytesArray.h"
#include "User_modbus.h"
#include "Bootloader.h"
#include "Decrypt_Block_Data.h"

//#define DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM //使用外部RAM调试

#define MODBUS_FUNCODE_UPGRADE_INFO  0x41
#define MODBUS_FUNCODE_PROGRAM_FLASH 0x42
#define MODBUS_FUNCODE_GENERAL_FUNC  0x43

#define UPGRADE_OP_CODE_READ_APP_LENGTH   0x0001 //读APP长度
#define UPGRADE_OP_CODE_WRITE_APP_LENGTH  0x8001 //写APP长度
#define UPGRADE_OP_CODE_READ_HW_ID        0x001D //读硬件识别码
#define UPGRADE_OP_CODE_WRITE_HW_ID       0x801D //写硬件识别码
#define UPGRADE_OP_CODE_APP_CHECKSUM      0x00C0 //获取APP校验
#define UPGRADE_OP_CODE_CHECK_UPGRADE     0x80C0 //校验升级
#define UPGRADE_OP_CODE_READ_STORED_CRC   0x00C1 //读存储的校验结果
#define UPGRADE_OP_CODE_WRITE_STORED_CRC  0x80C1 //写存储的校验结果
#define UPGRADE_OP_CODE_ERASE_APP         0x00F0 //擦除APP区域的Flash
#define UPGRADE_OP_CODE_ERASE_UPDATE_FLAG 0x00F1 //擦除升级标记
#define UPGRADE_OP_CODE_ERASE_FLASH       0x00FF //擦除指定区域的Flash
#define UPGRADE_OP_CODE_JUMP_TO_APP       0x474F //跳转到APP

typedef struct
{
	unsigned int hwId;
	unsigned int size;
	unsigned int crc;
}Upgrade_Info_t;

Upgrade_Info_t UpgradeInfo;

#ifdef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
#include "gd32f10x.h"
unsigned long GetAppCRC_From_Ram(void)
{
	unsigned int app_size;
	unsigned int crc;

	app_size = UpgradeInfo.size;

	crc_data_register_reset();

	if(UpgradeInfo.size > 262144)
	{
		//比Flash还大，明显错误，不计算
		crc = CRC_DATA;
	}
	else
	{
		//使用硬件计算CRC
		crc = crc_block_data_calculate((uint32_t *)(0x60000000),app_size / sizeof(uint32_t));
	}

	/* 校验CRC */
	return crc;
}

int ProgramBlock_to_Ram(unsigned char * buf, unsigned long address, unsigned int size)
{
	if(size % 4)
	{
            return 1;
	}
	
	unsigned int * words;
	int i,ret=0;

	//转换为32位数据的指针
	words = (unsigned int * )buf;
	for (i = 0; i < size; i+=4)
	{
            *((unsigned int *)(0x60000000 + address + i)) = words[i/4];
	}
	return ret;
}
#endif

void ModBus_Command_Decode_Upgrade_Info(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	UpgradeInfo.hwId = BigEndianBytesArray_To_Uint32(buf+3);
	UpgradeInfo.size = BigEndianBytesArray_To_Uint32(buf+7);
	UpgradeInfo.crc = BigEndianBytesArray_To_Uint32(buf+11);
	Decryption_Init(UpgradeInfo.crc);
	Uint32_To_BigEndianBytesArray(buf+3,UpgradeInfo.hwId);
	Feedback(buf,len);
}

//编程Flash
void ModBus_Command_Decode_Program_Flash(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int addr,size;
	int ret=0;
	addr = BigEndianBytesArray_To_Uint32(buf+2);
	size = BigEndianBytesArray_To_Uint16(buf+6);

	UI_BootLoader_Set_Program_Info(addr+size);

	//解密数据
	Decrypt_Block_Data((uint32_t *)(buf + 8),size/4);

	//开始烧录
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	ret = Bootloader_ProgramBlock(buf+8,addr,size);
	#else
	ret = ProgramBlock_to_Ram(buf+8,addr,size);
	#endif
	if (ret)
	{
		//烧录出错,反馈烧录了0字节
		Uint16_To_BigEndianBytesArray(buf+6, 0);
	}

	len = 10;
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_Read_App_Length(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int size;
	size = Bootloader_Read_App_Size();
	Uint32_To_BigEndianBytesArray(buf+4, size);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_Write_App_Length(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int size,ret;
	size = BigEndianBytesArray_To_Uint32(buf+4);
	ret = Bootloader_Write_App_Size(size);
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}
void ModBus_Command_Decode_General_Func_Read_HW_ID(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int hwID;
	hwID = Bootloader_GetAppCRC();
	Uint32_To_BigEndianBytesArray(buf+4, hwID);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_Write_HW_ID(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int hwID;
	hwID = Bootloader_GetAppCRC();
	Uint32_To_BigEndianBytesArray(buf+4, hwID);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_App_Checksum(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int crc;
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	crc = Bootloader_GetAppCRC();
	#else
	crc = GetAppCRC_From_Ram();
	#endif
	Uint32_To_BigEndianBytesArray(buf+4, crc);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_Check_Upgrade(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int crc,ret = 0;
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	Bootloader_Write_App_Size(UpgradeInfo.size);
	Bootloader_Write_App_CRC(UpgradeInfo.crc);
	crc = Bootloader_GetAppCRC();
	#else
	crc = GetAppCRC_From_Ram();
	#endif
	if(crc != UpgradeInfo.crc)
	{
		ret = 1;
	}
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}


void ModBus_Command_Decode_General_Func_Read_Stored_CRC(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int crc;
	crc = Bootloader_Read_Stored_CRC();
	Uint32_To_BigEndianBytesArray(buf+4, crc);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_Write_Stored_CRC(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int crc,ret;
	crc = BigEndianBytesArray_To_Uint32(buf+4);
	ret = Bootloader_Write_App_CRC(crc);
	UI_BootLoader_Set_Check_CRC_Info(ret);
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}


void ModBus_Command_Decode_General_Func_Erase_App(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	unsigned int ret;
	ret = Bootloader_EraseApp();
	UI_BootLoader_Set_Erase_Info(ret);
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	#endif
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func_Erase_Flash(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	unsigned int addr,ret;
	addr = BigEndianBytesArray_To_Uint32(buf+4);
	ret = Bootloader_EraseFlash(addr);
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	#endif
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_General_Func(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned short funcode;
	funcode = BigEndianBytesArray_To_Uint16(buf + 2);
	switch (funcode)
	{
		case UPGRADE_OP_CODE_READ_APP_LENGTH:
			ModBus_Command_Decode_General_Func_Read_App_Length(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_WRITE_APP_LENGTH:
			ModBus_Command_Decode_General_Func_Write_App_Length(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_READ_HW_ID:
			ModBus_Command_Decode_General_Func_Read_HW_ID(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_WRITE_HW_ID:
			ModBus_Command_Decode_General_Func_Write_HW_ID(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_APP_CHECKSUM:
			ModBus_Command_Decode_General_Func_App_Checksum(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_CHECK_UPGRADE:
			ModBus_Command_Decode_General_Func_Check_Upgrade(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_READ_STORED_CRC:
			ModBus_Command_Decode_General_Func_Read_Stored_CRC(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_WRITE_STORED_CRC:
			ModBus_Command_Decode_General_Func_Write_Stored_CRC(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_ERASE_APP:
			ModBus_Command_Decode_General_Func_Erase_App(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_ERASE_FLASH:
			ModBus_Command_Decode_General_Func_Erase_Flash(buf,len,Feedback);
			break;
		case UPGRADE_OP_CODE_JUMP_TO_APP:
			Bootloader_Set_Jump_Flag(1);
			break;
		default :
			break;
	}
	Feedback(buf,len);
}

void ModBus_Command_Decode(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned char funcode;
	funcode=buf[1];
	switch (funcode)
	{
		case MODBUS_FUNCODE_UPGRADE_INFO:
			ModBus_Command_Decode_Upgrade_Info(buf,len,Feedback);
			break;
		case MODBUS_FUNCODE_PROGRAM_FLASH:
			ModBus_Command_Decode_Program_Flash(buf,len,Feedback);
			break;
		case MODBUS_FUNCODE_GENERAL_FUNC:
			ModBus_Command_Decode_General_Func(buf,len,Feedback);
			break;
		default :
			break;
	}
}
