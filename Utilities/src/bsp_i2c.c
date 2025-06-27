/*!
    \file    bsp_i2c.c
    \brief   firmware functions to manage I2C

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp_i2c.h"

#include <stdio.h>
#include "gd32g5x3.h"

void bsp_i2c_init(void)
{
    /* 1. ʱ��ʹ�� */
    rcu_periph_clock_enable(RCU_DMA0);     // DMAʱ��
    rcu_periph_clock_enable(RCU_DMAMUX);   // DMAMUXʱ��
    rcu_periph_clock_enable(RCU_GPIO_I2C); // GPIOʱ��
    rcu_periph_clock_enable(RCU_I2C);      // I2C����ʱ��

    /* 2. GPIO���� */
    // SCL��������
    gpio_af_set(I2C_SCL_PORT, I2C_GPIO_AF, I2C_SCL_PIN);
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SCL_PIN);

    // SDA��������
    gpio_af_set(I2C_SDA_PORT, I2C_GPIO_AF, I2C_SDA_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SDA_PIN);

    /* 3. I2C�������� */
    // ʱ������ (��׼ģʽ400kHz)
    i2c_timing_config(I2CX, 0x1, 0xA, 0);      // ����SCL�ߵ͵�ƽʱ��
    i2c_master_clock_config(I2CX, 0x73, 0x6E); // ������ʱ��

    // ʹ��I2C����
    i2c_enable(I2CX);

    // ��ʼ��eeprom
    i2c_eeprom_init();
}

void i2c_gpio_reset(void)
{
    /* ��λI2C_SCL_PIN��I2C_SDA_PIN */
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, I2C_SCL_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, I2C_SDA_PIN);
}

void i2c_bus_reset()
{
    /* ��SDA/SCL����ΪGPIOģʽ */
    GPIO_BC(I2C_SCL_PORT) |= I2C_SCL_PIN;
    GPIO_BC(I2C_SDA_PORT) |= I2C_SDA_PIN;
    /* ��λI2C_SCL_PIN��I2C_SDA_PIN */
    i2c_gpio_reset();
    // ����ղ���ȷ��ʱ��
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    /* ����ֹͣ�ź� */
    GPIO_BOP(I2C_SCL_PORT) |= I2C_SCL_PIN;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIO_BOP(I2C_SDA_PORT) |= I2C_SDA_PIN;
    /* ��������I2C_SCL_PIN��I2C_SCL */
    /* ��������I2C_SDA_PIN��I2C_SDA */
    gpio_mode_set(I2C_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SCL_PIN);
    gpio_output_options_set(I2C_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SCL_PIN);
    gpio_mode_set(I2C_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C_SDA_PIN);
    gpio_output_options_set(I2C_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_60MHZ, I2C_SDA_PIN);
}

void i2c_dma_tx_config(uint32_t mem_addr, uint32_t size)
{
    dma_parameter_struct dma_init_struct; // DMA���ýṹ��

    /* ��ʼ��DMAͨ�� */
    dma_deinit(I2C_DMAx, I2C_DMA_TX_CH);    // ��λDMAͨ���Ĵ���
    dma_struct_para_init(&dma_init_struct); // ��ʼ���ṹ��ΪĬ��ֵ

    /* ����DMA������� */
    dma_init_struct.request = I2C_DMA_TX_REQUEST;             // DMA����Դѡ��(I2C����)
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;     // ���䷽���ڴ������
    dma_init_struct.memory_addr = mem_addr;                   // �ڴ�Դ��ַ
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;     // �ڴ����ݿ��8λ(ƥ��I2C)
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;  // �ڴ��ַ����(�������ݿ�)
    dma_init_struct.number = size;                            // ����������
    dma_init_struct.periph_addr = (uint32_t)&I2C_TDATA(I2CX); // ����Ŀ���ַ(I2C���ݼĴ���)
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT; // �������ݿ��8λ
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE; // �����ַ�̶�(ʼ��д��ͬһ�Ĵ���)
    dma_init_struct.priority = DMA_PRIORITY_HIGH;             // �ߴ������ȼ�
    dma_init(I2C_DMAx, I2C_DMA_TX_CH, &dma_init_struct);      // Ӧ������

    /* �߼�ģʽ���� */
    dma_circulation_disable(I2C_DMAx, I2C_DMA_TX_CH);  // ����ѭ��ģʽ(���δ���)
    dmamux_synchronization_disable(I2C_DMA_MUX_TX_CH); // ����DMAMUXͬ������

    /* ʹ��DMAͨ�� */
    dma_channel_enable(I2C_DMAx, I2C_DMA_TX_CH); //

    /* ʹ��DMA��������ж� */
    dma_interrupt_enable(I2C_DMAx, I2C_DMA_TX_CH, DMA_INT_FTF);

    /* ����DMA���� */
    i2c_dma_enable(I2CX, I2C_DMA_TRANSMIT);
}

void i2c_dma_rx_config(uint32_t mem_addr, uint32_t size)
{
    dma_parameter_struct dma_init_struct;

    /* ��λDMAͨ�� */
    dma_deinit(I2C_DMAx, I2C_DMA_RX_CH);    // ��λDMAͨ���Ĵ���
    dma_struct_para_init(&dma_init_struct); // ��ʼ���ṹ��

    /* ����DMA������� */
    dma_init_struct.request = I2C_DMA_RX_REQUEST;             // DMA����Դѡ��(I2C����)
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;     // ���䷽��������ڴ�
    dma_init_struct.memory_addr = mem_addr;                   // �ڴ�Ŀ���ַ
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;     // �ڴ����ݿ��8λ
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;  // �ڴ��ַ����
    dma_init_struct.number = size;                            // ����������
    dma_init_struct.periph_addr = (uint32_t)&I2C_RDATA(I2CX); // ����Դ��ַ(I2C���ݼĴ���)
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT; // �������ݿ��8λ
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE; // �����ַ�̶�
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;       // �������ȼ�(ȷ������ʵʱ��)
    dma_init(I2C_DMAx, I2C_DMA_RX_CH, &dma_init_struct);      // Ӧ������

    /* �߼�ģʽ���� */
    dma_circulation_disable(I2C_DMAx, I2C_DMA_RX_CH);  // ����ѭ��ģʽ
    dmamux_synchronization_disable(I2C_DMA_MUX_RX_CH); // ����DMAMUXͬ��

    /* ����DMAͨ�� */
    dma_channel_enable(I2C_DMAx, I2C_DMA_RX_CH); // ����DMAͨ��

    /* ʹ��DMA��������ж� */
    dma_interrupt_enable(I2C_DMAx, I2C_DMA_RX_CH, DMA_INT_FTF);

    /* ����DMA���� */
    i2c_dma_enable(I2CX, I2C_DMA_RECEIVE);
}
