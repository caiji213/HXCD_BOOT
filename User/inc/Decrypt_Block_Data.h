#ifndef __DECRYPT_BLOCK_DATA_H__
#define __DECRYPT_BLOCK_DATA_H__

extern void Decryption_Init(unsigned long key);
extern void Decrypt_Block_Data(unsigned long * pData, unsigned long size);

#endif
