#include "User_modbus_command_decode.h"
#include "User_Convert_Int_BytesArray.h"
#include "Decrypt_Block_Data.h"
#include "Bootloader.h"
#include "User_modbus.h"

#define ModBus_FUNCODE_Read_Holding_Registers  0x03 //�����ּĴ���
#define MODBUS_FUNCODE_UPGRADE_INFO  0x41
#define MODBUS_FUNCODE_PROGRAM_FLASH 0x42
#define MODBUS_FUNCODE_GENERAL_FUNC  0x43

#define UPGRADE_OP_CODE_READ_APP_LENGTH   0x0001 //��APP����
#define UPGRADE_OP_CODE_WRITE_APP_LENGTH  0x8001 //дAPP����
#define UPGRADE_OP_CODE_READ_HW_ID        0x001D //��Ӳ��ʶ����
#define UPGRADE_OP_CODE_WRITE_HW_ID       0x801D //дӲ��ʶ����
#define UPGRADE_OP_CODE_APP_CHECKSUM      0x00C0 //��ȡAPPУ��
#define UPGRADE_OP_CODE_CHECK_UPGRADE     0x80C0 //У������
#define UPGRADE_OP_CODE_READ_STORED_CRC   0x00C1 //���洢��У����
#define UPGRADE_OP_CODE_WRITE_STORED_CRC  0x80C1 //д�洢��У����
#define UPGRADE_OP_CODE_ERASE_APP         0x00F0 //����APP�����Flash
#define UPGRADE_OP_CODE_ERASE_UPDATE_FLAG 0x00F1 //�����������
#define UPGRADE_OP_CODE_ERASE_FLASH       0x00FF //����ָ�������Flash
#define UPGRADE_OP_CODE_JUMP_TO_APP       0x474F //��ת��APP
#define UPGRADE_OP_CODE_JUMP_TO_BOOT      0x4F47 //��ת��bootloader

extern void Bootloader_RunBootloader(void);

typedef struct
{
	unsigned int hwId;
	unsigned int size;
	unsigned int crc;
}Upgrade_Info_t;

Upgrade_Info_t UpgradeInfo;

void ModBus_Command_Decode_Feedback_JumpBootloader(unsigned char * buf, unsigned char addr, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	int i,len;
	buf[0]=addr;
	buf[1]=MODBUS_FUNCODE_GENERAL_FUNC;
	buf[2]=UPGRADE_OP_CODE_JUMP_TO_BOOT >> 8;
	buf[3]=UPGRADE_OP_CODE_JUMP_TO_BOOT & 0xFF;
	for(i=4;i<8;i++)
	{
		buf[i]=0;
	}
	len = 10;
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_Feedback_Error(unsigned char err_code, unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	buf[1]|=0x80; //��Ǵ���
	buf[2]=err_code;
	len=5;
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

void ModBus_Command_Decode_Upgrade_Info(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	UpgradeInfo.hwId = BigEndianBytesArray_To_Uint32(buf+3);
	UpgradeInfo.size = BigEndianBytesArray_To_Uint32(buf+7);
	UpgradeInfo.crc = BigEndianBytesArray_To_Uint32(buf+11);
	Decryption_Init(UpgradeInfo.crc);
	Uint32_To_BigEndianBytesArray(buf+3,UpgradeInfo.hwId);
	Feedback(buf,len);
}

//���Flash
void ModBus_Command_Decode_Program_Flash(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int addr,size;
	int ret=0;
	addr = BigEndianBytesArray_To_Uint32(buf+2);
	size = BigEndianBytesArray_To_Uint16(buf+6);

	//��������
	Decrypt_Block_Data((unsigned long *)(buf + 8),size/4);

	//��ʼ��¼
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	ret = Bootloader_ProgramBlock(buf+8,addr,size);
	#else
	ret = ProgramBlock_to_Ram(buf+8,addr,size);
	#endif
	if (ret)
	{
		//��¼����,������¼��0�ֽ�
		Uint16_To_BigEndianBytesArray(buf+6, 0);
	}

	len = 10;
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}


//modbus���߶�ȡApp���򳤶�
void ModBus_Command_Decode_General_Func_Read_App_Length(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int size;
	size = Bootloader_Read_App_Size();
	Uint32_To_BigEndianBytesArray(buf+4, size);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

//modbusд��App���򳤶�
void ModBus_Command_Decode_General_Func_Write_App_Length(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int size,ret;
	size = BigEndianBytesArray_To_Uint32(buf+4);
	ret = Bootloader_Write_App_Size(size);
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

//modbus��ȡӲ��ID��
void ModBus_Command_Decode_General_Func_Read_HW_ID(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int hwID;
	hwID = Bootloader_GethwID();
	Uint32_To_BigEndianBytesArray(buf+4, hwID);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}
//modbusд��Ӳ��ID��
void ModBus_Command_Decode_General_Func_Write_HW_ID(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int hwID;
	//hwID = Bootloader_SethwID();
	Uint32_To_BigEndianBytesArray(buf+4, hwID);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

//modbus��ȡӦ��App CRCУ���ֽ�
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

//modbus����������Ϣ
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

//modbus��ȡ�洢��App CRC����
void ModBus_Command_Decode_General_Func_Read_Stored_CRC(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int crc;
	//crc = Bootloader_Read_Stored_CRC();
	crc = Bootloader_CheckApp();
	Uint32_To_BigEndianBytesArray(buf+4, crc);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

//modbusд��洢��App CRC����
void ModBus_Command_Decode_General_Func_Write_Stored_CRC(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned int crc,ret;
	crc = BigEndianBytesArray_To_Uint32(buf+4);
	ret = Bootloader_Write_App_CRC(crc);
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}

//modbus����Ӧ��App����
void ModBus_Command_Decode_General_Func_Erase_App(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	#ifndef DEBUG_COMMAND_DECODE_USE_EXTERNAL_RAM
	unsigned int ret;
	ret = Bootloader_EraseApp();
	Uint32_To_BigEndianBytesArray(buf+4, ret);
	#endif
	ModBus_Fill_CRC16(buf,len);
	Feedback(buf,len);
}


//ָ�����ͨ�ú���
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
		case UPGRADE_OP_CODE_JUMP_TO_APP:
			Bootloader_Set_Jump_Flag(1);
			break;
		case UPGRADE_OP_CODE_JUMP_TO_BOOT:
			Feedback(buf,len); //�Ѿ���Boot�У�ֱ��ԭ���ݻظ�
			break;
		default :
			ModBus_Command_Decode_Feedback_Error(1, buf, len, Feedback);
			break;
	}
}

#define ADDR_DeviceType 2500

extern char str_DeviceInfo[];

/* �����ӻ��յ�����������ּĴ��� */
void ModBus_Command_Decode_Read_Holding_Registers(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned short base_addr, regs_num;
	unsigned char bytes_num;
	int i;

	//��ȡ�����еĻ���ַ
	base_addr = BigEndianBytesArray_To_Uint16(buf + 2);
	//��������ļĴ�������
	regs_num = BigEndianBytesArray_To_Uint16(buf + 4);

	if((ADDR_DeviceType == base_addr) && (regs_num <= 64))
	{
		//����ֻʵ��DeviceType�����ݣ�������λ��ɨ�裬��ַ������ ADDR_DeviceType

		//�ֽ����ǼĴ�������������
		bytes_num = regs_num * 2;
		//��������
		buf[2]=bytes_num;
		for(i=0;i<regs_num;i++)
		{
			buf[3+i*2]=str_DeviceInfo[i*2+1];
			buf[3+i*2+1]=str_DeviceInfo[i*2];
		}
		len = bytes_num + 5;
		ModBus_Fill_CRC16(buf,len);
		Feedback(buf,len);
	}
	else
	{
		//��������
		ModBus_Command_Decode_Feedback_Error(02,buf,len,Feedback);
	}
}

void ModBus_Command_Decode(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len))
{
	unsigned char funcode;
	funcode=buf[1];
	switch (funcode)
	{
		case ModBus_FUNCODE_Read_Holding_Registers:
			ModBus_Command_Decode_Read_Holding_Registers(buf,len,Feedback);
			break;
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
			//��֧�ִ˹�����
			ModBus_Command_Decode_Feedback_Error(01,buf,len,Feedback);
			break;
	}
}


