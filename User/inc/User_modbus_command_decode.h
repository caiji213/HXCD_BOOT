#ifndef __USER_MODBUS_COMMAND_DECODE_H__
#define __USER_MODBUS_COMMAND_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void ModBus_Command_Decode(unsigned char * buf, unsigned int len, void (*Feedback)(unsigned char * buf, unsigned int len));
void ModBus_Command_Decode_Feedback_JumpBootloader(unsigned char * buf, unsigned char addr, void (*Feedback)(unsigned char * buf, unsigned int len));

#ifdef __cplusplus
}
#endif

#endif

