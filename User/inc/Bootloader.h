#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "gd32g5x3.h"
#include "gd32g5x3_fmc.h"

    /* 存储区信息 */
    typedef struct
    {
        uint32_t start_addr;
        uint32_t end_addr;
        uint32_t size; // 字节数
        uint32_t page_size;
        uint32_t page_num;
    } Memory_Info;

    extern Memory_Info App_Flash_Info;
    extern Memory_Info Boot_Flash_Info;
    extern Memory_Info Ram_Info;

    extern int Bootloader_Jump_Flag; // 跳转标记

    /* 应用程序信息 */
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
        uint32_t stack_addr;   // intvec[0] 初始堆栈指针
        application_t *func_p; // intvec[1] 初始程序计数器
    } vector_t;

/* ==================================================================
 * Flash分区信息（GD32G553 512KB Flash - 双Bank模式 DBS=1）
 * 每页大小：1KB
 * Bank0：0x08000000 - 0x0803FFFF，共256页
 * Bank1：0x08040000 - 0x0807FFFF，共256页
 *
 * 页编号    Page    Begin           End             Size(Dec)    Size(hex)
 * ==================================================================
 * Bank0-000    0     0x08000000     0x080003FF      1024          0x400    <- Boot 起始页
 * Bank0-001    1     0x08000400     0x080007FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank0-031    31    0x08007C00     0x08007FFF      1024          0x400    <- Boot 最后一页
 * Bank0-032    32    0x08008000     0x080083FF      1024          0x400    <- Crypto 区
 * Bank0-033    33    0x08008400     0x080087FF      1024          0x400    <- App 起始页
 * ...         ...     ...           ...              ...           ...
 * Bank0-255    255   0x0803FC00     0x0803FFFF      1024          0x400
 * ==================================================================
 * Bank1-000    0     0x08040000     0x080403FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank1-252    252   0x0807F800     0x0807FBFF      1024          0x400
 * Bank1-253    253   0x0807FC00     0x0807FFFF      1024          0x400    <- App 最后一页（含APP校验数据）
 * ==================================================================
 *
 * 分区规划：
 * 1. Boot  使用 Bank0 页 0~31           (32页 = 32KB) -> 0x08000000 ~ 0x08007FFF
 * 2. Crypto 使用 Bank0 页 32            (1页 = 1KB)   -> 0x08008000 ~ 0x080083FF
 * 3. App   使用 Bank0 页 33~255 + Bank1 页 0~253 (479KB) -> 0x08008400 ~ 0x0807FFFF
 *
 * ==================================================================================
 * 区域名称    起始地址       结束地址        大小       使用说明
 * ----------------------------------------------------------------------------------
 * Boot 区     0x08000000     0x08007FFF      32KB       Bootloader 固件本体
 *             - 启动初始化、中断向量、跳转 App、通信升级协议等逻辑
 *
 * Crypto 区   0x08008000     0x080083FF      1KB
 *             - 存储AES加密密钥、设备证书、安全配置等敏感数据
 *             - 硬件加密引擎专用访问区域，禁止普通代码访问
 *             - 使用物理写保护机制防止未授权读取
 *
 * App 区      0x08008400     0x0807FFFF      479KB      应用程序主固件
 *             - 实际用户代码、业务逻辑、运行时资源
 *             - 最后 8 字节用于存储APP校验数据：
 *                 0x0807FFF8：App_Size（4字节）
 *                 0x0807FFFC：App_CRC32（4字节）
 *
 * ==================================================================================
 * APP校验数据位置：
 * - App信息存储在App区域最后8字节 (Bank1页253的最后8字节)
 *   - 大小地址: 0x0807FFF8
 *   - CRC地址:  0x0807FFFC
 * ==================================================================
 */

/* ================== Flash 分区定义 ================== */
#define BOOT_START_ADDR      0x08000000UL
#define BOOT_SIZE           (32U * 1024UL)  // 32KB
#define BOOT_END_ADDR       (BOOT_START_ADDR + BOOT_SIZE - 1)  // 0x08007FFF

#define INFO_START_ADDR     (BOOT_START_ADDR + BOOT_SIZE)      // 0x08008000UL   // Crypto 区起始地址
#define INFO_SIZE           (1U * 1024UL)   // 1KB
#define INFO_END_ADDR       (INFO_START_ADDR + INFO_SIZE - 1)  // 0x080083FF

#define APP_START_ADDR       0x08008400UL//(INFO_START_ADDR + INFO_SIZE)      // 0x08008400UL   // App 区起始地址
#define APP_SIZE            (479U * 1024UL) // 479KB
#define APP_END_ADDR        (APP_START_ADDR + APP_SIZE - 1)    // 0x0807FFFF

/* ================== 校验APP存储位置 ================== */
#define APP_SIZE_ADDR (APP_END_ADDR - 8) // 0x0807FFF8 (双字对齐地址)
#define APP_CRC_ADDR (APP_END_ADDR - 4)  // 0x0807FFFC

/* ================== 运行标志定义 ================== */
#define FLAG_RUNAPP_FORCE 0x70704166 // fApp 强制进入App
#define FLAG_GO_APP 0x70704167       // gApp 指令进入App
#define FLAG_RUNAPP 0x00707041       // App 正常进入App
#define FLAG_RUNBOOT 0x746F6F42      // Boot
#define FLAG_CRC_ERROR 0x45435243    // CRCE
// 定义编程状态结构体
typedef struct {
    uint8_t buffer[8];      // 8字节缓冲区
    uint8_t buffered;       // 缓冲区中已有字节数 (0, 4, 8)
} FlashProgramState;

/* ================== 其他常量定义 ================== */
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
    //extern int Bootloader_ProgramBlock_4Align(uint32_t address, uint8_t *data, uint32_t size);
#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */