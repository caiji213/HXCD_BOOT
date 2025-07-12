/*
 * ModBus_CRC Module
 * Author	:	Peng Jin Biao
 * Date	:	2020-03-26
 */
#include "ModBus_CRC.h"

unsigned short ModBus_CRC16(unsigned char *pData, unsigned short len)
{
	const unsigned short crcTableFast[] =
		{
			0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
			0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};

	unsigned short crc = 0xffff;
	unsigned short i;
	unsigned char ch;

	for (i = 0; i < len; i++)
	{
		ch = *pData++;
		crc = crcTableFast[(ch ^ crc) & 0x0F] ^ (crc >> 4);
		crc = crcTableFast[((ch >> 4) ^ crc) & 0x0F] ^ (crc >> 4);
	}
	return crc;
}

unsigned short ModBus_CRC16_Check(unsigned char *pData, unsigned short len)
{
	unsigned short crc;
	unsigned char bH, bL;
	unsigned char crcH, crcL;
	if (len < 4)
	{
		return 1;
	}
	bH = pData[len - 1];
	bL = pData[len - 2];
	crc = ModBus_CRC16(pData, len - 2);
	crcH = (crc >> 8) & 0xFF;
	crcL = crc & 0xFF;
	if (((crcH == bH) && (crcL == bL)) || ((crcL == bH) && (crcL == bH)))
		return 0;
	else
		return 1;
}

// 对buf填写 ModBus_CRC16，填写位置在buf的len-1和len-2处
void ModBus_Fill_CRC16(unsigned char *buf, unsigned short len)
{
	unsigned short crc;
	if (len < 2)
	{
		return;
	}
	crc = ModBus_CRC16(buf, len - 2);
	buf[len - 1] = (crc >> 8) & 0xFF;
	buf[len - 2] = crc & 0xFF;
}
