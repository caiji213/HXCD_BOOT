#ifndef _USER_MODBUS_H
#define _USER_MODBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#define CMD_FROM_RS485 0
#define CMD_FROM_RS232 1
#define CMD_FROM_USART3 2

extern void ModBus_Fill_CRC16(unsigned char *buf, unsigned short len);

extern void ModBus_Init(unsigned char addr);
extern void ModBus_ChangeAddr(unsigned char addr);
extern void ModBus_Slave_Process(void);
extern void ModBus_Slave_Received_Data(unsigned char * buf, unsigned int len, unsigned char cmd_from);
extern void ModBus_Slave_Send_Finished(void);

extern void SendBytesInfoProcUART2(unsigned char * pSendInfo, unsigned int nSendCount);

#ifdef __cplusplus
}
#endif

#endif
