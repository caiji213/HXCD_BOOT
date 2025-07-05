#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#ifdef __cplusplus
extern "C" {
#endif


/* 存储区信息 */
typedef struct
{
	unsigned long start_addr;
	unsigned long end_addr;
	unsigned long size; //字节数
	unsigned long page_size;
	unsigned long page_num;
}Memory_Info;

extern Memory_Info App_Flash_Info;
extern Memory_Info Boot_Flash_Info;
extern Memory_Info Ram_Info;

extern int Bootloader_Jump_Flag; //跳转标记


/* 应用程序信息 */
typedef struct
{
	unsigned long hwId;
	unsigned long size;
	unsigned long crc;
	unsigned long addr_size;
	unsigned long addr_crc;
}Info_t;

extern Info_t App_Info;
extern Info_t Boot_Info;

typedef void (application_t) (void);

typedef struct vector
{
    unsigned long        stack_addr;     // intvec[0] is initial Stack Pointer
    application_t   *func_p;        // intvec[1] is initial Program Counter
} vector_t;

/* ================== Flash 分区定义 ================== */
#define BOOT_START_ADDR       0x08000000UL
#define BOOT_SIZE             (48U * 1024UL)   // 48KB
#define BOOT_END_ADDR         (BOOT_START_ADDR + BOOT_SIZE - 1UL)

#define INFO_START_ADDR       0x0800C000UL
#define INFO_SIZE             (13U * 1024UL)   // 13KB
#define INFO_END_ADDR         (INFO_START_ADDR + INFO_SIZE - 1UL)

#define APP_START_ADDR        0x0800F400UL
#define APP_SIZE              (448U * 1024UL)  // 448KB
#define APP_END_ADDR          (APP_START_ADDR + APP_SIZE - 1UL)

#define DATA_START_ADDR       0x08080000UL
#define DATA_SIZE             (3U * 1024UL)    // 3KB
#define DATA_END_ADDR         (DATA_START_ADDR + DATA_SIZE - 1UL)

/* ================== 元数据存储位置 ================== */
#define APP_SIZE_ADDR         (APP_END_ADDR - 7UL)  // 0x0807FFF9
#define APP_CRC_ADDR          (APP_END_ADDR - 3UL)  // 0x0807FFFD

/* ================== FMC 标志定义 ================== */
#define FMC_FLAG_END          FMC_STAT_END
#define FMC_FLAG_WPERR        FMC_STAT_WPERR
#define FMC_FLAG_PGERR        FMC_STAT_PGERR

/* ================== 运行标志定义 ================== */
#define FLAG_RUNAPP_FORCE					0x70704166 //fApp 强制进入App
#define FLAG_GO_APP							0x70704167 //gApp 指令进入App
#define FLAG_RUNAPP							0x00707041 //App 正常进入App
#define FLAG_RUNBOOT						0x746F6F42 //Boot
#define FLAG_CRC_ERROR						0x45435243 //CRCE

/* ================== 其他常量定义 ================== */
#define App_Offset            0x10000UL
#define Boot_Vector      0x08000000UL
#define App_Vector       (Boot_Vector + App_Offset)
#define Sram_Vector      0x20000000UL
#define Sram_UpdateFlag_Vector 0x20000000UL


#define CFG_BOOTLOADER

extern void Bootloader_Hal_Init(void);
extern int Bootloader_CheckBoot(void);
extern unsigned long Bootloader_GetBootCRC(void);
extern int Bootloader_CheckApp(void);
extern unsigned long Bootloader_GetAppCRC(void);
extern int Bootloader_EraseApp(void);
extern int Bootloader_ProgramBlock(unsigned char * buf, unsigned long address, unsigned int size);
extern unsigned long Bootloader_Read_Stored_CRC(void);
extern int Bootloader_Write_App_CRC(unsigned long crc);
extern unsigned long Bootloader_Read_App_Size(void);
extern int Bootloader_Write_App_Size(unsigned long size);
extern int Bootloader_Get_Jump_Flag(void);
extern void Bootloader_Set_Jump_Flag(int value);
extern int Bootloader_EraseAllFlash(void);
extern unsigned int Bootloader_GethwID(void);
extern void Bootloader_RunAPP(void);

#ifdef __cplusplus
}
#endif

#endif

