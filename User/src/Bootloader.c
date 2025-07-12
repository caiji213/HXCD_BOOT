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

/* ==================================================================
 * Flash������Ϣ��GD32G553 512KB Flash - ˫Bankģʽ DBS=1��
 * ÿҳ��С��1KB
 * Bank0��0x08000000 - 0x0803FFFF����256ҳ
 * Bank1��0x08040000 - 0x0807FFFF����256ҳ
 *
 * ҳ���    Page    Begin           End             Size(Dec)    Size(hex)
 * ==================================================================
 * Bank0-000    0     0x08000000     0x080003FF      1024          0x400    <- Boot ��ʼҳ
 * Bank0-001    1     0x08000400     0x080007FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank0-031    31    0x08007C00     0x08007FFF      1024          0x400    <- Boot ���һҳ
 * Bank0-032    32    0x08008000     0x080083FF      1024          0x400    <- Crypto ��
 * Bank0-033    33    0x08008400     0x080087FF      1024          0x400    <- App ��ʼҳ
 * ...         ...     ...           ...              ...           ...
 * Bank0-255    255   0x0803FC00     0x0803FFFF      1024          0x400
 * ==================================================================
 * Bank1-000    0     0x08040000     0x080403FF      1024          0x400
 * ...         ...     ...           ...              ...           ...
 * Bank1-252    252   0x0807F800     0x0807FBFF      1024          0x400
 * Bank1-253    253   0x0807FC00     0x0807FFFF      1024          0x400    <- App ���һҳ����APPУ�����ݣ�
 * ==================================================================
 *
 * �����滮��
 * 1. Boot  ʹ�� Bank0 ҳ 0~31           (32ҳ = 32KB) -> 0x08000000 ~ 0x08007FFF
 * 2. Crypto ʹ�� Bank0 ҳ 32            (1ҳ = 1KB)   -> 0x08008000 ~ 0x080083FF
 * 3. App   ʹ�� Bank0 ҳ 33~255 + Bank1 ҳ 0~253 (479KB) -> 0x08008400 ~ 0x0807FFFF
 *
 * ==================================================================================
 * ��������    ��ʼ��ַ       ������ַ        ��С       ʹ��˵��
 * ----------------------------------------------------------------------------------
 * Boot ��     0x08000000     0x08007FFF      32KB       Bootloader �̼�����
 *             - ������ʼ�����ж���������ת App��ͨ������Э����߼�
 *
 * Crypto ��   0x08008000     0x080083FF      1KB
 *             - �洢AES������Կ���豸֤�顢��ȫ���õ���������
 *             - Ӳ����������ר�÷������򣬽�ֹ��ͨ�������
 *             - ʹ������д�������Ʒ�ֹδ��Ȩ��ȡ
 *
 * App ��      0x08008400     0x0807FFFF      479KB      Ӧ�ó������̼�
 *             - ʵ���û����롢ҵ���߼�������ʱ��Դ
 *             - ��� 8 �ֽ����ڴ洢APPУ�����ݣ�
 *                 0x0807FFF8��App_Size��4�ֽڣ�
 *                 0x0807FFFC��App_CRC32��4�ֽڣ�
 *
 * ==================================================================================
 * APPУ������λ�ã�
 * - App��Ϣ�洢��App�������8�ֽ� (Bank1ҳ253�����8�ֽ�)
 *   - ��С��ַ: 0x0807FFF8
 *   - CRC��ַ:  0x0807FFFC
 * ==================================================================
 */

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
   // configure_flash_waitstates();

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
    App_Info.crc = *((uint32_t *)(App_Info.addr_crc));
    App_Info.hwId = Bootloader_GethwID();

    // Boot��Ϣ
    Boot_Info.addr_size = BOOT_END_ADDR - 8;
    Boot_Info.addr_crc = BOOT_END_ADDR - 4;
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
                                    boot_size,
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
                                    app_size,
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
    __disable_irq(); //���ж�
    // Bank0���ֵ�ҳ��Χ
    if (start_addr <= bank0_end_addr)
    {
        // Bank0������Ҫ������ҳ
        start_page = (start_addr - BOOT_START_ADDR) / page_size;
        end_page = 255; // Bank0�����ҳ��
		
        __disable_irq(); //���ж�
        // ����Flash
        fmc_unlock();
        
		
        // ����Bank0��Appҳ
        for (uint32_t i = start_page; i <= end_page; i++)
        {
            status = fmc_page_erase(FMC_BANK0, i);
            if (status != FMC_READY)
            {
                fmc_lock();
				__enable_irq(); //���ж�
                return 1;
            }
        }
        
        // ����Flash��Bank1����ʱ���ٴν�����
        fmc_lock();
		__enable_irq(); //���ж�
    }

    // Bank1���ֵ�ҳ��Χ��0��App������ַ���ڵ�ҳ��
    uint32_t bank1_start_addr = 0x08040000; // Bank1��ʼ��ַ

    if (App_Flash_Info.end_addr >= bank1_start_addr)
    {
        // ����Bank1���ֵ�ҳ��Χ
        start_page = 0; // Bank1��ʼҳ
        end_page = (App_Flash_Info.end_addr - bank1_start_addr) / page_size;
		
        __disable_irq(); //���ж�
        // ����Flash
        fmc_unlock();
       
		
        // ����Bank1��Appҳ
        for (uint32_t i = start_page; i <= end_page; i++)
        {
            status = fmc_page_erase(FMC_BANK1, i);
            if (status != FMC_READY)
            {
                fmc_lock();
				__enable_irq(); //���ж�
                return 1;
            }
        }

        // ��������Flash
        fmc_lock();
		__enable_irq(); //���ж�
    }

    return 0;
}

/*
* ����ȫ��flash����
*/
int Bootloader_EraseAllFlash(void)
{
    fmc_state_enum status;
	
    __disable_irq(); //���ж�
    // ����Flash
    fmc_unlock();

    // Bank0�������
    status = fmc_bank0_erase();
    if (status != FMC_READY)
    {
        fmc_lock();
		__enable_irq(); //���ж�
        return 1;
    }

    // Bank1�������
    status = fmc_bank1_erase();
    fmc_lock();
    __enable_irq(); //���ж�
    return (status == FMC_READY) ? 0 : 1;
}

///*
//* ˫�ֽ���¼
//*/
//int Bootloader_ProgramBlock(unsigned char *buf, uint32_t address, uint32_t size)
//{
//    // ȷ����ַ��APP����
//    if (address < APP_START_ADDR || address > APP_END_ADDR)
//        return 1;

//    // ȷ����С��8�ı��� (˫�ֶ���)
//    if (size % 8 != 0)
//        return 2;

//    uint32_t words = size / 8;
//    uint64_t *data_ptr = (uint64_t *)buf;
//    fmc_state_enum status;
//	
//    __disable_irq(); //���ж�
//    // ����Flash
//    fmc_unlock();

//    for (uint32_t i = 0; i < words; i++)
//    {
//        status = fmc_doubleword_program(address + i * 8, data_ptr[i]);
//        if (status != FMC_READY)
//        {
//			// ��������Flash
//            fmc_lock();
//			__enable_irq(); //���ж�
//            return 3;
//        }
//    }

//    // ��������Flash
//    fmc_lock();
//	__enable_irq(); //���ж�
//    return 0;
//}
//int Bootloader_ProgramBlock(unsigned char *buf, uint32_t address, uint32_t size)
//{
//    uint32_t double_words = size / 8;
//    uint64_t *data_ptr = (uint64_t *)buf;
//    fmc_state_enum status;
//    
//    //printf("[Flash] Preparing to program %u double-words\r\n", double_words);
//    __disable_irq(); // �����ж�
//    fmc_unlock();    // ����Flash

//    for (uint32_t i = 0; i < double_words; i++)
//    {
//        uint32_t current_addr = address + i * 8;
//        
//        // �ؼ��޸ģ�ʹ��˫�ֱ�̺���
//        status = fmc_doubleword_program(current_addr, data_ptr[i]);
//        
//        if (status != FMC_READY)
//        {
//            //printf("[Flash] Error: Programming failed at 0x%08X, status: %d\r\n", current_addr, status);
//            fmc_lock();
//            __enable_irq();
//            return 3;
//        }
//    }

//    fmc_lock();     // ����Flash
//    __enable_irq(); // �ָ��ж�
//    
//    return 0;
//}

static FlashProgramState s_prog_state = {0};

void Bootloader_Program_Init(void)
{
    s_prog_state.buffered = 0;
}
/*
* �µı�̺�����Ҫ�����ݴ�С��4�ı���
*/
int Bootloader_ProgramBlock(unsigned char *buf, uint32_t address, uint32_t size)
{
    // ����ַ�Ƿ���APP����
    if (address < APP_START_ADDR || address > APP_END_ADDR)
        return 1;
    
    // ȷ����С��4�ı���
    if (size % 4 != 0)
        return 2;
    
    fmc_state_enum status;
    uint8_t *data_ptr = buf;
    uint32_t remaining = size;
    
    __disable_irq();
    fmc_unlock();
    
    // 1. �������������4�ֽ����ݣ��뱾������ǰ4�ֽںϲ�
    if (s_prog_state.buffered == 4) {
        // �ϲ���8�ֽ� (��������4�ֽ� + ����ǰ4�ֽ�)
        memcpy(s_prog_state.buffer + 4, data_ptr, 4);
        
        // д������8�ֽ�
        status = fmc_doubleword_program(address, *((uint64_t *)s_prog_state.buffer));
        if (status != FMC_READY) {
            fmc_lock();
            __enable_irq();
            return 3;
        }
        
        address += 8;
        data_ptr += 4;
        remaining -= 4;
        s_prog_state.buffered = 0;
    }
    
    // 2. ����������8�ֽڿ�
    while (remaining >= 8) {
        status = fmc_doubleword_program(address, *((uint64_t *)data_ptr));
        if (status != FMC_READY) {
            fmc_lock();
            __enable_irq();
            return 3;
        }
        
        address += 8;
        data_ptr += 8;
        remaining -= 8;
    }
    
    // 3. ����ʣ�����ݣ���֤��4�ֽڣ�
    if (remaining == 4) {
        // ��ʣ��4�ֽڴ��뻺����
        memcpy(s_prog_state.buffer, data_ptr, 4);
        s_prog_state.buffered = 4;
    } else {
        s_prog_state.buffered = 0;
    }
    
    fmc_lock();
    __enable_irq();
    return 0;
}
int Bootloader_Write_App_CRC(uint32_t crc)
{
    uint64_t combined = ((uint64_t)crc << 32) | App_Info.size;
    fmc_state_enum status;
	
    __disable_irq(); //���ж�
    // ����Flash
    fmc_unlock();

    status = fmc_doubleword_program(APP_SIZE_ADDR, combined);

    // ��������Flash
    fmc_lock();
    __enable_irq(); //���ж�
	
    if (status == FMC_READY)
    {
        App_Info.crc = crc;
        return 0;
    }
    return 1;
}

int Bootloader_Write_App_Size(uint32_t size)
{
    uint64_t combined = ((uint64_t)App_Info.crc << 32) | size;
    fmc_state_enum status;

	__disable_irq(); //���ж�
    // ����Flash
    fmc_unlock();

    status = fmc_doubleword_program(APP_SIZE_ADDR, combined);


    // ��������Flash
    fmc_lock();
    __enable_irq(); //���ж�
	
    if (status == FMC_READY)
    {
        App_Info.size = size;
        return 0;
    }
    return 1;
}
/*��ת��Ӧ��App����ת֮ǰ�ȹر��Ѿ�ʹ�õ����裬ע�⣬�ú���ֻ�����ж���ִ�У�������ת���޷��ٽ����ж�*/
//void Bootloader_RunAPP(void)
//{
//    volatile vector_t *vector_p = (vector_t *)APP_START_ADDR;
//	volatile uint32_t stack_arr[100]    = {0}; // Allocate some stack
//                                               // just to show that
//                                               // the SP should be reset
//                                               // before the jump - or the
//                                               // stack won't be configured
//                                               // correctly.
//    __disable_irq(); // �ر������ж�
//    __set_FAULTMASK(1);	//�ر��ж�,ȷ����ת������ ��������ж�,������תʧ��

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
// void Bootloader_RunAPP(void)
//{
//    // ʹ��volatile��ֹ�������Ż�
//    volatile uint32_t *app_vector = (volatile uint32_t *)APP_START_ADDR;
//    
//    // ֱ�ӻ�ȡ�ؼ���ַ������ͨ��ָ��ṹ�壩
//    volatile uint32_t stack_ptr = app_vector[0];  // ջָ����0x00ƫ��
//    volatile uint32_t reset_handler = app_vector[1]; // ��λ������0x04ƫ��
//    
//    // �ر������ж�
//    __disable_irq();
//	
//    // �����������赽Ĭ��״̬
//    bsp_deinit();
//	
//    // ����VTORǰ�������
//    __DSB();
//    
//    // ����������ƫ�ƣ�128�ֽڶ��룩
//    SCB->VTOR = APP_START_ADDR & 0xFFFFFF80;
//    
//    // �������ڴ���������
//    __DSB();
//    __ISB();
//    
//    // ʹ��������లȫ��ת
//    __ASM volatile(
//        "msr msp, %0  \n\t"   // ������ջָ��
//        "bx %1       \n\t"   // ��ת����λ������
//        : 
//        : "r" (stack_ptr), "r" (reset_handler)
//    );
//}
void Bootloader_RunAPP(void)
{
    // �ر�SysTick��ʱ��������жϱ�־
    SysTick->CTRL = 0;                     // ����SysTick
    SysTick->VAL = 0;                      // �����ǰ����ֵ
    NVIC_ClearPendingIRQ(SysTick_IRQn);    // ���SysTick�жϹ����־

    // ��������жϹ����־
    for (int i = 0; i < 8; i++)            // ��������32λNVIC����Ĵ���(IRQ0-IRQ239)
	{  
        NVIC->ICPR[i] = 0xFFFFFFFF;        // ������й����ж�
    }
    SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;   // ���PendSV�����־
    
    // ʹ��volatile��ֹ�������Ż�
    volatile uint32_t *app_vector = (volatile uint32_t *)APP_START_ADDR;
    
    // ֱ�ӻ�ȡ�ؼ���ַ
    volatile uint32_t stack_ptr = app_vector[0];     // ջָ����0x00ƫ��
    volatile uint32_t reset_handler = app_vector[1]; // ��λ������0x04ƫ��
    
    // �ر������ж�
    __disable_irq();
    __set_BASEPRI(0);  // ȡ���κ����ȼ�����

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
        "msr msp, %0     \n\t"    // ������ջָ��
        "bx %1          \n\t"     // ��ת����λ������
        : 
        : "r" (stack_ptr), "r" (reset_handler)
    );
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