#include "string.h"
//#include "User_DMA.h"
#include "bsp.h"
#include "Bootloader.h"
#include "ModBus_CRC.h"
#include "User_modbus.h"
#include "User_Convert_Int_BytesArray.h"
#include "User_modbus_command_decode.h"

#define ModBus_Function_Code_Read_Holding_Registers  0x03 //�����ּĴ���
#define ModBus_Function_Code_Read_Input_Registers    0x04
#define ModBus_Function_Code_Write_Coil              0x05
#define ModBus_Function_Code_Write_Holding_Register  0x06 //д�������ּĴ���
#define ModBus_Function_Code_Write_Holding_Registers 0x10

typedef struct
{
	//һЩ��־
	unsigned char Slave_Received_Data      :1;
	unsigned char Slave_Response_is_Pending:1; //�ӻ��ȴ��ظ�
}ModBus_Flag;

typedef struct 
{
	unsigned char Slave_Addr; 	//modbus�ӻ���ַ

	unsigned char Slave_Send_Buffer[128];  //�ӻ����ͻ�����
	unsigned int Slave_Send_Buffer_Len;  //�ӻ����ͻ���������
		
	unsigned char * Slave_Received_Data;
	unsigned int Slave_Received_Data_Len;
	
	unsigned char cmd_from; //������Դ

	ModBus_Flag flag; //һЩ��־
}ModBus_Var;

ModBus_Var ModBusVar;

void ModBus_Init(unsigned char addr)
{
	memset(&ModBusVar,0,sizeof(ModBusVar));
	ModBusVar.Slave_Addr = addr;
}

void ModBus_ChangeAddr(unsigned char addr)
{
	ModBusVar.Slave_Addr = addr;
}

//����λ����������
//extern void SendBytesInfoProcUART2(unsigned char * pSendInfo, unsigned int nSendCount);
void ModBus_Slave_Response_Data(unsigned char * buf, unsigned int len)
{
	//UART3����һָ֡�DMAģʽ
	//DMAx_Uart2TX_Enable(UART2, DMA1_Stream6, buf, len);
	//DMAx_Uart1TX_Enable(USART1, DMA2_Stream7, buf, len);
	bsp_rs485_dma_send(buf,len);
}
 
/* �ӻ�������ɣ������жϻص����� */
void ModBus_Slave_Send_Finished(void)
{
	
}

//�ӻ��յ����ݣ����д��������жϻص�����
void ModBus_Slave_Received_Data(unsigned char * buf, unsigned int len, unsigned char cmd_from)
{
	ModBusVar.Slave_Received_Data = buf;
	ModBusVar.Slave_Received_Data_Len = len;
	ModBusVar.flag.Slave_Received_Data = 1;	//������յ�����
	ModBusVar.cmd_from = cmd_from; //���������Դ
}

/*!
    \brief      ���������ּĴ���
    \param[in]  pData: �������ݴ�ŵ�ַ
    \param[in]  length: ��������,���ֽ�
    \param[out] none
    \retval     none
*/

//�ӻ���������λ���Խ�, 1ms����1��
void ModBus_Slave_Process(void)
{
	ModBus_Var *p = &ModBusVar;
	if(p->flag.Slave_Received_Data)
	{
		//�յ����ݣ����д���
		if(p->Slave_Addr == (*p->Slave_Received_Data)) //�ж��Ƿ�Ϊ������ַ
		{
			//��ҪУ��һ��Modbus��CRC
			if(ModBus_CRC16_Check(p->Slave_Received_Data,p->Slave_Received_Data_Len) == 0)
			{
				//У��ɹ����ж���Դ
				switch(p->cmd_from)
				{
					case CMD_FROM_USART1:
						ModBus_Command_Decode(p->Slave_Received_Data,p->Slave_Received_Data_Len,ModBus_Slave_Response_Data);
						break;
					case CMD_FROM_USART2:
						ModBus_Command_Decode(p->Slave_Received_Data,p->Slave_Received_Data_Len,bsp_rs232_dma_send);
					    //ModBus_Command_Decode(p->Slave_Received_Data,p->Slave_Received_Data_Len,SendBytesInfoProcUART2);
						break;
					default:
						break;
				}
			}
			else
			{
				//У���������
			}
		}
		//����յ����ݱ�־
		p->flag.Slave_Received_Data = 0;
	}
}



