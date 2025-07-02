/*!
    \file    at24cxx.h
    \brief   the header file of AT24Cxx
*/

#ifndef AT24CXX_H
#define AT24CXX_H

#include "gd32g5x3.h"

typedef enum {
  I2C_START= 0,     // 启动
  I2C_SEND_ADDRESS, // 发送地址
  I2C_RESTART,      // 重新启动
  I2C_TRANSMIT_DATA,// DMA传输
  I2C_RELOAD,       // 重载模式
  I2C_STOP,         // 停止
  I2C_END           // 结束
} i2c_process_enum;

// 页大小定义 (24C64页大小为32)
#define I2C_PAGE_SIZE               32       // I2C页写操作单次最大字节数
#define I2C_TIME_OUT                (uint16_t)(180000)  // I2C操作超时时间
#define I2C_WRITE_TIMEOUT_CYCLES    1080000   // I2C等待回应超时时间
#define EEP_FIRST_PAGE              0x0000    // EEPROM起始页地址
#define I2C_OK                      0         // I2C操作成功状态
#define I2C_FAIL                    1         // I2C操作失败状态
#define EEPROM_BLOCK0_ADDRESS       0xA0      // EEPROM Block0设备地址(I2C从机地址)


/* 函数声明 */
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