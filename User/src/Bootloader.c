#include "Bootloader.h"
#include "gd32g5x3_fmc.h"

// 全局变量定义
Memory_Info App_Flash_Info;
Memory_Info Boot_Flash_Info;
Memory_Info Ram_Info;

int Bootloader_Jump_Flag = 0;
Info_t App_Info;
Info_t Boot_Info;

/* ==================================================================
 * Flash分区信息，GD32G553，512KB Flash，双 Bank 模式（DBS=1）
 * 每页大小：1KB
 * Bank0：0x08000000 - 0x0803FFFF，共256页
 * Bank1：0x08040000 - 0x0807FFFF，共256页
 * 
 * 页编号    Page    Begin           End             Size(Dec)    Size(hex)
 * ==================================================================
 * Bank0-000    0     0x08000000     0x080003FF      1024          0x400    <- Boot 起始页
 * Bank0-001    1     0x08000400     0x080007FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank0-047   47     0x0800BC00     0x0800BFFF      1024          0x400    <- Boot 最后一页
 * Bank0-048   48     0x0800C000     0x0800C3FF      1024          0x400    <- Info 起始页
 * ...         ...     ...           ...              ...           ...
 * Bank0-060   60     0x0800F000     0x0800F3FF      1024          0x400    <- Info 最后一页
 * Bank0-061   61     0x0800F400     0x0800F7FF      1024          0x400    <- App 起始页
 * ...         ...     ...           ...              ...           ...
 * Bank0-255   255    0x0803FC00     0x0803FFFF      1024          0x400
 * ==================================================================
 * Bank1-000    0     0x08040000     0x080403FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank1-192   192    0x0807C000     0x0807C3FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank1-252   252    0x0807FC00     0x0807FFFF      1024          0x400    <- App 最后一页
 * Bank1-253   253    0x08080000     0x080803FF      1024          0x400    <- Data 起始页
 * Bank1-254   254    0x08080400     0x080807FF      1024          0x400
 * Bank1-255   255    0x08080800     0x08080BFF      1024          0x400    <- Data 最后一页
 * ==================================================================
 * 
 * 分区规划：
 * 1. Boot 使用 Bank0 页 0~47        (48KB)       -> 0x08000000 ~ 0x0800BFFF
 * 2. Info 使用 Bank0 页 48~60      (13KB)       -> 0x0800C000 ~ 0x0800F3FF
 * 3. App  使用 Bank0 页 61~255 + Bank1 页 0~252 (448KB) -> 0x0800F400 ~ 0x0807FFFF
 * 4. Data 使用 Bank1 页 253~255     (3KB)        -> 0x08080000 ~ 0x08080BFF
 * 
 * 元数据位置：
 * - App信息存储在App区域最后8字节 (Bank1页252的最后8字节)
 *   - 大小地址: 0x0807FFF8 (低4字节)
 *   - CRC地址:  0x0807FFFC (高4字节)
 * ==================================================================
 */

// 自定义常量
#define FLASH_PAGE_SIZE     fmc_page_size_get()  // 动态获取页大小

// 配置Flash等待状态（必须调用）
static void configure_flash_waitstates(void)
{
    // 设置FMC等待状态为7 (216MHz需要)
    fmc_wscnt_set(FMC_WAIT_STATE_7);
}

void Bootloader_Hal_Init(void)
{
    // 配置216MHz等待状态
    configure_flash_waitstates();
    
    // Bootloader Flash信息
    Boot_Flash_Info.page_size = FLASH_PAGE_SIZE;
    Boot_Flash_Info.start_addr = BOOT_START_ADDR;
    Boot_Flash_Info.size = BOOT_SIZE;
    Boot_Flash_Info.end_addr = BOOT_END_ADDR;
    Boot_Flash_Info.page_num = BOOT_SIZE / FLASH_PAGE_SIZE;

    // App Flash信息
    App_Flash_Info.page_size = FLASH_PAGE_SIZE;
    App_Flash_Info.start_addr = APP_START_ADDR;
    App_Flash_Info.size = APP_SIZE;
    App_Flash_Info.end_addr = APP_END_ADDR;
    App_Flash_Info.page_num = APP_SIZE / FLASH_PAGE_SIZE;

    // RAM信息 (保留16字节存储标志)
    Ram_Info.page_size = 4;
    Ram_Info.start_addr = SRAM_BASE;
    Ram_Info.size = 0x10; // 16字节
    Ram_Info.end_addr = SRAM_BASE + 0x10 - 1;
    Ram_Info.page_num = 4;

    // App信息
    App_Info.addr_size = APP_SIZE_ADDR;
    App_Info.addr_crc = APP_CRC_ADDR;
    App_Info.size = *((uint32_t *)(App_Info.addr_size));
    App_Info.crc = *((uint32_t *)(App_Info.addr_crc));
    App_Info.hwId = Bootloader_GethwID();

    // Boot信息
    Boot_Info.addr_size = BOOT_END_ADDR - 7;
    Boot_Info.addr_crc = BOOT_END_ADDR - 3;
    Boot_Info.size = *((uint32_t *)(Boot_Info.addr_size));
    Boot_Info.crc = *((uint32_t *)(Boot_Info.addr_crc));
    Boot_Info.hwId = Bootloader_GethwID();
}

int Bootloader_EraseApp(void)
{
    fmc_state_enum status;
    uint32_t i;
    
    // 解锁Flash
    fmc_unlock();
    
    // Bank0擦除 (页61-255, 195页)
    for(i = 61; i <= 255; i++) {
        status = fmc_page_erase(FMC_BANK0, i);
        if(status != FMC_READY) {
            fmc_lock();
            return 1;
        }
    }
    
    // Bank1擦除 (页0-252, 253页)
    for(i = 0; i <= 252; i++) {
        status = fmc_page_erase(FMC_BANK1, i);
        if(status != FMC_READY) {
            fmc_lock();
            return 1;
        }
    }
    
    // 重新锁定Flash
    fmc_lock();
    return 0;
}

int Bootloader_EraseAllFlash(void)
{
    fmc_state_enum status;
    
    // 解锁Flash
    fmc_unlock();
    
    // Bank0整块擦除
    status = fmc_bank0_erase();
    if(status != FMC_READY) {
        fmc_lock();
        return 1;
    }
    
    // Bank1整块擦除
    status = fmc_bank1_erase();
    fmc_lock();
    
    return (status == FMC_READY) ? 0 : 1;
}

int Bootloader_ProgramBlock(unsigned char * buf, uint32_t address, uint32_t size)
{
    // 确保地址在APP区域
    if(address < APP_START_ADDR || address > APP_END_ADDR)
        return 1;
    
    // 确保大小是8的倍数 (双字对齐)
    if(size % 8 != 0)
        return 2;
    
    uint32_t words = size / 8;
    uint64_t *data_ptr = (uint64_t*)buf;
    fmc_state_enum status;
    
    // 解锁Flash
    fmc_unlock();
    
    for(uint32_t i = 0; i < words; i++) {
        status = fmc_doubleword_program(address + i*8, data_ptr[i]);
        if(status != FMC_READY) {
            fmc_lock();
            return 3;
        }
    }
    
    // 重新锁定Flash
    fmc_lock();
    return 0;
}

int Bootloader_Write_App_CRC(uint32_t crc)
{
    uint64_t combined = ((uint64_t)crc << 32) | App_Info.size;
    fmc_state_enum status;
    
    // 解锁Flash
    fmc_unlock();
    
    status = fmc_doubleword_program(APP_SIZE_ADDR, combined);
    
    // 重新锁定Flash
    fmc_lock();
    
    if(status == FMC_READY) {
        App_Info.crc = crc;
        return 0;
    }
    return 1;
}

int Bootloader_Write_App_Size(uint32_t size)
{
    uint64_t combined = ((uint64_t)App_Info.crc << 32) | size;
    fmc_state_enum status;
    
    // 解锁Flash
    fmc_unlock();
    
    status = fmc_doubleword_program(APP_SIZE_ADDR, combined);
    
    // 重新锁定Flash
    fmc_lock();
    
    if(status == FMC_READY) {
        App_Info.size = size;
        return 0;
    }
    return 1;
}

void Bootloader_RunAPP(void)
{
    const vector_t *vector_p = (vector_t*)App_Flash_Info.start_addr;
    
    __disable_irq();  // 关闭所有中断
    
    // 重置所有外设到默认状态
    rcu_deinit();
    
    // 配置堆栈指针和向量表
    __set_MSP(vector_p->stack_addr);
    SCB->VTOR = App_Flash_Info.start_addr;
    
    // 跳转到应用
    vector_p->func_p();
    
    // 理论上不会执行到这里
    while(1);
}

/* 检查Boot区域完整性*/
int Bootloader_CheckBoot(void)
{
    uint32_t boot_size = Boot_Info.size;
    uint32_t boot_crc = Boot_Info.crc;

    if(boot_size > Boot_Flash_Info.size) 
        return 1;
    
    uint32_t crc = Bootloader_GetBootCRC();
    return (crc != boot_crc);
}

/* 检查APP区域完整性*/
int Bootloader_CheckApp(void)
{
    uint32_t app_size = App_Info.size;
    uint32_t app_crc = App_Info.crc;

    if(app_size > App_Flash_Info.size)
        return 1;
    
    uint32_t crc = Bootloader_GetAppCRC();
    return (crc != app_crc);
}

/* 检查Boot程序的CRC校验结果*/
uint32_t Bootloader_GetBootCRC(void)
{
    uint32_t boot_size = Boot_Info.size;
    
    crc_deinit(); // 复位CRC
    
    if(boot_size > Boot_Flash_Info.size)
        return 0;
    
    return crc_block_data_calculate((void *)Boot_Flash_Info.start_addr, 
                                    boot_size, 
                                    INPUT_FORMAT_WORD);
}

/* 检查APP程序的CRC结果*/
uint32_t Bootloader_GetAppCRC(void)
{
    uint32_t app_size = App_Info.size;
    
    crc_deinit(); // 复位CRC
    
    if(app_size > App_Flash_Info.size)
        return 0;
    
    return crc_block_data_calculate((void *)App_Flash_Info.start_addr, 
                                    app_size, 
                                    INPUT_FORMAT_WORD);
}

uint32_t Bootloader_Read_Stored_CRC(void)
{
    return *((uint32_t *)(App_Info.addr_crc));
}

uint32_t Bootloader_Read_App_Size(void)
{
    return *((uint32_t *)(App_Info.addr_size));
}

int Bootloader_Get_Jump_Flag(void)
{
    return Bootloader_Jump_Flag;
}

void Bootloader_Set_Jump_Flag(int value)
{
    Bootloader_Jump_Flag = value;
}

uint32_t Bootloader_GethwID(void)
{
    // 读取芯片ID (具体寄存器需参考手册)
    return *((uint32_t *)0x1FFFF7E8);
}