#include "Bootloader.h"
#include "gd32g5x3_fmc.h"

// ȫ�ֱ�������
Memory_Info App_Flash_Info;
Memory_Info Boot_Flash_Info;
Memory_Info Ram_Info;

int Bootloader_Jump_Flag = 0;
Info_t App_Info;
Info_t Boot_Info;

// �Զ��峣��
#define FLASH_PAGE_SIZE     fmc_page_size_get()  // ��̬��ȡҳ��С
//#define FMC_TIMEOUT_COUNT   1000000UL            // FMC������ʱ����

// ===================================================================
// ��������
// ===================================================================

// ����Flash�ȴ�״̬��������ã�
static void configure_flash_waitstates(void)
{
    // ����FMC�ȴ�״̬Ϊ7 (216MHz��Ҫ)
    fmc_wscnt_set(FMC_WAIT_STATE_7);
}

// ˫�ֱ��ԭ�Ӳ���
static int program_doubleword(uint32_t address, uint64_t data)
{
    fmc_state_enum status = fmc_doubleword_program(address, data);
    
    if(status != FMC_READY) {
        return 1;
    }
    
    // ��������־
    fmc_flag_clear(FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    
    return 0;
}

// Ԫ���ݱ�̺�����ԭ�Ӳ�����
static int program_metadata(uint32_t size, uint32_t crc)
{
    uint32_t meta_addr = APP_SIZE_ADDR;
    uint64_t combined = ((uint64_t)crc << 32) | size;
    
    return program_doubleword(meta_addr, combined);
}

// ===================================================================
// ��������ʵ��
// ===================================================================

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

    // RAM��Ϣ (����16�ֽڴ洢��־)
    Ram_Info.page_size = 4;
    Ram_Info.start_addr = SRAM_BASE;
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

int Bootloader_EraseApp(void)
{
    fmc_state_enum status;
    uint32_t i;
    
    // ����Flash
    fmc_unlock();
    
    // Bank0���� (ҳ61-255, 195ҳ)
    for(i = 61; i <= 255; i++) {
        status = fmc_page_erase(FMC_BANK0, i);
        if(status != FMC_READY) {
            fmc_lock();
            return 1;
        }
    }
    
    // Bank1���� (ҳ0-252, 253ҳ)
    for(i = 0; i <= 252; i++) {
        status = fmc_page_erase(FMC_BANK1, i);
        if(status != FMC_READY) {
            fmc_lock();
            return 1;
        }
    }
    
    // ��������Flash
    fmc_lock();
    return 0;
}

int Bootloader_EraseAllFlash(void)
{
    fmc_state_enum status;
    
    // ����Flash
    fmc_unlock();
    
    // Bank0�������
    status = fmc_bank0_erase();
    if(status != FMC_READY) {
        fmc_lock();
        return 1;
    }
    
    // Bank1�������
    status = fmc_bank1_erase();
    fmc_lock();
    
    return (status == FMC_READY) ? 0 : 1;
}

int Bootloader_ProgramBlock(unsigned char * buf, uint32_t address, uint32_t size)
{
    // ȷ����ַ��APP����
    if(address < APP_START_ADDR || address > APP_END_ADDR)
        return 1;
    
    // ȷ����С��8�ı��� (˫�ֶ���)
    if(size % 8 != 0)
        return 2;
    
    uint32_t words = size / 8;
    uint64_t *data_ptr = (uint64_t*)buf;
    
    for(uint32_t i = 0; i < words; i++) {
        if(program_doubleword(address + i*8, data_ptr[i]) != 0) {
            return 3;
        }
    }
    return 0;
}

int Bootloader_Write_App_CRC(uint32_t crc)
{
    // ԭ�Ӳ���д��Ԫ����
    return program_metadata(App_Info.size, crc);
}

int Bootloader_Write_App_Size(uint32_t size)
{
    // ԭ�Ӳ���д��Ԫ����
    return program_metadata(size, App_Info.crc);
}

void Bootloader_RunAPP(void)
{
    const vector_t *vector_p = (vector_t*)App_Flash_Info.start_addr;
    
    __disable_irq();  // �ر������ж�
    
    // �����������赽Ĭ��״̬
    rcu_deinit();
    
    // ���ö�ջָ���������
    __set_MSP(vector_p->stack_addr);
    SCB->VTOR = App_Flash_Info.start_addr;
    
    // ��ת��Ӧ��
    vector_p->func_p();
    
    // �����ϲ���ִ�е�����
    while(1);
}

// ===================================================================
// ��������ʵ��
// ===================================================================

/* ���Boot����������*/
int Bootloader_CheckBoot(void)
{
    uint32_t boot_size = Boot_Info.size;
    uint32_t boot_crc = Boot_Info.crc;

    if(boot_size > Boot_Flash_Info.size) 
        return 1;
    
    uint32_t crc = Bootloader_GetBootCRC();
    return (crc != boot_crc);
}

/* ���APP����������*/
int Bootloader_CheckApp(void)
{
    uint32_t app_size = App_Info.size;
    uint32_t app_crc = App_Info.crc;

    if(app_size > App_Flash_Info.size)
        return 1;
    
    uint32_t crc = Bootloader_GetAppCRC();
    return (crc != app_crc);
}

/* ���Boot�����CRCУ����*/
uint32_t Bootloader_GetBootCRC(void)
{
    uint32_t boot_size = Boot_Info.size;
    
    crc_deinit(); // ��λCRC
    
    if(boot_size > Boot_Flash_Info.size)
        return 0;
    
    return crc_block_data_calculate((void *)Boot_Flash_Info.start_addr, 
                                    boot_size, 
                                    INPUT_FORMAT_WORD);
}

/* ���APP�����CRC���*/
uint32_t Bootloader_GetAppCRC(void)
{
    uint32_t app_size = App_Info.size;
    
    crc_deinit(); // ��λCRC
    
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
    // ��ȡоƬID (����Ĵ�����ο��ֲ�)
    return *((uint32_t *)0x1FFFF7E8);
}