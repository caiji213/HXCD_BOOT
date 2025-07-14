#include "User_modbus_command_decode.h"
#include "User_Convert_Int_BytesArray.h"
#include "Decrypt_Block_Data.h"
#include "Bootloader.h"
#include "User_modbus.h"
#include <stdio.h>

#define ModBus_FUNCODE_Read_Holding_Registers 0x03 // 读保持寄存器
#define MODBUS_FUNCODE_UPGRADE_INFO 0x41
#define MODBUS_FUNCODE_PROGRAM_FLASH 0x42
#define MODBUS_FUNCODE_GENERAL_FUNC 0x43

#define UPGRADE_OP_CODE_READ_APP_LENGTH 0x0001	 // 读APP长度
#define UPGRADE_OP_CODE_WRITE_APP_LENGTH 0x8001	 // 写APP长度
#define UPGRADE_OP_CODE_READ_HW_ID 0x001D		 // 读硬件识别码
#define UPGRADE_OP_CODE_WRITE_HW_ID 0x801D		 // 写硬件识别码
#define UPGRADE_OP_CODE_APP_CHECKSUM 0x00C0		 // 获取APP校验
#define UPGRADE_OP_CODE_CHECK_UPGRADE 0x80C0	 // 校验升级
#define UPGRADE_OP_CODE_READ_STORED_CRC 0x00C1	 // 读存储的校验结果
#define UPGRADE_OP_CODE_WRITE_STORED_CRC 0x80C1	 // 写存储的校验结果
#define UPGRADE_OP_CODE_ERASE_APP 0x00F0		 // 擦除APP区域的Flash
#define UPGRADE_OP_CODE_ERASE_UPDATE_FLAG 0x00F1 // 擦除升级标记
#define UPGRADE_OP_CODE_ERASE_FLASH 0x00FF		 // 擦除指定区域的Flash
#define UPGRADE_OP_CODE_JUMP_TO_APP 0x474F		 // 跳转到APP
#define UPGRADE_OP_CODE_JUMP_TO_BOOT 0x4F47		 // 跳转到bootloader

extern void Bootloader_RunBootloader(void);

typedef struct
{
	unsigned int hwId;
	unsigned int size;
	unsigned int crc;
} Upgrade_Info_t;

Upgrade_Info_t UpgradeInfo;

void ModBus_Command_Decode_Feedback_JumpBootloader(unsigned char *buf, unsigned char addr, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	int i, len;
	buf[0] = addr;
	buf[1] = MODBUS_FUNCODE_GENERAL_FUNC;
	buf[2] = UPGRADE_OP_CODE_JUMP_TO_BOOT >> 8;
	buf[3] = UPGRADE_OP_CODE_JUMP_TO_BOOT & 0xFF;
	for (i = 4; i < 8; i++)
	{
		buf[i] = 0;
	}
	len = 10;
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

void ModBus_Command_Decode_Feedback_Error(unsigned char err_code, unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	buf[1] |= 0x80; // 标记错误
	buf[2] = err_code;
	len = 5;
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

void ModBus_Command_Decode_Upgrade_Info(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	UpgradeInfo.hwId = BigEndianBytesArray_To_Uint32(buf + 3);
	UpgradeInfo.size = BigEndianBytesArray_To_Uint32(buf + 7);
	UpgradeInfo.crc = BigEndianBytesArray_To_Uint32(buf + 11);
	Decryption_Init(UpgradeInfo.crc);
	Uint32_To_BigEndianBytesArray(buf + 3, UpgradeInfo.hwId);

	// 初始化编程状态
	Bootloader_Program_Init();
	Feedback(buf, len);
}

// 8的倍数版本
// void ModBus_Command_Decode_Program_Flash(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
//{
//     unsigned int addr, size;
//     int ret = 0;
//     addr = BigEndianBytesArray_To_Uint32(buf+2);
//     size = BigEndianBytesArray_To_Uint16(buf+6);
//
//     // 物理地址计算
//     uint32_t physical_addr = APP_START_ADDR + addr;
//     uint32_t padding = 0;
//
//     // 计算对齐填充
//     if (physical_addr % 8 != 0) {
//         padding = 8 - (physical_addr % 8);
//         physical_addr = (physical_addr + 7) & ~0x7; // 8字节对齐
//     }

//    unsigned char* data_start = buf + 8 + padding;
//    unsigned int real_size = size - padding;

//    // 修复多余数据问题：检查实际可处理的数据量
//    unsigned int max_data_available = len - 8 - padding;
//    if (real_size > max_data_available) {
//        real_size = max_data_available;
//    }

//    // 解密数据
//    if (real_size > 0)
//	{
//        // 执行解密操作
//        unsigned int dwords = (real_size + 3) / 4; // 向上取整到4字节倍数
//        Decrypt_Block_Data((unsigned long *)data_start, dwords);
//
//        // 编程Flash
//        ret = Bootloader_ProgramBlock(data_start, physical_addr, real_size);
//    }

//	if (ret)
//	{
//		//烧录出错,反馈烧录了0字节
//		Uint16_To_BigEndianBytesArray(buf+6, 0);
//	}
//    len = 10;
//    ModBus_Fill_CRC16(buf, len);
//    Feedback(buf, len);
//}

void ModBus_Command_Decode_Program_Flash(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int addr_offset, size;
	int ret = 0;

	// 解析地址偏移和数据大小
	addr_offset = BigEndianBytesArray_To_Uint32(buf + 2);
	size = BigEndianBytesArray_To_Uint16(buf + 6);

	// 计算物理地址
	uint32_t physical_addr = APP_START_ADDR + addr_offset;

	// 数据起始位置（跳过命令头）
	unsigned char *data_start = buf + 8;

	// 检查数据大小是否合法
	if (size > (len - 8))
	{
		size = len - 8; // 修正为实际可用大小
	}

	// 检查数据大小是否是4的倍数
	if (size % 4 != 0)
	{
		// 如果大小不是4的倍数，调整为4的倍数
		size = (size + 3) & ~0x03; // 向上对齐到4字节倍数
	}

	// 解密数据（如果需要）
	// 执行解密操作（按4字节倍数）
	unsigned int dwords = size / 4;
	Decrypt_Block_Data((unsigned long *)data_start, dwords);

	// 编程Flash
	ret = Bootloader_ProgramBlock(data_start, physical_addr, size);

	// 处理响应
	if (ret)
	{
		// 烧录出错，反馈烧录了0字节
		Uint16_To_BigEndianBytesArray(buf + 6, 0);
	}
	else
	{
		// 成功烧录，反馈实际烧录的字节数
		Uint16_To_BigEndianBytesArray(buf + 6, size);
	}

	// 填充响应长度并计算CRC
	len = 10;
	ModBus_Fill_CRC16(buf, len);

	// 发送响应
	Feedback(buf, len);
}

// modbus总线读取App程序长度
void ModBus_Command_Decode_General_Func_Read_App_Length(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int size;
	size = Bootloader_Read_App_Size();
	Uint32_To_BigEndianBytesArray(buf + 4, size);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus写入App程序长度
void ModBus_Command_Decode_General_Func_Write_App_Length(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int size, ret;
	size = BigEndianBytesArray_To_Uint32(buf + 4);
	ret = Bootloader_Write_App_Size(size);
	Uint32_To_BigEndianBytesArray(buf + 4, ret);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus读取硬件ID号
void ModBus_Command_Decode_General_Func_Read_HW_ID(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int hwID;
	hwID = Bootloader_GethwID();
	Uint32_To_BigEndianBytesArray(buf + 4, hwID);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus写入硬件ID号
void ModBus_Command_Decode_General_Func_Write_HW_ID(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int hwID;
	// hwID = Bootloader_SethwID();
	Uint32_To_BigEndianBytesArray(buf + 4, hwID);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus读取应用App CRC校验字节
void ModBus_Command_Decode_General_Func_App_Checksum(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int crc;
#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	crc = Bootloader_GetAppCRC();
#else
	crc = GetAppCRC_From_Ram();
#endif
	Uint32_To_BigEndianBytesArray(buf + 4, crc);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus更新升级信息
void ModBus_Command_Decode_General_Func_Check_Upgrade(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int crc, ret = 0;
#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	Bootloader_Write_App_Size(UpgradeInfo.size);
	Bootloader_Write_App_CRC(UpgradeInfo.crc);
	crc = Bootloader_GetAppCRC();
#else
	crc = GetAppCRC_From_Ram();
#endif
	// 暂时不校验
	//	if(crc != UpgradeInfo.crc)
	//	{
	//		ret = 1;
	//	}
	Uint32_To_BigEndianBytesArray(buf + 4, ret);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus读取存储的App CRC数据
void ModBus_Command_Decode_General_Func_Read_Stored_CRC(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int crc;
	// crc = Bootloader_Read_Stored_CRC();
	crc = Bootloader_CheckApp();
	Uint32_To_BigEndianBytesArray(buf + 4, crc);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus写入存储的App CRC数据
void ModBus_Command_Decode_General_Func_Write_Stored_CRC(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned int crc, ret;
	crc = BigEndianBytesArray_To_Uint32(buf + 4);
	ret = Bootloader_Write_App_CRC(crc);
	Uint32_To_BigEndianBytesArray(buf + 4, ret);
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// modbus擦除应用App程序
void ModBus_Command_Decode_General_Func_Erase_App(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	unsigned int ret;
	ret = Bootloader_EraseApp();
	Uint32_To_BigEndianBytesArray(buf + 4, ret);
#endif
	ModBus_Fill_CRC16(buf, len);
	Feedback(buf, len);
}

// 指令解码通用函数
void ModBus_Command_Decode_General_Func(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned short funcode;
	funcode = BigEndianBytesArray_To_Uint16(buf + 2);
	switch (funcode)
	{
	case UPGRADE_OP_CODE_READ_APP_LENGTH:
		ModBus_Command_Decode_General_Func_Read_App_Length(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_WRITE_APP_LENGTH:
		ModBus_Command_Decode_General_Func_Write_App_Length(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_READ_HW_ID:
		ModBus_Command_Decode_General_Func_Read_HW_ID(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_WRITE_HW_ID:
		ModBus_Command_Decode_General_Func_Write_HW_ID(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_APP_CHECKSUM:
		ModBus_Command_Decode_General_Func_App_Checksum(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_CHECK_UPGRADE:
		ModBus_Command_Decode_General_Func_Check_Upgrade(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_READ_STORED_CRC:
		ModBus_Command_Decode_General_Func_Read_Stored_CRC(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_WRITE_STORED_CRC:
		ModBus_Command_Decode_General_Func_Write_Stored_CRC(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_ERASE_APP:
		ModBus_Command_Decode_General_Func_Erase_App(buf, len, Feedback);
		break;
	case UPGRADE_OP_CODE_JUMP_TO_APP:
		Bootloader_Set_Jump_Flag(1);
		break;
	case UPGRADE_OP_CODE_JUMP_TO_BOOT:
		Feedback(buf, len); // 已经在Boot中，直接原数据回复
		break;
	default:
		ModBus_Command_Decode_Feedback_Error(1, buf, len, Feedback);
		break;
	}
}

#define ADDR_DeviceType 2500

extern char str_DeviceInfo[];

/* 解析从机收到的命令：读保持寄存器 */
void ModBus_Command_Decode_Read_Holding_Registers(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned short base_addr, regs_num;
	unsigned char bytes_num;
	int i;

	// 读取命令中的基地址
	base_addr = BigEndianBytesArray_To_Uint16(buf + 2);
	// 解析命令的寄存器数量
	regs_num = BigEndianBytesArray_To_Uint16(buf + 4);

	if ((ADDR_DeviceType == base_addr) && (regs_num <= 64))
	{
		// 这里只实现DeviceType的内容，方便上位机扫描，地址必须是 ADDR_DeviceType

		// 字节数是寄存器数量的两倍
		bytes_num = regs_num * 2;
		// 反馈数据
		buf[2] = bytes_num;
		for (i = 0; i < regs_num; i++)
		{
			buf[3 + i * 2] = str_DeviceInfo[i * 2 + 1];
			buf[3 + i * 2 + 1] = str_DeviceInfo[i * 2];
		}
		len = bytes_num + 5;
		ModBus_Fill_CRC16(buf, len);
		Feedback(buf, len);
	}
	else
	{
		// 反馈错误
		ModBus_Command_Decode_Feedback_Error(02, buf, len, Feedback);
	}
}

void ModBus_Command_Decode(unsigned char *buf, unsigned int len, void (*Feedback)(unsigned char *buf, unsigned int len))
{
	unsigned char funcode;
	funcode = buf[1];
	switch (funcode)
	{
	case ModBus_FUNCODE_Read_Holding_Registers:
		ModBus_Command_Decode_Read_Holding_Registers(buf, len, Feedback);
		break;
	case MODBUS_FUNCODE_UPGRADE_INFO:
		ModBus_Command_Decode_Upgrade_Info(buf, len, Feedback);
		break;
	case MODBUS_FUNCODE_PROGRAM_FLASH:
		ModBus_Command_Decode_Program_Flash(buf, len, Feedback);
		break;
	case MODBUS_FUNCODE_GENERAL_FUNC:
		ModBus_Command_Decode_General_Func(buf, len, Feedback);
		break;
	default:
		// 不支持此功能码
		ModBus_Command_Decode_Feedback_Error(01, buf, len, Feedback);
		break;
	}
}
