#include "bsp.h"
#include "Bootloader.h"
#include "gd32g5x3_fmc.h"
#include "gd32g5x3.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// 全局变量定义
Memory_Info App_Flash_Info;
Memory_Info Boot_Flash_Info;
Memory_Info Ram_Info;

int Bootloader_Jump_Flag = 0;
Info_t App_Info;
Info_t Boot_Info;

// 自定义常量
#define FLASH_PAGE_SIZE fmc_page_size_get() // 动态获取页大小

// 配置Flash等待状态
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

    // RAM信息
    Ram_Info.page_size = 4;
    Ram_Info.start_addr = Sram_UpdateFlag_Vector;
    Ram_Info.size = 0x10; // 16字节
    Ram_Info.end_addr = SRAM_BASE + 0x10 - 1;
    Ram_Info.page_num = 4;

    // App信息
    App_Info.addr_size = APP_SIZE_ADDR;
    App_Info.addr_crc = APP_CRC_ADDR;
    App_Info.size = *((uint32_t *)(App_Info.addr_size));
    // App_Info.size = 22288;//测试使用
    App_Info.crc = *((uint32_t *)(App_Info.addr_crc));
    App_Info.hwId = Bootloader_GethwID();

    // Boot信息
    Boot_Info.addr_size = BOOT_SIZE_ADDR;
    Boot_Info.addr_crc = BOOT_CRC_ADDR;
    Boot_Info.size = *((uint32_t *)(Boot_Info.addr_size));
    Boot_Info.crc = *((uint32_t *)(Boot_Info.addr_crc));
    Boot_Info.hwId = Bootloader_GethwID();
}

/* 检查Boot区域完整性*/
int Bootloader_CheckBoot(void)
{
    uint32_t boot_size = Boot_Info.size;
    uint32_t boot_crc = Boot_Info.crc;

    if (boot_size > Boot_Flash_Info.size)
        return 1;

    uint32_t crc = Bootloader_GetBootCRC();
    return (crc != boot_crc);
}

/* 检查Boot程序的CRC校验结果*/
uint32_t Bootloader_GetBootCRC(void)
{
    uint32_t boot_size = Boot_Info.size;

    crc_data_register_reset(); // 复位数据寄存器

    if (boot_size > Boot_Flash_Info.size)
        return 0;

    return crc_block_data_calculate((void *)Boot_Flash_Info.start_addr,
                                    boot_size / sizeof(uint32_t),
                                    INPUT_FORMAT_WORD);
}

/* 检查APP区域完整性*/
int Bootloader_CheckApp(void)
{
    uint32_t app_size = App_Info.size;
    uint32_t app_crc = App_Info.crc;

    if (app_size > App_Flash_Info.size)
        return 1;

    uint32_t crc = Bootloader_GetAppCRC();
    return (crc != app_crc);
}

/* 检查APP程序的CRC结果*/
uint32_t Bootloader_GetAppCRC(void)
{
    uint32_t app_size = App_Info.size;

    crc_data_register_reset(); // 复位数据寄存器

    if (app_size > App_Flash_Info.size)
        return 0;

    return crc_block_data_calculate((void *)App_Flash_Info.start_addr,
                                    app_size / sizeof(uint32_t),
                                    INPUT_FORMAT_WORD);
}

/*
 * 擦除APP区域
 */
int Bootloader_EraseApp(void)
{
    fmc_state_enum status;
    uint32_t start_page, end_page;
    uint32_t page_size = App_Flash_Info.page_size;

    // 计算起始页（Bank0部分）
    uint32_t start_addr = App_Flash_Info.start_addr;
    uint32_t bank0_end_addr = 0x0803FFFFUL; // Bank0结束地址
    __disable_irq();                        // 关中断
    // Bank0部分的页范围
    if (start_addr <= bank0_end_addr)
    {
        // Bank0部分需要擦除的页
        start_page = (start_addr - BOOT_START_ADDR) / page_size;
        end_page = 255; // Bank0的最大页号

        __disable_irq(); // 关中断
        // 解锁Flash
        fmc_unlock();

        // 擦除Bank0的App页
        for (uint32_t i = start_page; i <= end_page; i++)
        {
            status = fmc_page_erase(FMC_BANK0, i);
            if (status != FMC_READY)
            {
                fmc_lock();
                __enable_irq(); // 开中断
                return 1;
            }
        }

        // 锁定Flash（Bank1擦除时会再次解锁）
        fmc_lock();
        __enable_irq(); // 开中断
    }

    // Bank1部分的页范围（0到App结束地址所在的页）
    uint32_t bank1_start_addr = 0x08040000; // Bank1起始地址

    if (App_Flash_Info.end_addr >= bank1_start_addr)
    {
        // 计算Bank1部分的页范围
        start_page = 0; // Bank1起始页
        end_page = (App_Flash_Info.end_addr - bank1_start_addr) / page_size;

        __disable_irq(); // 关中断
        // 解锁Flash
        fmc_unlock();

        // 擦除Bank1的App页
        for (uint32_t i = start_page; i <= end_page; i++)
        {
            status = fmc_page_erase(FMC_BANK1, i);
            if (status != FMC_READY)
            {
                fmc_lock();
                __enable_irq(); // 开中断
                return 1;
            }
        }

        // 重新锁定Flash
        fmc_lock();
        __enable_irq(); // 开中断
    }

    return 0;
}

/*
 * 擦除全部flash区域
 */
int Bootloader_EraseAllFlash(void)
{
    fmc_state_enum status;

    __disable_irq(); // 关中断
    // 解锁Flash
    fmc_unlock();

    // Bank0整块擦除
    status = fmc_bank0_erase();
    if (status != FMC_READY)
    {
        fmc_lock();
        __enable_irq(); // 开中断
        return 1;
    }

    // Bank1整块擦除
    status = fmc_bank1_erase();
    fmc_lock();
    __enable_irq(); // 开中断
    return (status == FMC_READY) ? 0 : 1;
}

///*
//* 双字节烧录
//*/
int Bootloader_ProgramBlock(unsigned char *buf, uint32_t address, uint32_t size)
{
    uint32_t double_words = size / 8;
    uint64_t *data_ptr = (uint64_t *)buf;
    fmc_state_enum status;

    // printf("[Flash] Preparing to program %u double-words\r\n", double_words);
    __disable_irq(); // 禁用中断
    fmc_unlock();    // 解锁Flash

    for (uint32_t i = 0; i < double_words; i++)
    {
        uint32_t current_addr = address + i * 8;

        // 使用双字编程函数
        status = fmc_doubleword_program(current_addr, data_ptr[i]);

        if (status != FMC_READY)
        {
            // printf("[Flash] Error: Programming failed at 0x%08X, status: %d\r\n", current_addr, status);
            fmc_lock();
            __enable_irq();
            return 3;
        }
    }

    fmc_lock();     // 锁定Flash
    __enable_irq(); // 恢复中断

    return 0;
}

// 在底层实现的适配
int Bootloader_Write_App_Size(uint32_t size)
{
    return Bootloader_Write_App_Info(size, App_Info.crc);
}

int Bootloader_Write_App_CRC(uint32_t crc)
{
    return Bootloader_Write_App_Info(App_Info.size, crc);
}

// 同时写入App_Size和App_CRC到同一个双字
int Bootloader_Write_App_Info(uint32_t size, uint32_t crc)
{
    fmc_state_enum status;
    uint64_t combined = ((uint64_t)crc << 32) | size;

    __disable_irq();
    fmc_unlock();
    status = fmc_doubleword_program(APP_SIZE_ADDR, combined);
    fmc_lock();
    __enable_irq();

    if (status == FMC_READY)
    {
        App_Info.size = size;
        App_Info.crc = crc;
        //printf("[Flash] Write OK - Size:%u CRC:0x%08X\n", size, crc);
        return 0;
    }

    //printf("[Flash] Write FAIL! Addr:0x%08X Stat:%d\n", APP_SIZE_ADDR, status);
    return 1;
}

/*跳转到应用App，跳转之前先关闭已经使用的外设，注意，该函数只能在中断外执行，否则跳转后无法再进入中断*/
// void Bootloader_RunAPP(void)
//{
//     volatile vector_t *vector_p = (vector_t *)APP_START_ADDR;
//	   volatile uint32_t stack_arr[100]    = {0}; // Allocate some stack
//                                                // just to show that
//                                                // the SP should be reset
//                                                // before the jump - or the
//                                                // stack won't be configured
//                                                // correctly.
//     __disable_irq(); // 关闭所有中断
//     __set_FAULTMASK(1);	//关闭中断,确保跳转过程中 不会进入中断,导致跳转失败

//    // 重置所有外设到默认状态
//	//bsp_deinit();
//
//    // 配置堆栈指针和向量表
//    __set_MSP(vector_p->stack_addr);

//     SCB->VTOR = APP_START_ADDR;

//    // 跳转到应用
//    vector_p->func_p();
//
//}
void Bootloader_RunAPP(void)
{
    // 关闭SysTick定时器并清除中断标志
    SysTick->CTRL = 0;                  // 禁用SysTick
    SysTick->VAL = 0;                   // 清除当前计数值
    NVIC_ClearPendingIRQ(SysTick_IRQn); // 清除SysTick中断挂起标志

    // 清除所有中断挂起标志
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICPR[i] = 0xFFFFFFFF; 
    }
	// 清除PendSV挂起标志
    SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk; 

    // 使用volatile防止编译器优化
    volatile uint32_t *app_vector = (volatile uint32_t *)APP_START_ADDR;

    // 直接获取关键地址
    volatile uint32_t stack_ptr = app_vector[0];     // 栈指针在0x00偏移
    volatile uint32_t reset_handler = app_vector[1]; // 复位函数在0x04偏移

    // 关闭所有中断
    __disable_irq();
	// 取消任何优先级屏蔽
    __set_BASEPRI(0); 

    // 重置所有外设到默认状态
    bsp_deinit();

    // 设置VTOR前添加屏障
    __DSB();
    __ISB();

    // 设置向量表偏移（128字节对齐）
    SCB->VTOR = (uint32_t)APP_START_ADDR & 0xFFFFFF80;

    // 完整的内存屏障序列
    __DSB();
    __ISB();

    // 执行跳转 - 先设置栈指针再跳转
    __ASM volatile(
        "msr msp, %0     \n\t" // 设置主栈指针
        "bx %1          \n\t"  // 跳转到复位处理函数
        : : "r"(stack_ptr),
        "r"(reset_handler));
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
    // 固定的ID
    return 0x21;
}