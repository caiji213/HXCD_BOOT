#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "gd32g5x3.h"
#include "gd32g5x3_fmc.h"

    /* �洢����Ϣ */
    typedef struct
    {
        uint32_t start_addr;
        uint32_t end_addr;
        uint32_t size; // �ֽ���
        uint32_t page_size;
        uint32_t page_num;
    } Memory_Info;

    extern Memory_Info App_Flash_Info;
    extern Memory_Info Boot_Flash_Info;
    extern Memory_Info Ram_Info;

    extern int Bootloader_Jump_Flag; // ��ת���

    /* Ӧ�ó�����Ϣ */
    typedef struct
    {
        uint32_t hwId;
        uint32_t size;
        uint32_t crc;
        uint32_t addr_size;
        uint32_t addr_crc;
    } Info_t;

    extern Info_t App_Info;
    extern Info_t Boot_Info;

    typedef void(application_t)(void);

    typedef struct vector
    {
        uint32_t stack_addr;   // intvec[0] ��ʼ��ջָ��
        application_t *func_p; // intvec[1] ��ʼ���������
    } vector_t;

/* ================== Flash �������� ================== */
#define BOOT_START_ADDR      0x08000000UL
#define BOOT_SIZE           (16U * 1024UL)  // 16KB
#define BOOT_END_ADDR       (BOOT_START_ADDR + BOOT_SIZE - 1)  // 0x08003FFF

#define INFO_START_ADDR     (BOOT_START_ADDR + BOOT_SIZE)      // 0x08004000UL   // Crypto ����ʼ��ַ
#define INFO_SIZE           (1U * 1024UL)   // 1KB
#define INFO_END_ADDR       (INFO_START_ADDR + INFO_SIZE - 1)  // 0x080043FF

#define APP_START_ADDR      (INFO_START_ADDR + INFO_SIZE)      // 0x08004400UL   // App ����ʼ��ַ
#define APP_SIZE            (495U * 1024UL) // 495KB
#define APP_END_ADDR        (APP_START_ADDR + APP_SIZE - 1)    // 0x0807FFFF

/* ================== У��APP�洢λ�� ================== */
#define APP_SIZE_ADDR (APP_END_ADDR - 8) // 0x0807FFF8 (˫�ֶ����ַ)
#define APP_CRC_ADDR (APP_END_ADDR - 4)  // 0x0807FFFC

/* ================== ���б�־���� ================== */
#define FLAG_RUNAPP_FORCE 0x70704166 // fApp ǿ�ƽ���App
#define FLAG_GO_APP 0x70704167       // gApp ָ�����App
#define FLAG_RUNAPP 0x00707041       // App ��������App
#define FLAG_RUNBOOT 0x746F6F42      // Boot
#define FLAG_CRC_ERROR 0x45435243    // CRCE

/* ================== ������������ ================== */
#define Sram_UpdateFlag_Vector (SRAM_BASE)

    extern void Bootloader_Hal_Init(void);
    extern int Bootloader_CheckBoot(void);
    extern uint32_t Bootloader_GetBootCRC(void);
    extern int Bootloader_CheckApp(void);
    extern uint32_t Bootloader_GetAppCRC(void);
    extern int Bootloader_EraseApp(void);
    extern int Bootloader_ProgramBlock(uint8_t *buf, uint32_t address, uint32_t size);
    extern uint32_t Bootloader_Read_Stored_CRC(void);
    extern int Bootloader_Write_App_CRC(uint32_t crc);
    extern uint32_t Bootloader_Read_App_Size(void);
    extern int Bootloader_Write_App_Size(uint32_t size);
    extern int Bootloader_Get_Jump_Flag(void);
    extern void Bootloader_Set_Jump_Flag(int value);
    extern int Bootloader_EraseAllFlash(void);
    extern uint32_t Bootloader_GethwID(void);
    extern void Bootloader_RunAPP(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */