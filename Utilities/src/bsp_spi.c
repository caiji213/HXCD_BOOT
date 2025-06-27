/*!
    \file    bsp_spi.c
    \brief   firmware functions to manage SPI

    \version 2025-06-05, V1.1.0, firmware for GD32G5x3
*/

#include "bsp_spi.h"

uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];

__IO FlagStatus spi_rx_complete_flag = RESET;

void bsp_spi_rcu_config(void)
{
    rcu_periph_clock_enable(SPI2_GPIO_CLK); // ʹ��GPIOCʱ��
    rcu_periph_clock_enable(SPI2_DMA_CLK);  // ʹ��DMA1ʱ��
    rcu_periph_clock_enable(RCU_DMAMUX);    // ʹ��DMAMUXʱ��
    rcu_periph_clock_enable(SPI2_CLK);      // ʹ��SPI2ʱ��
}

void bsp_spi_gpio_config(void)
{
    /* SPI2���Ÿ������� */
    gpio_af_set(SPI2_SCK_PORT, GPIO_AF_6, SPI2_SCK_PIN);
    gpio_af_set(SPI2_MISO_PORT, GPIO_AF_6, SPI2_MISO_PIN);

    /* SCK���� */
    gpio_mode_set(SPI2_SCK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_SCK_PIN);
    gpio_output_options_set(SPI2_SCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, SPI2_SCK_PIN);

    /* MISO���� */
    gpio_mode_set(SPI2_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_MISO_PIN);
    gpio_output_options_set(SPI2_MISO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, SPI2_MISO_PIN);
}

void bsp_spi_periph_config(void)
{
    spi_parameter_struct spi_init_struct;
    spi_struct_para_init(&spi_init_struct);

    /* ����SPI���� */
    spi_init_struct.device_mode = SPI_MASTER;                       // ����ģʽ
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;          // ȫ˫��ģʽ
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;                // 8λ����֡
    spi_init_struct.nss = SPI_NSS_SOFT;                             // �������NSS
    spi_init_struct.endian = SPI_ENDIAN_MSB;                        // MSB����
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE; // ����
    spi_init_struct.prescale = SPI_PSC_128;                         // ��Ƶ

    spi_init(SPI2_PERIPH, &spi_init_struct);

    // ע�⣺����ʹ�� SPI_BYTE_ACCESS����ֹSPI��8bit֡�����������ݺϲ�ģʽ���DMA��λ
    spi_fifo_access_size_config(SPI2_PERIPH, SPI_BYTE_ACCESS);

    spi_enable(SPI2_PERIPH);
}

void bsp_spi_dma_init(void)
{
    dma_parameter_struct dma_init_struct;

    /********** ���� SPI2 TX�����ͣ�DMA��DMA1 ͨ��1 **********/
    // ��λ DMA ����ͨ��
    dma_deinit(SPI2_DMA, SPI2_TX_DMA_CH);

    // ��ʼ���ṹ��ΪĬ��ֵ
    dma_struct_para_init(&dma_init_struct);

    // ���� DMA ���Ͳ���
    dma_init_struct.request = SPI2_TX_DMA_REQUEST;                  // DMA ����Դ
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;           // ���ڴ浽����
    dma_init_struct.memory_addr = (uint32_t)spi_tx_buffer;          // Դ��ַ�����ͻ�����
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;        // �����ڴ��ַ����
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;           // �ڴ��ȣ�8λ
    dma_init_struct.number = SPI_BUFFER_SIZE;                       // ������������
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2_PERIPH); // Ŀ���ַ��SPI2 ���ݼĴ���
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;       // ��ֹ�����ַ����
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;       // �����ȣ�8λ
    dma_init_struct.priority = DMA_PRIORITY_HIGH;                   // ���ȼ�����

    // ��ʼ�� DMA ͨ��
    dma_init(SPI2_DMA, SPI2_TX_DMA_CH, &dma_init_struct);

    // �����ڴ浽�ڴ�ģʽ
    dma_memory_to_memory_disable(SPI2_DMA, SPI2_TX_DMA_CH);

    // ����ѭ��ģʽ
    dma_circulation_disable(SPI2_DMA, SPI2_TX_DMA_CH);

    // ʹ�ܴ�������ж�
    dma_interrupt_enable(SPI2_DMA, SPI2_TX_DMA_CH, DMA_INT_FTF);

    // ���� DMAMUX ͬ��ģʽ��ͨ��1��
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH1);

    /********** ���� SPI2 RX�����գ�DMA��DMA1 ͨ��0 **********/
    // ��λ DMA ����ͨ��
    dma_deinit(SPI2_DMA, SPI2_RX_DMA_CH);

    // ��ʼ���ṹ��ΪĬ��ֵ
    dma_struct_para_init(&dma_init_struct);

    // ���� DMA ���ղ���
    dma_init_struct.request = SPI2_RX_DMA_REQUEST;                  // DMA ����Դ
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;           // �����赽�ڴ�
    dma_init_struct.memory_addr = (uint32_t)spi_rx_buffer;          // Ŀ���ַ�����ջ�����
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;        // �����ڴ��ַ����
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;           // �ڴ��ȣ�8λ
    dma_init_struct.number = SPI_BUFFER_SIZE;                       // ������������
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2_PERIPH); // Դ��ַ��SPI2 ���ݼĴ���
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;       // ��ֹ�����ַ����
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;       // �����ȣ�8λ
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;             // ���ȼ�������

    // ��ʼ�� DMA ͨ��
    dma_init(SPI2_DMA, SPI2_RX_DMA_CH, &dma_init_struct);

    // �����ڴ浽�ڴ�ģʽ
    dma_memory_to_memory_disable(SPI2_DMA, SPI2_RX_DMA_CH);

    // ����ѭ��ģʽ
    dma_circulation_disable(SPI2_DMA, SPI2_RX_DMA_CH);

    // ʹ�ܴ�������ж�
    dma_interrupt_enable(SPI2_DMA, SPI2_RX_DMA_CH, DMA_INT_FTF);

    // ���� DMAMUX ͬ��ģʽ��ͨ��0��
    dmamux_synchronization_disable(DMAMUX_MULTIPLEXER_CH0);

    /********** ���� SPI2 �� DMA ���� **********/
    spi_dma_enable(SPI2_PERIPH, SPI_DMA_TRANSMIT); // ���� SPI2 TX DMA
    spi_dma_enable(SPI2_PERIPH, SPI_DMA_RECEIVE);  // ���� SPI2 RX DMA
}

void bsp_spi_init(void)
{
    bsp_spi_rcu_config();    // ����ʱ��
    bsp_spi_gpio_config();   // ����GPIO
    bsp_spi_periph_config(); // ����SPI����
    bsp_spi_dma_init();      // ����DMA
}

void bsp_spi_transfer(uint32_t size)
{
    /* ȷ�����䳤�Ȳ�������������С */
    if (size > SPI_BUFFER_SIZE)
    {
        size = SPI_BUFFER_SIZE;
    }

    /* ��ʼ��������Ϊ0x00 */
    for (uint32_t i = 0; i < size; i++)
    {
        spi_tx_buffer[i] = 0x00;
        spi_rx_buffer[i] = 0x00;
    }

    /* ����DMA���䳤�� */
    dma_transfer_number_config(SPI2_DMA, SPI2_TX_DMA_CH, size);
    dma_transfer_number_config(SPI2_DMA, SPI2_RX_DMA_CH, size);

    /* ����DMA���� - �������������������� */
    dma_channel_enable(SPI2_DMA, SPI2_RX_DMA_CH);
    dma_channel_enable(SPI2_DMA, SPI2_TX_DMA_CH);

    // �ȴ��������
    while (spi_rx_complete_flag == RESET)
        ;

    /* ���������ɱ�־ */
    spi_rx_complete_flag = RESET;
}

// ���ݴ�����
void process_spi_data(uint8_t *data, uint32_t size)
{
    //    // ��ӡ����ͷ
    //    printf("Received %u bytes of data:\n", size);
    //
    //    // ���д�ӡ���ݣ�ÿ��16�ֽڣ�
    //    for (uint32_t i = 0; i < size; i++) {
    //        // ÿ16�ֽڴ�ӡһ��
    //        if (i % 16 == 0) {
    //            // ��ӡ��ǰ�к�
    //            if (i > 0) printf("\n");
    //            printf("%04X: ", i);
    //        }
    //
    //        // ��ӡ��ǰ�ֽڵ�ʮ������ֵ
    //        printf("%02X ", data[i]);
    //    }
    //
    //    // ��ӡ������
    //    printf("\n\n");
}