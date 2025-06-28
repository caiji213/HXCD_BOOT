/*
Module	:	ModBus_Command_Queue
Author	:	Peng Jin Biao
Date	:	2022-3-29

*/
#ifndef __MODBUS_COMMAND_QUEUE_H__
#define __MODBUS_COMMAND_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ModBus_Command_Length 128
#define ModBus_Command_Queue_Size 16

#define ModBus_Command_From_Local 0
#define ModBus_Command_From_Host  1


typedef struct 
{
	unsigned char cmd_from; //命令来源
	unsigned int cmd_len; //命令长度
	unsigned char Command[ModBus_Command_Length];
}ModBus_Command_Type;

#define ModBus_Command_Queue_FAILED 0
#define ModBus_Command_Queue_SUCCESS 1

unsigned int ModBus_Command_Queue_Init(void);
unsigned int ModBus_Command_Queue_Put(const ModBus_Command_Type * cmd);
unsigned int ModBus_Command_Queue_Get(ModBus_Command_Type *cmd);
unsigned int ModBus_Command_Queue_Is_Empty(void);
unsigned int ModBus_Command_Queue_Count(void);

#ifdef __cplusplus
}
#endif

#endif
