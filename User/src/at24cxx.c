/*!
    \file    at24cxx.c
    \brief   the read and write function file (modified for 24C64)

  \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "at24cxx.h"
#include "bsp_i2c.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE      8192    // 24c64总容量
#define TEXT_BUFFER_SIZE 300     // 测试的数据容量
/*
  如果发送字节数大于 255，必须通过将 I2C_CTL0 寄存器 RELOAD 位置 1 来使能重载模式。
  在重载模式下，当 BYTENUM[7:0]个字节传输完成，I2C_STAT 寄存器 TCR 位将置 1，
  并且在BYTENUM[7:0]更新一个非零值之前，SCL 被拉低。----- GD32G553用户手册1258页

  如果待接收字节数大于 255，必须将 I2C_CTL0 寄存器中 RELOAD 位置 1 来使能重载模式。
  在重载模式下，当 BYTENUM[7:0]个字节传输完成，I2C_STAT寄存器中 TCR 位将置 1，
  在 BYTENUM[7:0]中写入一个非零值之前，SCL 被拉低。----- GD32G553用户手册1260页
*/
#define MAX_RELOAD_SIZE 255 // 硬件I2C规定单次传输的字节数，大于255需要重载

#define MIN(a, b) ((a) < (b) ? (a) : (b))
uint16_t eeprom_address;
__attribute__((aligned(32))) uint8_t tx_buffer[BUFFER_SIZE] = {0};
__attribute__((aligned(32))) uint8_t i2c_buffer_read[BUFFER_SIZE] = {0};

__IO FlagStatus i2c_dma_tx_complete = RESET;
__IO FlagStatus i2c_dma_rx_complete = RESET;

/*!
    \brief      I2C读写测试函数 (24C64)
*/
#if 1
uint8_t i2c_24c64_test(void)
{
  uint16_t i;
  uint8_t i2c_buffer_write[TEXT_BUFFER_SIZE];

  printf("\r\nAT24C64 writing...\r\n");
  for (i = 0; i < TEXT_BUFFER_SIZE; i++)
  {
    i2c_buffer_write[i] = i & 0xFF;
  }

  // 打印写入的前16字节数据
  printf("Writing data (first 16 bytes): ");
  for (i = 0; i < 16; i++)
  {
    printf("%02X ", i2c_buffer_write[i]);
  }
  printf("\r\n");

  // 打印写入的最后16字节数据
  printf("Writing data (last 16 bytes): ");
  for (i = TEXT_BUFFER_SIZE - 16; i < TEXT_BUFFER_SIZE; i++)
  {
    printf("%02X ", i2c_buffer_write[i]);
  }
  printf("\r\n");

  eeprom_buffer_write(i2c_buffer_write, EEP_FIRST_PAGE, TEXT_BUFFER_SIZE);
  printf("AT24C64 reading...\r\n");
  delay_1ms(10);
  eeprom_buffer_read(i2c_buffer_read, EEP_FIRST_PAGE, TEXT_BUFFER_SIZE);

  // 打印读取的前16字节数据
  printf("Read data (first 16 bytes): ");
  for (i = 0; i < 16; i++)
  {
    printf("%02X ", i2c_buffer_read[i]);
  }
  printf("\r\n");

  // 打印读取的最后16字节数据
  printf("Read data (last 16 bytes): ");
  for (i = TEXT_BUFFER_SIZE - 16; i < TEXT_BUFFER_SIZE; i++)
  {
    printf("%02X ", i2c_buffer_read[i]);
  }
  printf("\r\n");

  // 详细比较数据并打印不匹配的位置
  uint8_t error_found = 0;
  for (i = 0; i < TEXT_BUFFER_SIZE; i++)
  {
    if (i2c_buffer_read[i] != i2c_buffer_write[i])
    {
      printf("Err: Mismatch at 0x%04X - Wrote: 0x%02X, Read: 0x%02X\r\n",
             EEP_FIRST_PAGE + i, i2c_buffer_write[i], i2c_buffer_read[i]);
      error_found = 1;

      // 打印错误位置附近的数据
      uint16_t start = (i > 8) ? i - 8 : 0;
      uint16_t end = (i < TEXT_BUFFER_SIZE - 8) ? i + 8 : TEXT_BUFFER_SIZE - 1;

      printf("Data around error (0x%04X):\r\n", EEP_FIRST_PAGE + i);
      printf("Address  | Written | Read\r\n");
      printf("----------------------------\r\n");

      for (uint16_t j = start; j <= end; j++)
      {
        printf("0x%04X |   0x%02X  |  0x%02X",
               EEP_FIRST_PAGE + j,
               i2c_buffer_write[j],
               i2c_buffer_read[j]);

        // 标记错误位置
        if (j == i)
        {
          printf("  <-- ERROR");
        }
        printf("\r\n");
      }
    }
  }

  if (error_found)
  {
    printf("I2C-AT24C64 test passed! All %d bytes match.\r\n", TEXT_BUFFER_SIZE);
    return I2C_OK;
  }
  else
  {
    // 额外打印整个缓冲区比较结果
    printf("\r\nFull buffer comparison:\r\n");
    for (i = 0; i < TEXT_BUFFER_SIZE; i += 16)
    {
      printf("Addr 0x%04X: ", EEP_FIRST_PAGE + i);
      for (uint16_t j = 0; j < 16 && (i + j) < TEXT_BUFFER_SIZE; j++)
      {
        if (i2c_buffer_read[i + j] == i2c_buffer_write[i + j])
        {
          printf("%02X ", i2c_buffer_read[i + j]);
        }
        else
        {
          printf("\033[31m%02X\033[0m ", i2c_buffer_read[i + j]); // 红色显示错误
        }
      }
      printf(" | ");
      for (uint16_t j = 0; j < 16 && (i + j) < TEXT_BUFFER_SIZE; j++)
      {
        printf("%c", (i2c_buffer_read[i + j] >= 32 && i2c_buffer_read[i + j] <= 126) ? i2c_buffer_read[i + j] : '.');
      }
      printf("\r\n");
    }

    return I2C_FAIL;
  }
}
#else
/*!
    \brief      I2C读写测试函数 (24C64) - 使用递减数据模式
*/
uint8_t i2c_24c64_test(void)
{
  uint16_t i;
  uint8_t i2c_buffer_write[TEXT_BUFFER_SIZE];

  printf("\r\nAT24C64 writing (decrement pattern)...\r\n");

  // 修改为从0xFF开始递减的数据模式
  for (i = 0; i < TEXT_BUFFER_SIZE; i++)
  {
    i2c_buffer_write[i] = (0xFF - (i % 0x100)); // 从0xFF递减到0x00循环
  }

  // 打印写入的前16字节数据（高值）
  printf("Writing data (first 16 bytes - high values): ");
  for (i = 0; i < 16; i++)
  {
    printf("%02X ", i2c_buffer_write[i]);
  }
  printf("\r\n");

  // 打印写入的最后16字节数据（低值）
  printf("Writing data (last 16 bytes - low values): ");
  for (i = TEXT_BUFFER_SIZE - 16; i < TEXT_BUFFER_SIZE; i++)
  {
    printf("%02X ", i2c_buffer_write[i]);
  }
  printf("\r\n");

  eeprom_buffer_write(i2c_buffer_write, EEP_FIRST_PAGE, TEXT_BUFFER_SIZE);
  printf("AT24C64 reading...\r\n");
  delay_1ms(10);
  eeprom_buffer_read(i2c_buffer_read, EEP_FIRST_PAGE, TEXT_BUFFER_SIZE);

  // 打印读取的前16字节数据
  printf("Read data (first 16 bytes): ");
  for (i = 0; i < 16; i++)
  {
    printf("%02X ", i2c_buffer_read[i]);
  }
  printf("\r\n");

  // 打印读取的最后16字节数据
  printf("Read data (last 16 bytes): ");
  for (i = TEXT_BUFFER_SIZE - 16; i < TEXT_BUFFER_SIZE; i++)
  {
    printf("%02X ", i2c_buffer_read[i]);
  }
  printf("\r\n");

  // 详细比较数据并打印不匹配的位置
  uint8_t error_found = 0;
  for (i = 0; i < TEXT_BUFFER_SIZE; i++)
  {
    uint8_t expected = (0xFF - (i % 0x100)); // 预期的递减值
    if (i2c_buffer_read[i] != expected)
    {
      printf("Err: Mismatch at 0x%04X - Expected: 0x%02X, Read: 0x%02X\r\n",
             EEP_FIRST_PAGE + i, expected, i2c_buffer_read[i]);
      error_found = 1;

      // 打印错误位置附近的数据
      uint16_t start = (i > 8) ? i - 8 : 0;
      uint16_t end = (i < TEXT_BUFFER_SIZE - 8) ? i + 8 : TEXT_BUFFER_SIZE - 1;

      printf("Data around error (0x%04X):\r\n", EEP_FIRST_PAGE + i);
      printf("Address  | Expected | Read\r\n");
      printf("--------------------------\r\n");

      for (uint16_t j = start; j <= end; j++)
      {
        uint8_t exp_val = (0xFF - (j % 0x100));
        printf("0x%04X |   0x%02X   |  0x%02X",
               EEP_FIRST_PAGE + j, exp_val, i2c_buffer_read[j]);

        // 标记错误位置
        if (j == i)
        {
          printf("  <-- ERROR");
        }
        printf("\r\n");
      }
      break; // 发现错误后中断，避免过多输出
    }
  }

  if (error_found)
  {
    printf("I2C-AT24C64 test passed! All %d bytes match the decrement pattern.\r\n", TEXT_BUFFER_SIZE);
    return I2C_OK;
  }
  else
  {
    // 额外打印整个缓冲区比较结果
    printf("\r\nFull buffer comparison (expected pattern):\r\n");
    for (i = 0; i < TEXT_BUFFER_SIZE; i += 16)
    {
      printf("Addr 0x%04X: ", EEP_FIRST_PAGE + i);
      for (uint16_t j = 0; j < 16 && (i + j) < TEXT_BUFFER_SIZE; j++)
      {
        uint8_t exp_val = (0xFF - ((i + j) % 0x100));
        if (i2c_buffer_read[i + j] == exp_val)
        {
          printf("%02X ", i2c_buffer_read[i + j]);
        }
        else
        {
          printf("\033[31m%02X\033[0m ", i2c_buffer_read[i + j]); // 红色显示错误
        }
      }
      printf(" | ");
      for (uint16_t j = 0; j < 16 && (i + j) < TEXT_BUFFER_SIZE; j++)
      {
        uint8_t value = i2c_buffer_read[i + j];
        printf("%c", (value >= 32 && value <= 126) ? value : '.');
      }
      printf("\r\n");
    }

    return I2C_FAIL;
  }
}
#endif

void i2c_eeprom_init(void)
{
  eeprom_address = EEPROM_BLOCK0_ADDRESS;
}

uint8_t eeprom_wait_ready(void)
{
  uint32_t timeout = I2C_WRITE_TIMEOUT_CYCLES;
  uint8_t result = I2C_OK;
  // 最大尝试次数
  uint8_t try_count = 10;

  while (try_count--)
  {
    // 检查总线忙状态
    uint32_t busy_timeout = I2C_WRITE_TIMEOUT_CYCLES / 10;
    while ((I2C_STAT(I2CX) & I2C_STAT_I2CBSY) && busy_timeout--)
    {
      __NOP();
    }
    if (busy_timeout == 0)
    {
      // 总线长期忙，复位总线
      i2c_bus_reset();
      continue;
    }

    // 发送起始条件
    I2C_CTL1(I2CX) |= I2C_CTL1_START;

    // 等待地址响应或错误标志（带超时）
    uint32_t flag_timeout = I2C_WRITE_TIMEOUT_CYCLES / 10;
    while (!(I2C_STAT(I2CX) & (I2C_STAT_ADDSEND | I2C_STAT_NACK | I2C_STAT_BERR)))
    {
      if (flag_timeout-- == 0)
        break;
    }

    if (flag_timeout == 0)
    {
      // 超时后重试
      i2c_stop_on_bus(I2CX);
      continue;
    }

    // 检测设备就绪（ADDSEND）
    if (I2C_STAT(I2CX) & I2C_STAT_ADDSEND)
    {
      I2C_STATC(I2CX) = I2C_STATC_ADDSENDC; // 清除地址标志
      i2c_stop_on_bus(I2CX);                // 直接发送停止条件

      // 等待停止条件完成
      uint32_t stop_timeout = I2C_WRITE_TIMEOUT_CYCLES / 10;
      while (!(I2C_STAT(I2CX) & I2C_STAT_STPDET) && stop_timeout--)
      {
        __NOP();
      }

      if (stop_timeout == 0)
      {
        // 停止条件超时，尝试总线复位
        i2c_bus_reset();
        continue;
      }
      // 清除停止标志
      I2C_STATC(I2CX) = I2C_STATC_STPDETC;
      result = I2C_OK;
      break; // 成功退出
    }

    // 错误处理
    if (I2C_STAT(I2CX) & (I2C_STAT_NACK | I2C_STAT_BERR))
    {
      // 清除错误标志
      if (I2C_STAT(I2CX) & I2C_STAT_NACK)
        I2C_STATC(I2CX) |= I2C_STATC_NACKC;
      if (I2C_STAT(I2CX) & I2C_STAT_BERR)
        I2C_STATC(I2CX) |= I2C_STATC_BERRC;
      // 确保总线释放
      i2c_stop_on_bus(I2CX);
    }
  }

  if (try_count == 0)
  {
    i2c_bus_reset();
    result = I2C_FAIL;
  }

  return result;
}

void eeprom_buffer_write(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte)
{
  uint16_t address_offset = write_address % I2C_PAGE_SIZE;
  uint16_t first_page_bytes = I2C_PAGE_SIZE - address_offset;

  if (number_of_byte <= first_page_bytes)
  {
    // 单页写入
    eeprom_page_write(p_buffer, write_address, number_of_byte);
    // delay_1ms(5);
    eeprom_wait_ready();
  }
  else
  {
    // 多页写入
    if (address_offset > 0)
    {
      // 写入首部不完整页
      eeprom_page_write(p_buffer, write_address, first_page_bytes);
      eeprom_wait_ready();
      p_buffer += first_page_bytes;
      write_address += first_page_bytes;
      number_of_byte -= first_page_bytes;
    }

    // 写入完整页
    while (number_of_byte >= I2C_PAGE_SIZE)
    {
      eeprom_page_write(p_buffer, write_address, I2C_PAGE_SIZE);
      eeprom_wait_ready();
      p_buffer += I2C_PAGE_SIZE;
      write_address += I2C_PAGE_SIZE;
      number_of_byte -= I2C_PAGE_SIZE;
    }

    // 写入尾部剩余字节
    if (number_of_byte > 0)
    {
      eeprom_page_write(p_buffer, write_address, number_of_byte);
      eeprom_wait_ready();
    }
  }
}

void eeprom_page_write(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte)
{
  i2c_process_enum write_state = I2C_START;
  uint16_t timeout = 0;
  uint8_t end_flag = 0;

  // 构造地址+数据的DMA缓冲区 (2字节地址)
  tx_buffer[0] = (write_address >> 8) & 0xFF; // 高字节地址
  tx_buffer[1] = write_address & 0xFF;        // 低字节地址
  memcpy(&tx_buffer[2], p_buffer, number_of_byte);

  dma_parameter_struct dma_init_struct;

  while (!end_flag)
  {
    switch (write_state)
    {
    case I2C_START:
      timeout = 0;
      i2c_master_addressing(I2CX, eeprom_address, I2C_MASTER_TRANSMIT);
      // 增加地址字节
      i2c_transfer_byte_number_config(I2CX, number_of_byte + 2);
      i2c_automatic_end_enable(I2CX);
      I2C_STAT(I2CX) |= I2C_STAT_TBE;

      // DMA配置
      i2c_dma_tx_config((uint32_t)tx_buffer, number_of_byte + 2);

      while (i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }

      if (timeout < I2C_TIME_OUT)
      {
        i2c_start_on_bus(I2CX);
        timeout = 0;
        write_state = I2C_TRANSMIT_DATA;
      }
      else
      {
        i2c_bus_reset();
        timeout = 0;
        write_state = I2C_START;
      }
      break;

    case I2C_TRANSMIT_DATA:
      timeout = 0;
      i2c_dma_tx_complete = RESET;
      while ((i2c_dma_tx_complete == RESET) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }

      if (timeout < I2C_TIME_OUT)
      {
        timeout = 0;
        write_state = I2C_STOP;
      }
      else
      {
        timeout = 0;
        write_state = I2C_START;
      }
      break;

    case I2C_STOP:
      timeout = 0;
      while ((!i2c_flag_get(I2CX, I2C_FLAG_STPDET)) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }

      if (timeout < I2C_TIME_OUT)
      {
        i2c_flag_clear(I2CX, I2C_FLAG_STPDET);
        end_flag = 1;
      }
      else
      {
        timeout = 0;
        write_state = I2C_START;
      }
      break;

    default:
      end_flag = 1;
      break;
    }
  }
}

/*!
    \brief      Write a single byte to EEPROM
    \param[in]  data: data to write
    \param[in]  write_address: address to write to
    \param[out] none
    \retval     I2C_OK or I2C_FAIL
*/
uint8_t eeprom_byte_write(uint8_t data, uint16_t write_address)
{
    i2c_process_enum write_state = I2C_START;
    uint16_t timeout = 0;
    uint8_t end_flag = 0;
    uint8_t result = I2C_OK;

    // 初始化传输缓冲区（地址+数据）
    tx_buffer[0] = (write_address >> 8) & 0xFF; // 地址高字节
    tx_buffer[1] = write_address & 0xFF;        // 地址低字节
    tx_buffer[2] = data;                        // 写入的数据

    while (!end_flag)
    {
        switch (write_state)
        {
        case I2C_START:
            // 发送起始信号和设备地址（写模式）
            i2c_master_addressing(I2CX, eeprom_address, I2C_MASTER_TRANSMIT);
            i2c_transfer_byte_number_config(I2CX, 3); // 总字节数：2字节地址 + 1字节数据
            i2c_automatic_end_enable(I2CX);           // 启用自动停止

            // 等待总线空闲
            timeout = 0;
            while (i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }

            if (timeout < I2C_TIME_OUT) {
                i2c_start_on_bus(I2CX);
                write_state = I2C_TRANSMIT_DATA;
            } else {
                i2c_bus_reset();
                result = I2C_FAIL;
                end_flag = 1;
            }
            break;

        case I2C_TRANSMIT_DATA:
            // 使用DMA传输数据（3字节）
            i2c_dma_tx_complete = RESET;
            i2c_dma_tx_config((uint32_t)tx_buffer, 3);

            // 等待DMA传输完成
            timeout = 0;
            while ((i2c_dma_tx_complete == RESET) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }

            if (timeout >= I2C_TIME_OUT) {
                result = I2C_FAIL;
                end_flag = 1;
            } else {
                write_state = I2C_STOP;
            }
            break;

        case I2C_STOP:
            // 等待停止条件产生
            timeout = 0;
            while (!i2c_flag_get(I2CX, I2C_FLAG_STPDET) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }

            if (timeout < I2C_TIME_OUT) {
                i2c_flag_clear(I2CX, I2C_FLAG_STPDET);
                end_flag = 1;
                result = I2C_OK;
            } else {
                i2c_bus_reset();
                result = I2C_FAIL;
                end_flag = 1;
            }
            break;

        default:
            end_flag = 1;
            result = I2C_FAIL;
            break;
        }
    }

    return result;
}

void eeprom_buffer_read(uint8_t *p_buffer, uint16_t read_address, uint16_t number_of_byte)
{
  i2c_process_enum state = I2C_START;
  uint16_t timeout = 0;
  uint8_t end_flag = 0;
  uint8_t restart_flag = 0;
  uint8_t first_reload_flag = 1;
  uint32_t nbytes_reload = 0;
  uint16_t total_bytes = number_of_byte; // 保存总字节数
  uint16_t bytes_read = 0;               // 已读取字节数计数
  uint16_t remaining = 0;

  dma_parameter_struct dma_init_struct;

  while (!end_flag)
  {
    switch (state)
    {
    case I2C_START:
      if (!restart_flag)
      {
        I2C_STAT(I2CX) |= I2C_STAT_TBE;
        i2c_master_addressing(I2CX, eeprom_address, I2C_MASTER_TRANSMIT);
        i2c_transfer_byte_number_config(I2CX, 2); // 24C642字节地址
        i2c_automatic_end_disable(I2CX);

        while (i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT))
        {
          timeout++;
        }

        if (timeout < I2C_TIME_OUT)
        {
          i2c_start_on_bus(I2CX);
          timeout = 0;
          state = I2C_SEND_ADDRESS;
        }
        else
        {
          i2c_bus_reset();
          timeout = 0;
          state = I2C_START;
        }
      }
      else
      {
        i2c_start_on_bus(I2CX);
        restart_flag = 0;
        state = I2C_TRANSMIT_DATA;
      }
      break;

    case I2C_SEND_ADDRESS:
      // 发送地址高字节
      while ((!i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }
      if (timeout < I2C_TIME_OUT)
      {
        i2c_data_transmit(I2CX, (read_address >> 8) & 0xFF);
        timeout = 0;
      }

      // 发送地址低字节
      while ((!i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }
      if (timeout < I2C_TIME_OUT)
      {
        i2c_data_transmit(I2CX, read_address & 0xFF);
        timeout = 0;
        state = I2C_RESTART;
      }
      else
      {
        timeout = 0;
        state = I2C_START;
      }
      break;

    case I2C_RESTART:
      while ((!i2c_flag_get(I2CX, I2C_FLAG_TC)) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }

      if (timeout < I2C_TIME_OUT)
      {
        i2c_master_addressing(I2CX, eeprom_address | 0x01, I2C_MASTER_RECEIVE); // 读模式
        i2c_reload_enable(I2CX);
        timeout = 0;
        state = I2C_RELOAD;
        restart_flag = 1;
      }
      else
      {
        timeout = 0;
        state = I2C_START;
      }
      break;

    case I2C_RELOAD:
      // 分段逻辑：使用bytes_read来跟踪剩余字节
      remaining = total_bytes - bytes_read;
      if (remaining > MAX_RELOAD_SIZE)
      {
        nbytes_reload = MAX_RELOAD_SIZE;
      }
      else
      {
        nbytes_reload = remaining;
        // 如果是第一次传输，禁用重载并启用自动结束
        if (first_reload_flag)
        {
          i2c_reload_disable(I2CX);
          i2c_automatic_end_enable(I2CX);
        }
      }

      if (first_reload_flag)
      {
        i2c_transfer_byte_number_config(I2CX, nbytes_reload);
        first_reload_flag = 0;
        // 配置DMA
        i2c_dma_rx_config((uint32_t)(p_buffer + bytes_read), nbytes_reload);

        state = I2C_START;
      }
      else
      {
        while ((!i2c_flag_get(I2CX, I2C_FLAG_TCR)) && (timeout < I2C_TIME_OUT))
        {
          timeout++;
        }

        if (timeout < I2C_TIME_OUT)
        {
          // 更新缓冲区指针 - 使用bytes_read而不是直接操作p_buffer
          // p_buffer += nbytes_reload;
          i2c_transfer_byte_number_config(I2CX, nbytes_reload);

          if (remaining <= MAX_RELOAD_SIZE)
          {
            i2c_reload_disable(I2CX);
            i2c_automatic_end_enable(I2CX);
          }

          timeout = 0;
          // 重载时重新配置DMA
          i2c_dma_rx_config((uint32_t)(p_buffer + bytes_read), nbytes_reload);
          state = I2C_TRANSMIT_DATA;
        }
        else
        {
          timeout = 0;
          state = I2C_START;
        }
      }
      break;

    case I2C_TRANSMIT_DATA:
      i2c_dma_rx_complete = RESET;

      timeout = 0;
      while ((i2c_dma_rx_complete == RESET) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }

      if (timeout < I2C_TIME_OUT)
      {
        // 更新已读取字节数
        bytes_read += nbytes_reload;

        if (I2C_CTL1(I2CX) & I2C_CTL1_RELOAD)
        {
          timeout = 0;
          state = I2C_RELOAD;
        }
        else
        {
          timeout = 0;
          state = I2C_STOP;
        }
      }
      else
      {
        timeout = 0;
        state = I2C_START;
      }
      break;

    case I2C_STOP:
      while ((!i2c_flag_get(I2CX, I2C_FLAG_STPDET)) && (timeout < I2C_TIME_OUT))
      {
        timeout++;
      }

      if (timeout < I2C_TIME_OUT)
      {
        i2c_flag_clear(I2CX, I2C_FLAG_STPDET);
        end_flag = 1;
      }
      else
      {
        timeout = 0;
        state = I2C_START;
      }
      break;

    default:
      end_flag = 1;
      break;
    }
  }
}

void i2c_dma_tx_callback(void)
{
  i2c_dma_tx_complete = SET;
}

void i2c_dma_rx_callback(void)
{
  i2c_dma_rx_complete = SET;
}
