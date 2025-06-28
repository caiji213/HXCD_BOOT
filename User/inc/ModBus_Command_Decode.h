/*
Module	:	ModBus_Command_Decode
Author	:	Peng Jin Biao
Date	:	2022-04-11

*/
#ifndef __MODBUS_COMMAND_DECODE_H__
#define __MODBUS_COMMAND_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void ModBus_Command_Decode(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len));

#ifdef __cplusplus
}
#endif

#endif
