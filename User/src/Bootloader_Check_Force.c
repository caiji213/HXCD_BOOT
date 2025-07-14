/*
 * Module	:	Bootloader_Check_Force
 * Author	:	Peng Jin Biao
 * Date	:	2023-10-20
 */
#include "systick.h"
#include "Bootloader_Check_Force.h"
#include "bsp.h"

static const unsigned char data_to_send[] = {0xAA, 0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
#define data_to_send_length  (sizeof(data_to_send))  // �Զ��������鳤��

// �Ƚ��ڴ�
int MemVerify(const unsigned char *data1, const unsigned char *data2, int len)
{
	for (int i = 0; i < len; i++) {
		if (data1[i] != data2[i]) {
			return 0;
		}
	}
	return 1;
}

// ����Ƿ�ǿ�ƽ���Bootloader
int Bootloader_Check_Force(void)
{
	// �ӳ�ȷ�������ȶ�
	delay_1ms(500);
	
	// ���͹̶���������
	bsp_rs232_dma_send((unsigned char *)data_to_send, data_to_send_length);
	
	// �ȴ����ݴ������
	delay_1ms(500);
	
	// ��֤�ػ�����
	return MemVerify(data_to_send, rs232_rxbuffer, data_to_send_length);
}
