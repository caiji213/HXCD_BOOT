/*!
    \file    at24cxx.h
    \brief   the header file of AT24Cxx
*/

#ifndef AT24CXX_H
#define AT24CXX_H

#include "gd32g5x3.h"

typedef enum {
  I2C_START= 0,     // ����
  I2C_SEND_ADDRESS, // ���͵�ַ
  I2C_RESTART,      // ��������
  I2C_TRANSMIT_DATA,// DMA����
  I2C_RELOAD,       // ����ģʽ
  I2C_STOP,         // ֹͣ
  I2C_END           // ����
} i2c_process_enum;

// ҳ��С���� (24C64ҳ��СΪ32)
#define I2C_PAGE_SIZE               32       // I2Cҳд������������ֽ���
#define I2C_TIME_OUT                (uint16_t)(180000)  // I2C������ʱʱ��
#define I2C_WRITE_TIMEOUT_CYCLES    1080000   // I2C�ȴ���Ӧ��ʱʱ��
#define EEP_FIRST_PAGE              0x0000    // EEPROM��ʼҳ��ַ
#define I2C_OK                      0         // I2C�����ɹ�״̬
#define I2C_FAIL                    1         // I2C����ʧ��״̬
#define EEPROM_BLOCK0_ADDRESS       0xA0      // EEPROM Block0�豸��ַ(I2C�ӻ���ַ)


/* �������� */
uint8_t i2c_24c64_test(void);  
void i2c_eeprom_init(void);
void i2c_dma_tx_callback(void);
void i2c_dma_rx_callback(void);
uint8_t quick_byte_test(uint16_t test_address, uint8_t test_value);
uint8_t eeprom_byte_write(uint8_t data, uint16_t write_address);
void eeprom_buffer_write(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte);
void eeprom_page_write(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte); 
void eeprom_buffer_read(uint8_t *p_buffer, uint16_t read_address, uint16_t number_of_byte); 

#endif /* AT24CXX_H */