#include "bsp.h"
#include "Bootloader.h"
#include "gd32g5x3_fmc.h"
#include "gd32g5x3.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// ȫ�ֱ�������
Memory_Info App_Flash_Info;
Memory_Info Boot_Flash_Info;
Memory_Info Ram_Info;

int Bootloader_Jump_Flag = 0;
Info_t App_Info;
Info_t Boot_Info;

// �Զ��峣��
#define FLASH_PAGE_SIZE fmc_page_size_get() // ��̬��ȡҳ��С

// ����Flash�ȴ�״̬
static void configure_flash_waitstates(void)
{
    // ����FMC�ȴ�״̬Ϊ7 (216MHz��Ҫ)
    fmc_wscnt_set(FMC_WAIT_STATE_7);
}

void Bootloader_Hal_Init(void)
{
    // ����216MHz�ȴ�״̬
    configure_flash_waitstates();

    // Bootloader Flash��Ϣ
    Boot_Flash_Info.page_size = FLASH_PAGE_SIZE;
    Boot_Flash_Info.start_addr = BOOT_START_ADDR;
    Boot_Flash_Info.size = BOOT_SIZE;
    Boot_Flash_Info.end_addr = BOOT_END_ADDR;
    Boot_Flash_Info.page_num = BOOT_SIZE / FLASH_PAGE_SIZE;

    // App Flash��Ϣ
    App_Flash_Info.page_size = FLASH_PAGE_SIZE;
    App_Flash_Info.start_addr = APP_START_ADDR;
    App_Flash_Info.size = APP_SIZE;
    App_Flash_Info.end_addr = APP_END_ADDR;
    App_Flash_Info.page_num = APP_SIZE / FLASH_PAGE_SIZE;

    // RAM��Ϣ
    Ram_Info.page_size = 4;
    Ram_Info.start_addr = Sram_UpdateFlag_Vector;
    Ram_Info.size = 0x10; // 16�ֽ�
    Ram_Info.end_addr = SRAM_BASE + 0x10 - 1;
    Ram_Info.page_num = 4;

    // App��Ϣ
    App_Info.addr_size = APP_SIZE_ADDR;
    App_Info.addr_crc = APP_CRC_ADDR;
    App_Info.size = *((uint32_t *)(App_Info.addr_size));
    // App_Info.size = 22288;//����ʹ��
    App_Info.crc = *((uint32_t *)(App_Info.addr_crc));
    App_Info.hwId = Bootloader_GethwID();

    // Boot��Ϣ
    Boot_Info.addr_size = BOOT_SIZE_ADDR;
    Boot_Info.addr_crc = BOOT_CRC_ADDR;
    Boot_Info.size = *((uint32_t *)(Boot_Info.addr_size));
    Boot_Info.crc = *((uint32_t *)(Boot_Info.addr_crc));
    Boot_Info.hwId = Bootloader_GethwID();
}

/* ���Boot����������*/
int Bootloader_CheckBoot(void)
{
    uint32_t boot_size = Boot_Info.size;
    uint32_t boot_crc = Boot_Info.crc;

    if (boot_size > Boot_Flash_Info.size)
        return 1;

    uint32_t crc = Bootloader_GetBootCRC();
    return (crc != boot_crc);
}

/* ���Boot�����CRCУ����*/
uint32_t Bootloader_GetBootCRC(void)
{
    uint32_t boot_size = Boot_Info.size;

    crc_data_register_reset(); // ��λ���ݼĴ���

    if (boot_size > Boot_Flash_Info.size)
        return 0;

    return crc_block_data_calculate((void *)Boot_Flash_Info.start_addr,
                                    boot_size / sizeof(uint32_t),
                                    INPUT_FORMAT_WORD);
}

/* ���APP����������*/
int Bootloader_CheckApp(void)
{
    uint32_t app_size = App_Info.size;
    uint32_t app_crc = App_Info.crc;

    if (app_size > App_Flash_Info.size)
        return 1;

    uint32_t crc = Bootloader_GetAppCRC();
    return (crc != app_crc);
}

/* ���APP�����CRC���*/
uint32_t Bootloader_GetAppCRC(void)
{
    uint32_t app_size = App_Info.size;

    crc_data_register_reset(); // ��λ���ݼĴ���

    if (app_size > App_Flash_Info.size)
        return 0;

    return crc_block_data_calculate((void *)App_Flash_Info.start_addr,
                                    app_size / sizeof(uint32_t),
                                    INPUT_FORMAT_WORD);
}

/*
 * ����APP����
 */
int Bootloader_EraseApp(void)
{
    fmc_state_enum status;
    uint32_t start_page, end_page;
    uint32_t page_size = App_Flash_Info.page_size;

    // ������ʼҳ��Bank0���֣�
    uint32_t start_addr = App_Flash_Info.start_addr;
    uint32_t bank0_end_addr = 0x0803FFFFUL; // Bank0������ַ
    __disable_irq();                        // ���ж�
    // Bank0���ֵ�ҳ��Χ
    if (start_addr <= bank0_end_addr)
    {
        // Bank0������Ҫ������ҳ
        start_page = (start_addr - BOOT_START_ADDR) / page_size;
        end_page = 255; // Bank0�����ҳ��

        __disable_irq(); // ���ж�
        // ����Flash
        fmc_unlock();

        // ����Bank0��Appҳ
        for (uint32_t i = start_page; i <= end_page; i++)
        {
            status = fmc_page_erase(FMC_BANK0, i);
            if (status != FMC_READY)
            {
                fmc_lock();
                __enable_irq(); // ���ж�
                return 1;
            }
        }

        // ����Flash��Bank1����ʱ���ٴν�����
        fmc_lock();
        __enable_irq(); // ���ж�
    }

    // Bank1���ֵ�ҳ��Χ��0��App������ַ���ڵ�ҳ��
    uint32_t bank1_start_addr = 0x08040000; // Bank1��ʼ��ַ

    if (App_Flash_Info.end_addr >= bank1_start_addr)
    {
        // ����Bank1���ֵ�ҳ��Χ
        start_page = 0; // Bank1��ʼҳ
        end_page = (App_Flash_Info.end_addr - bank1_start_addr) / page_size;

        __disable_irq(); // ���ж�
        // ����Flash
        fmc_unlock();

        // ����Bank1��Appҳ
        for (uint32_t i = start_page; i <= end_page; i++)
        {
            status = fmc_page_erase(FMC_BANK1, i);
            if (status != FMC_READY)
            {
                fmc_lock();
                __enable_irq(); // ���ж�
                return 1;
            }
        }

        // ��������Flash
        fmc_lock();
        __enable_irq(); // ���ж�
    }

    return 0;
}

/*
 * ����ȫ��flash����
 */
int Bootloader_EraseAllFlash(void)
{
    fmc_state_enum status;

    __disable_irq(); // ���ж�
    // ����Flash
    fmc_unlock();

    // Bank0�������
    status = fmc_bank0_erase();
    if (status != FMC_READY)
    {
        fmc_lock();
        __enable_irq(); // ���ж�
        return 1;
    }

    // Bank1�������
    status = fmc_bank1_erase();
    fmc_lock();
    __enable_irq(); // ���ж�
    return (status == FMC_READY) ? 0 : 1;
}

///*
//* ˫�ֽ���¼
//*/
int Bootloader_ProgramBlock(unsigned char *buf, uint32_t address, uint32_t size)
{
    uint32_t double_words = size / 8;
    uint64_t *data_ptr = (uint64_t *)buf;
    fmc_state_enum status;

    // printf("[Flash] Preparing to program %u double-words\r\n", double_words);
    __disable_irq(); // �����ж�
    fmc_unlock();    // ����Flash

    for (uint32_t i = 0; i < double_words; i++)
    {
        uint32_t current_addr = address + i * 8;

        // ʹ��˫�ֱ�̺���
        status = fmc_doubleword_program(current_addr, data_ptr[i]);

        if (status != FMC_READY)
        {
            // printf("[Flash] Error: Programming failed at 0x%08X, status: %d\r\n", current_addr, status);
            fmc_lock();
            __enable_irq();
            return 3;
        }
    }

    fmc_lock();     // ����Flash
    __enable_irq(); // �ָ��ж�

    return 0;
}

// �ڵײ�ʵ�ֵ�����
int Bootloader_Write_App_Size(uint32_t size)
{
    return Bootloader_Write_App_Info(size, App_Info.crc);
}

int Bootloader_Write_App_CRC(uint32_t crc)
{
    return Bootloader_Write_App_Info(App_Info.size, crc);
}

// ͬʱд��App_Size��App_CRC��ͬһ��˫��
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

/*��ת��Ӧ��App����ת֮ǰ�ȹر��Ѿ�ʹ�õ����裬ע�⣬�ú���ֻ�����ж���ִ�У�������ת���޷��ٽ����ж�*/
// void Bootloader_RunAPP(void)
//{
//     volatile vector_t *vector_p = (vector_t *)APP_START_ADDR;
//	   volatile uint32_t stack_arr[100]    = {0}; // Allocate some stack
//                                                // just to show that
//                                                // the SP should be reset
//                                                // before the jump - or the
//                                                // stack won't be configured
//                                                // correctly.
//     __disable_irq(); // �ر������ж�
//     __set_FAULTMASK(1);	//�ر��ж�,ȷ����ת������ ��������ж�,������תʧ��

//    // �����������赽Ĭ��״̬
//	//bsp_deinit();
//
//    // ���ö�ջָ���������
//    __set_MSP(vector_p->stack_addr);

//     SCB->VTOR = APP_START_ADDR;

//    // ��ת��Ӧ��
//    vector_p->func_p();
//
//}
void Bootloader_RunAPP(void)
{
    // �ر�SysTick��ʱ��������жϱ�־
    SysTick->CTRL = 0;                  // ����SysTick
    SysTick->VAL = 0;                   // �����ǰ����ֵ
    NVIC_ClearPendingIRQ(SysTick_IRQn); // ���SysTick�жϹ����־

    // ��������жϹ����־
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICPR[i] = 0xFFFFFFFF; 
    }
	// ���PendSV�����־
    SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk; 

    // ʹ��volatile��ֹ�������Ż�
    volatile uint32_t *app_vector = (volatile uint32_t *)APP_START_ADDR;

    // ֱ�ӻ�ȡ�ؼ���ַ
    volatile uint32_t stack_ptr = app_vector[0];     // ջָ����0x00ƫ��
    volatile uint32_t reset_handler = app_vector[1]; // ��λ������0x04ƫ��

    // �ر������ж�
    __disable_irq();
	// ȡ���κ����ȼ�����
    __set_BASEPRI(0); 

    // �����������赽Ĭ��״̬
    bsp_deinit();

    // ����VTORǰ�������
    __DSB();
    __ISB();

    // ����������ƫ�ƣ�128�ֽڶ��룩
    SCB->VTOR = (uint32_t)APP_START_ADDR & 0xFFFFFF80;

    // �������ڴ���������
    __DSB();
    __ISB();

    // ִ����ת - ������ջָ������ת
    __ASM volatile(
        "msr msp, %0     \n\t" // ������ջָ��
        "bx %1          \n\t"  // ��ת����λ������
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
    // �̶���ID
    return 0x21;
}