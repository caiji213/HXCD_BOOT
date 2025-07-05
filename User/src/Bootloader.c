#include "bsp.h"
#include "Bootloader.h"
#include "gd32g5x3_fmc.h"

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
 * Bank0-015    15    0x08003C00     0x08003FFF      1024          0x400    <- Boot ���һҳ
 * Bank0-016    16    0x08004000     0x080043FF      1024          0x400    <- Crypto ��
 * Bank0-017    17    0x08004400     0x080047FF      1024          0x400    <- App ��ʼҳ
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
 * 1. Boot  ʹ�� Bank0 ҳ 0~15           (16ҳ = 16KB) -> 0x08000000 ~ 0x08003FFF
 * 2. Crypto ʹ�� Bank0 ҳ 16            (1ҳ = 1KB)   -> 0x08004000 ~ 0x080043FF
 * 3. App   ʹ�� Bank0 ҳ 17~255 + Bank1 ҳ 0~253 (495KB) -> 0x08004400 ~ 0x0807FFFF
 *
 * ==================================================================================
 * ��������    ��ʼ��ַ       ������ַ        ��С       ʹ��˵��
 * ----------------------------------------------------------------------------------
 * Boot ��     0x08000000     0x08003FFF      16KB       Bootloader �̼�����
 *             - ������ʼ�����ж���������ת App��ͨ������Э����߼�
 *
 * Crypto ��   0x08004000     0x080043FF      1KB
 *             - �洢AES������Կ���豸֤�顢��ȫ���õ���������
 *             - Ӳ����������ר�÷������򣬽�ֹ��ͨ�������
 *             - ʹ������д�������Ʒ�ֹδ��Ȩ��ȡ
 *
 * App ��      0x08004400     0x0807FFFF      495KB      Ӧ�ó������̼�
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
    App_Info.crc = *((uint32_t *)(App_Info.addr_crc));
    App_Info.hwId = Bootloader_GethwID();

    // Boot��Ϣ
    Boot_Info.addr_size = BOOT_END_ADDR - 7;
    Boot_Info.addr_crc = BOOT_END_ADDR - 3;
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

    crc_deinit(); // ��λCRC

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

    crc_deinit(); // ��λCRC

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

/*
* ˫�ֽ���¼
*/
int Bootloader_ProgramBlock(unsigned char *buf, uint32_t address, uint32_t size)
{
    // ȷ����ַ��APP����
    if (address < APP_START_ADDR || address > APP_END_ADDR)
        return 1;

    // ȷ����С��8�ı��� (˫�ֶ���)
    if (size % 8 != 0)
        return 2;

    uint32_t words = size / 8;
    uint64_t *data_ptr = (uint64_t *)buf;
    fmc_state_enum status;
	
    __disable_irq(); //���ж�
    // ����Flash
    fmc_unlock();

    for (uint32_t i = 0; i < words; i++)
    {
        status = fmc_doubleword_program(address + i * 8, data_ptr[i]);
        if (status != FMC_READY)
        {
            fmc_lock();
			__enable_irq(); //���ж�
            return 3;
        }
    }

    // ��������Flash
    fmc_lock();
	__enable_irq(); //���ж�
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

void Bootloader_RunAPP(void)
{
    const vector_t *vector_p = (vector_t *)App_Flash_Info.start_addr;

    __disable_irq(); // �ر������ж�

    // �����������赽Ĭ��״̬
    rcu_deinit();
	bsp_deinit();
		
    // ���ö�ջָ���������
    __set_MSP(vector_p->stack_addr);
    SCB->VTOR = App_Flash_Info.start_addr;

    // ��ת��Ӧ��
    vector_p->func_p();

    // �����ϲ���ִ�е�����
    while (1)
        ;
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