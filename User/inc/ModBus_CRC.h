/*
* ModBus_CRC Header
* Author	:	Peng Jin Biao
* Date	:	2020-03-26
*/
#ifndef	__MODBUS_CRC_H__
#define	__MODBUS_CRC_H__
#ifdef __cplusplus
extern "C" {
#endif

extern unsigned short ModBus_CRC16(unsigned char *pData, unsigned short len);
extern unsigned short ModBus_CRC16_Check(unsigned char *pData, unsigned short len);
extern void ModBus_Fill_CRC16(unsigned char *buf, unsigned short len);


#ifdef __cplusplus
}
#endif
#endif
