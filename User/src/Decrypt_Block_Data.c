// #define USE_F407HARDWARE_CRC // 注释则是gd32G553

#include <stdint.h>

#ifdef USE_F407HARDWARE_CRC
#include "crc32_st.h"
#else
#include "gd32g5x3_crc.h"
#endif

#include "Decrypt_Block_Data.h"

#ifdef USE_F407HARDWARE_CRC
#define CRC_DATA (*((volatile uint32_t *)(0x40023000)))
#define CRC_CTL (*((volatile uint32_t *)(0x40023008)))
#endif

unsigned long crc;

void Decryption_Init(unsigned long key)
{
#ifdef USE_F407HARDWARE_CRC
	CRC_ResetDR();
	crc = CRC_CalcBlockCRC(key);
#else
	// crc_deinit();
	crc_data_register_reset();						   // 复位数据寄存器
	crc_init_data_register_write(0xFFFFFFFF);		   // 确保初始值正确
	crc_single_data_calculate(key, INPUT_FORMAT_WORD); // 计算单个32位数据
	crc = crc_data_register_read();					   // 获取结果
#endif
}

/* 解密 */
void Decrypt_Block_Data(unsigned long *pData, unsigned long size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		pData[i] ^= crc;
#ifdef USE_F407HARDWARE_CRC
		crc = CRC_CalcBlockCRC(pData[i]);
#else
		crc_single_data_calculate(pData[i], INPUT_FORMAT_WORD); // 计算单个32位数据
		crc = crc_data_register_read();							// 获取结果
#endif
	}
}
