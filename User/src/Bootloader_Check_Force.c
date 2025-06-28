/*
* Module	:	Bootloader_Check_Force
* Author	:	Peng Jin Biao
* Date	:	2023-10-20
*/
//#include "User_DMA.h"
#include "systick.h"
#include "Bootloader_Check_Force.h"
#include "bsp.h"
#define data_to_send_length 12
unsigned char * data_to_send = (unsigned char *)0x1FFF7A10UL;

//�Ƚ��ڴ�
int MemVerify(unsigned char * data1, unsigned char * data2, int len)
{
	int i;
	for (i=0; i<len; i++)
	{
		if ((*data1)!=(*data2))
		{
			return 0;
		}
	}
	return 1;
}

//����Ƿ�ǿ�ƽ���Bootloader
int Bootloader_Check_Force(void)
{
	int ret = 0;
	//��鴮�ڵ�TXD��RXD�Ƿ��·
	//���ȷ���һ���ַ���
	delay_1ms(500);
	//DMAx_Uart1TX_Enable(USART1, DMA2_Stream7, data_to_send, data_to_send_length);
	bsp_rs485_dma_send(data_to_send, data_to_send_length);
	//��ʱ�ȴ�
	delay_1ms(500);
	//����Ƿ���յ�
	if(MemVerify(data_to_send,rs485_rxbuffer,data_to_send_length))
	{
		ret = 1;
	}
	return ret;
}
