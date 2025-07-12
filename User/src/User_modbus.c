#include "string.h"
#include "bsp.h"
#include "Bootloader.h"
#include "ModBus_CRC.h"
#include "User_modbus.h"
#include "User_Convert_Int_BytesArray.h"
#include "User_modbus_command_decode.h"

#define ModBus_Function_Code_Read_Holding_Registers 0x03 // 读保持寄存器
#define ModBus_Function_Code_Read_Input_Registers 0x04
#define ModBus_Function_Code_Write_Coil 0x05
#define ModBus_Function_Code_Write_Holding_Register 0x06 // 写单个保持寄存器
#define ModBus_Function_Code_Write_Holding_Registers 0x10

typedef struct
{
	// 一些标志
	unsigned char Slave_Received_Data : 1;
	unsigned char Slave_Response_is_Pending : 1; // 从机等待回复
} ModBus_Flag;

typedef struct
{
	unsigned char Slave_Addr; // modbus从机地址

	unsigned char Slave_Send_Buffer[128]; // 从机发送缓冲区
	unsigned int Slave_Send_Buffer_Len;	  // 从机发送缓冲区长度

	unsigned char *Slave_Received_Data;
	unsigned int Slave_Received_Data_Len;

	unsigned char cmd_from; // 命令来源

	ModBus_Flag flag; // 一些标志
} ModBus_Var;

ModBus_Var ModBusVar;

void ModBus_Init(unsigned char addr)
{
	memset(&ModBusVar, 0, sizeof(ModBusVar));
	ModBusVar.Slave_Addr = addr;
}

void ModBus_ChangeAddr(unsigned char addr)
{
	ModBusVar.Slave_Addr = addr;
}

// 向上位机发送命令
// extern void SendBytesInfoProcUART2(unsigned char * pSendInfo, unsigned int nSendCount);
void ModBus_Slave_Response_Data(unsigned char *buf, unsigned int len)
{
	// rs485发送一帧指令，DMA模式
	//	bsp_rs485_dma_send(buf,len);
	//	bsp_rs232_dma_send(buf,len);
}

/* 从机发送完成，处于中断回调函数 */
void ModBus_Slave_Send_Finished(void)
{
}

// 从机收到数据，进行处理，处于中断回调函数
void ModBus_Slave_Received_Data(unsigned char *buf, unsigned int len, unsigned char cmd_from)
{
	ModBusVar.Slave_Received_Data = buf;
	ModBusVar.Slave_Received_Data_Len = len;
	ModBusVar.flag.Slave_Received_Data = 1; // 标记已收到数据
	ModBusVar.cmd_from = cmd_from;			// 标记命令来源
}

/*!
	\brief      反馈读保持寄存器
	\param[in]  pData: 参数数据存放地址
	\param[in]  length: 参数长度,按字节
	\param[out] none
	\retval     none
*/

// 从机处理，与上位机对接, 1ms处理1次
void ModBus_Slave_Process(void)
{
	ModBus_Var *p = &ModBusVar;
	if (p->flag.Slave_Received_Data)
	{
		// 收到数据，进行处理
		if (p->Slave_Addr == (*p->Slave_Received_Data)) // 判断是否为本机地址
		{
			// 需要校验一下Modbus的CRC
			if (ModBus_CRC16_Check(p->Slave_Received_Data, p->Slave_Received_Data_Len) == 0)
			{
				// 校验成功，判断来源
				switch (p->cmd_from)
				{
				case CMD_FROM_RS485:
					ModBus_Command_Decode(p->Slave_Received_Data, p->Slave_Received_Data_Len, bsp_rs485_dma_send);
					break;
				case CMD_FROM_RS232:
					ModBus_Command_Decode(p->Slave_Received_Data, p->Slave_Received_Data_Len, bsp_rs232_dma_send);
					break;
				default:
					break;
				}
			}
			else
			{
				// 校验出错的情况
			}
		}
		// 清除收到数据标志
		p->flag.Slave_Received_Data = 0;
	}
}
