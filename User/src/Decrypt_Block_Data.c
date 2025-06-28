#define USE_HARDWARE_CRC
#include <stdint.h>
#ifndef USE_HARDWARE_CRC
#include "crc32_st.h"
#endif
#include "Decrypt_Block_Data.h"

#ifdef USE_HARDWARE_CRC
#define CRC_DATA (*((volatile uint32_t *)(0x40023000)))
#define CRC_CTL  (*((volatile uint32_t *)(0x40023008)))
#endif

unsigned long crc;

void Decryption_Init(unsigned long key)
{
#ifdef USE_HARDWARE_CRC
	CRC_CTL |= 1;
	CRC_DATA = key;
	crc = CRC_DATA;
#else
	CRC_ResetDR();
	crc = CRC_CalcBlockCRC(key);
#endif
}

/* Ω‚√‹ */
void Decrypt_Block_Data(unsigned long * pData, unsigned long size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		pData[i] ^= crc;
		#ifdef USE_HARDWARE_CRC
		CRC_DATA=pData[i];
		crc = CRC_DATA;
		#else
		crc = CRC_CalcBlockCRC(pData[i]);
		#endif
	}
}

