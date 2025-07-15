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

/* ==================================================================
 * Flash������Ϣ��GD32G553 512KB Flash - ˫Bankģʽ DBS=1��
 * ÿҳ��С��1KB
 * Bank0��0x08000000 - 0x0803FFFF����256ҳ
 * Bank1��0x08040000 - 0x0807FFFF����256ҳ
 *
 * ҳ���    Page    Begin           End             Size(Dec)    Size(hex)
 * ==================================================================
 * Bank0-000    0     0x08000000     0x080003FF      1024          0x400    <- Boot ��ʼҳ
 * ...         ...     ...           ...              ...           ...
 * Bank0-023    23    0x08005C00     0x08005FFF      1024          0x400    <- Boot ���һҳ
 * Bank0-024    24    0x08006000     0x080063FF      1024          0x400    <- Crypto ��
 * Bank0-025    25    0x08006400     0x080067FF      1024          0x400    <- App ��ʼҳ
 * ...         ...     ...           ...              ...           ...
 * Bank0-255    255   0x0803FC00     0x0803FFFF      1024          0x400
 * ==================================================================
 * Bank1-000    0     0x08040000     0x080403FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank1-255    255   0x0807FC00     0x0807FFFF      1024          0x400    <- App ���һҳ����APPУ�����ݣ�
 * ==================================================================
 *
 * �����滮��
 * 1. Boot  ʹ�� Bank0 ҳ 0~23           (24ҳ = 24KB) -> 0x08000000 ~ 0x08005FFF
 * 2. Crypto ʹ�� Bank0 ҳ 24            (1ҳ = 1KB)   -> 0x08006000 ~ 0x080063FF
 * 3. App   ʹ�� Bank0 ҳ 25~255 + Bank1 ҳ 0~255 (487KB) -> 0x08006400 ~ 0x0807FFFF
 *
 * ==================================================================================
 * ��������    ��ʼ��ַ       ������ַ        ��С       ʹ��˵��
 * ----------------------------------------------------------------------------------
 * Boot ��     0x08000000     0x08005FFF      24KB       Bootloader �̼�����
 *             - ������ʼ�����ж���������ת App��ͨ������Э����߼�
 *
 * Crypto ��   0x08006000     0x080063FF      1KB
 *             - �洢AES������Կ���豸֤�顢��ȫ���õ���������
 *             - Ӳ����������ר�÷������򣬽�ֹ��ͨ�������
 *             - ʹ������д�������Ʒ�ֹδ��Ȩ��ȡ
 *
 * App ��      0x08006400     0x0807FFFF      487KB      Ӧ�ó������̼�
 *             - ʵ���û����롢ҵ���߼�������ʱ��Դ
 *             - ��� 8 �ֽ����ڴ洢APPУ�����ݣ�
 *                 0x0807FFF8��App_Size��4�ֽڣ�
 *                 0x0807FFFC��App_CRC32��4�ֽڣ�
 *
 * ==================================================================================
 * APPУ������λ�ã�
 * - App��Ϣ�洢��App�������8�ֽ� (Bank1ҳ255�����8�ֽ�)
 *   - ��С��ַ: 0x0807FFF8
 *   - CRC��ַ:  0x0807FFFC
 * ==================================================================
 */

/* ================== Flash �������� ================== */
#define BOOT_START_ADDR 0x08000000UL                                      // Boot ����ʼ��ַ
#define BOOT_SIZE (24U * 1024UL)                        // 24KB
#define BOOT_END_ADDR (BOOT_START_ADDR + BOOT_SIZE - 1) // 0x08005FFF

#define INFO_START_ADDR (BOOT_START_ADDR + BOOT_SIZE)   // 0x08006000UL   // Crypto ����ʼ��ַ
#define INFO_SIZE (1U * 1024UL)                         // 1KB
#define INFO_END_ADDR (INFO_START_ADDR + INFO_SIZE - 1) // 0x080063FF

#define APP_START_ADDR (INFO_START_ADDR + INFO_SIZE)    // 0x08006400UL   // App ����ʼ��ַ
#define APP_SIZE (487U * 1024UL)                        // 487KB
#define APP_END_ADDR (APP_START_ADDR + APP_SIZE - 1)    // 0x0807FFFF

/* ================== У��洢λ�� ================== */
#define APP_SIZE_ADDR (APP_END_ADDR - 7)                 // 0x0807FFF8 (˫�ֶ����ַ)
#define APP_CRC_ADDR (APP_END_ADDR - 3)                  // 0x0807FFFC

#define BOOT_SIZE_ADDR (BOOT_END_ADDR - 7)               // 0x08005FF8 (˫�ֶ����ַ)
#define BOOT_CRC_ADDR (BOOT_END_ADDR - 3)                // 0x08005FFC

/* ================== ���б�־���� ================== */
#define FLAG_RUNAPP_FORCE 0x70704166                     // fApp ǿ�ƽ���App
#define FLAG_GO_APP 0x70704167                           // gApp ָ�����App
#define FLAG_RUNAPP 0x00707041                           // App ��������App
#define FLAG_RUNBOOT 0x746F6F42                          // Boot
#define FLAG_CRC_ERROR 0x45435243                        // CRCE

/* ================== SRAM_BASE���� ================== */
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
    extern void Bootloader_Program_Init(void);
    extern int Bootloader_Write_App_Info(uint32_t size, uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */