/*
 * Module	:	Bootloader_Check_Force
 * Author	:	Peng Jin Biao
 * Date	:	2023-10-20
 */
#include "systick.h"
#include "Bootloader_Check_Force.h"
#include "bsp.h"

static const unsigned char data_to_send[] = {0xAA, 0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
#define data_to_send_length  (sizeof(data_to_send))  // 自动计算数组长度

// 比较内存
int MemVerify(const unsigned char *data1, const unsigned char *data2, int len)
{
	for (int i = 0; i < len; i++) {
		if (data1[i] != data2[i]) {
			return 0;
		}
	}
	return 1;
}

// 检查是否强制进入Bootloader
int Bootloader_Check_Force(void)
{
	// 延迟确保串口稳定
	delay_1ms(500);
	
	// 发送固定测试数据
	bsp_rs232_dma_send((unsigned char *)data_to_send, data_to_send_length);
	
	// 等待数据传输完成
	delay_1ms(500);
	
	// 验证回环数据
	return MemVerify(data_to_send, rs232_rxbuffer, data_to_send_length);
}
