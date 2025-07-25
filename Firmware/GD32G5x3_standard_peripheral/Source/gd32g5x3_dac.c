/*!
    \file    gd32g5x3_dac.c
    \brief   DAC driver

    \version 2025-02-10, V1.1.0, firmware for GD32G5x3
*/

/*
    Copyright (c) 2025, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32g5x3_dac.h"

/* DAC register bit offset */
#define OUT1_REG_OFFSET           ((uint32_t)0x00000010U)
#define DH_12BIT_OFFSET           ((uint32_t)0x00000010U)
#define DH_8BIT_OFFSET            ((uint32_t)0x00000008U)

#define DAC_STAT_FLAG_MASK0       (DAC_FLAG_DDUDR0 | DAC_FLAG_CALF0 | DAC_FLAG_BWT0 | DAC_FLAG_DDUDR1 | DAC_FLAG_CALF1 | DAC_FLAG_BWT1)
#define DAC_INT_EN_MASK0          (DAC_INT_DDUDR0 | DAC_INT_DDUDR1)
#define DAC_INT_FLAG_MASK0        (DAC_INT_FLAG_DDUDR0 | DAC_INT_FLAG_DDUDR1)

/*!
    \brief      deinitialize DAC
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[out] none
    \retval     none
*/
void dac_deinit(uint32_t dac_periph)
{
    switch(dac_periph){
        case DAC0:
            /* reset DAC0 */
            rcu_periph_reset_enable(RCU_DAC0RST);
            rcu_periph_reset_disable(RCU_DAC0RST);
            break;
        case DAC1:
            /* reset DAC1 */
            rcu_periph_reset_enable(RCU_DAC1RST);
            rcu_periph_reset_disable(RCU_DAC1RST);
            break;
        case DAC2:
            /* reset DAC2 */
            rcu_periph_reset_enable(RCU_DAC2RST);
            rcu_periph_reset_disable(RCU_DAC2RST);
            break;
        case DAC3:
            /* reset DAC3 */
            rcu_periph_reset_enable(RCU_DAC3RST);
            rcu_periph_reset_disable(RCU_DAC3RST);
            break;
        default:
            break;
    }
}

/*!
    \brief      enable DAC
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DEN0;
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DEN1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      disable DAC
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_disable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DEN0);
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DEN1);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable DAC DMA function
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_dma_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DDMAEN0;
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DDMAEN1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      disable DAC DMA function
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_dma_disable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DDMAEN0);
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DDMAEN1);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  mode: DAC working mode
                only one parameter can be selected which is shown as below:
      \arg        NORMAL_PIN_BUFFON: DAC_OUT_x work in normal mode and connect to external pin with buffer enable
      \arg        NORMAL_PIN_PERIPH_BUFFON: DAC_OUT_x work in normal mode and connect to external pin and on chip peripherals with buffer enable
      \arg        NORMAL_PIN_BUFFOFF: DAC_OUT_x work in normal mode and connect to external pin with buffer disable
      \arg        NORMAL_PERIPH_BUFFOFF: DAC_OUT_x work in normal mode and connect to on chip peripherals with buffer disable
      \arg        SAMPLEKEEP_PIN_BUFFON: DAC_OUT_x work in sample and keep mode and connect to external pin with buffer enable
      \arg        SAMPLEKEEP_PIN_PERIPH_BUFFON: DAC_OUT_x work in sample and keep mode and connect to external pin and on chip peripherals with buffer enable
      \arg        SAMPLEKEEP_PIN_BUFFOFF: DAC_OUT_x work in sample and keep mode and connect to external pin and on chip peripherals with buffer enable
      \arg        SAMPLEKEEP_PERIPH_BUFFOFF: DAC_OUT_x work in sample and keep mode and connect to on chip peripherals with buffer disable
    \param[out] none
    \retval     none
*/
void dac_mode_config(uint32_t dac_periph, uint8_t dac_out, uint32_t mode)
{
    if(DAC_OUT0 == dac_out){
        /* configure DAC0 mode */
        DAC_MDCR(dac_periph) &= ~(uint32_t)DAC_MDCR_MODE0;
        DAC_MDCR(dac_periph) |= mode;
    }else if(DAC_OUT1 == dac_out){
        /* configure DAC1 mode */
        DAC_MDCR(dac_periph) &= ~(uint32_t)DAC_MDCR_MODE1;
        DAC_MDCR(dac_periph) |= (mode << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      get the DACx trimming value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     DACx trimming value
*/
uint32_t dac_trimming_value_get(uint32_t dac_periph, uint8_t dac_out)
{
    uint32_t tmp = 0U;
    if(DAC_OUT0 == dac_out) {
        /* get the DAC_OUT_0 trimming value */
        tmp = DAC_CALR(dac_periph) & DAC_CALR_OTV0;
    } else if(DAC_OUT1 == dac_out) {
        /* get the DAC_OUT_1 trimming value */
        tmp = (DAC_CALR(dac_periph) & DAC_CALR_OTV1) >> OUT1_REG_OFFSET;
    } else {
        /* illegal parameters */
    }

    return tmp;
}

/*!
    \brief      set the DACx trimming value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  trim_value: set new DAC trimming value(0x0 ~ 0x1F)
    \param[out] none
*/
void dac_trimming_value_set(uint32_t dac_periph, uint8_t dac_out, uint32_t trim_value)
{
    uint32_t tmp = 0U;
    
    /* get the trimming value */
    tmp = DAC_CALR(dac_periph);

    if(DAC_OUT0 == dac_out) {
        /* set the DACx_OUT0 trimming value */
        tmp &= ~(uint32_t)DAC_CALR_OTV0;
        tmp |= (trim_value & DAC_CALR_OTV0);
        DAC_CALR(dac_periph) = tmp;
    }else if(DAC_OUT1 == dac_out){
        /* set the DACx_OUT1 trimming value */
        tmp &= ~(uint32_t)DAC_CALR_OTV1;
        tmp |= ((trim_value << OUT1_REG_OFFSET) & DAC_CALR_OTV1);
        DAC_CALR(dac_periph) = tmp;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable the DACx trimming
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
*/
void dac_trimming_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        /* enable the DACx_OUT0 trimming */
        DAC_CTL0(dac_periph) |= DAC_CTL0_CALEN0;
    }else if(DAC_OUT1 == dac_out){
        /* enable the DACx_OUT1 trimming */
        DAC_CTL0(dac_periph) |= DAC_CTL0_CALEN1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      disable the DACx trimming
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
*/
void dac_trimming_disable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        /* disable the DACx_OUT0 trimming */
        DAC_CTL0(dac_periph) &= ~DAC_CTL0_CALEN0;
    }else if(DAC_OUT1 == dac_out){
        /* disable the DACx_OUT1 trimming */
        DAC_CTL0(dac_periph) &= ~DAC_CTL0_CALEN1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      get DAC output value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     DAC output data: 0~4095
*/
uint16_t dac_output_value_get(uint32_t dac_periph, uint8_t dac_out)
{
    uint16_t data = 0U;

    if(DAC_OUT0 == dac_out){
        /* store the DACx_OUT0 output value */
        data = (uint16_t)DAC_OUT0_DO(dac_periph);
    }else if(DAC_OUT1 == dac_out){
        /* store the DACx_OUT1 output value */
        data = (uint16_t)DAC_OUT1_DO(dac_periph);
    }else{
        /* illegal parameters */
    }

    return data;
}

/*!
    \brief      config DAC input data format
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  format: data format written in data holding register
                only one parameter can be selected which is shown as below:
      \arg        DAC_DATA_FORMAT_UNSIGNED: input data is int unsigned format
      \arg        DAC_DATA_FORMAT_SIGNED: input data is int signed format
    \param[out] none
    \retval     none
*/
void dac_data_format_config(uint32_t dac_periph, uint8_t dac_out, uint32_t format)
{
    if(DAC_OUT0 == dac_out){
        if(DAC_DATA_FORMAT_UNSIGNED == format){
            DAC_MDCR(dac_periph) &= ~DAC_MDCR_DHFMT0;
        }else if(DAC_DATA_FORMAT_SIGNED == format){
            DAC_MDCR(dac_periph) |= DAC_MDCR_DHFMT0;
        }else{
            /* illegal parameters */
        }
    }else if(DAC_OUT1 == dac_out){
        if(DAC_DATA_FORMAT_UNSIGNED == format){
            DAC_MDCR(dac_periph) &= ~DAC_MDCR_DHFMT1;
        }else if(DAC_DATA_FORMAT_SIGNED == format){
            DAC_MDCR(dac_periph) |= DAC_MDCR_DHFMT1;
        }else{
            /* illegal parameters */
        }
    }else{
    }
}

/*!
    \brief      set DAC data holding register value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  dac_align: DAC data alignment mode
                only one parameter can be selected which is shown as below:
      \arg        DAC_ALIGN_12B_R: 12-bit right-aligned data
      \arg        DAC_ALIGN_12B_L: 12-bit left-aligned data
      \arg        DAC_ALIGN_8B_R: 8-bit right-aligned data
    \param[in]  data: data to be loaded(0~4095)
    \param[out] none
    \retval     none
*/
void dac_data_set(uint32_t dac_periph, uint8_t dac_out, uint32_t dac_align, uint16_t data)
{
    /* DAC_OUT0 data alignment */
    if(DAC_OUT0 == dac_out){
        switch(dac_align){
        /* 12-bit right-aligned data */
        case DAC_ALIGN_12B_R:
            DAC_OUT0_R12DH(dac_periph) = data;
            break;
        /* 12-bit left-aligned data */
        case DAC_ALIGN_12B_L:
            DAC_OUT0_L12DH(dac_periph) = data;
            break;
        /* 8-bit right-aligned data */
        case DAC_ALIGN_8B_R:
            DAC_OUT0_R8DH(dac_periph) = data;
            break;
        default:
            break;
        }
    }else if(DAC_OUT1 == dac_out){
        /* DAC_OUT1 data alignment */
        switch(dac_align){
        /* 12-bit right-aligned data */
        case DAC_ALIGN_12B_R:
            DAC_OUT1_R12DH(dac_periph) = data;
            break;
        /* 12-bit left-aligned data */
        case DAC_ALIGN_12B_L:
            DAC_OUT1_L12DH(dac_periph) = data;
            break;
        /* 8-bit right-aligned data */
        case DAC_ALIGN_8B_R:
            DAC_OUT1_R8DH(dac_periph) = data;
            break;
        default:
            break;
        }
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable DAC trigger
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_trigger_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DTEN0;
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DTEN1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      disable DAC trigger
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_trigger_disable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DTEN0);
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DTEN1);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC trigger source
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  triggersource: external trigger of DAC
                only one parameter can be selected which is shown as below:
      \arg        DAC_TRIGGER_EXTERNAL: external trigger selected by TRIGSEL
      \arg        DAC_TRIGGER_SOFTWARE: software trigger
    \param[out] none
    \retval     none
*/
void dac_trigger_source_config(uint32_t dac_periph, uint8_t dac_out, uint32_t triggersource)
{
    if(DAC_OUT0 == dac_out){
        /* configure DACx_OUT0 trigger source */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DTSEL0);
        DAC_CTL0(dac_periph) |= triggersource;
    }else if(DAC_OUT1 == dac_out){
        /* configure DACx_OUT1 trigger source */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DTSEL1);
        DAC_CTL0(dac_periph) |= (triggersource << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable DAC software trigger
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \retval     none
*/
void dac_software_trigger_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_SWT(dac_periph) |= (uint32_t)DAC_SWT_SWTR0;
    }else if(DAC_OUT1 == dac_out){
        DAC_SWT(dac_periph) |= (uint32_t)DAC_SWT_SWTR1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC wave mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  wave_mode: DAC wave mode
                only one parameter can be selected which is shown as below:
      \arg        DAC_WAVE_DISABLE: wave mode disable
      \arg        DAC_WAVE_MODE_LFSR: LFSR noise mode
      \arg        DAC_WAVE_MODE_TRIANGLE: triangle noise mode
      \arg        DAC_WAVE_MODE_SAWTOOTH: sawtooth wave mode
    \param[out] none
    \retval     none
*/
void dac_wave_mode_config(uint32_t dac_periph, uint8_t dac_out, uint32_t wave_mode)
{
    if(DAC_OUT0 == dac_out){
        /* configure DACx_OUT0 wave mode */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DWM0);
        DAC_CTL0(dac_periph) |= wave_mode;
    }else if(DAC_OUT1 == dac_out){
        /* configure DACx_OUT1 wave mode */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DWM1);
        DAC_CTL0(dac_periph) |= (wave_mode << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC LFSR noise mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  unmask_bits: LFSR noise unmask bits
                only one parameter can be selected which is shown as below:
      \arg        DAC_LFSR_BIT0: unmask the LFSR bit0
      \arg        DAC_LFSR_BITS1_0: unmask the LFSR bits[1:0]
      \arg        DAC_LFSR_BITS2_0: unmask the LFSR bits[2:0]
      \arg        DAC_LFSR_BITS3_0: unmask the LFSR bits[3:0]
      \arg        DAC_LFSR_BITS4_0: unmask the LFSR bits[4:0]
      \arg        DAC_LFSR_BITS5_0: unmask the LFSR bits[5:0]
      \arg        DAC_LFSR_BITS6_0: unmask the LFSR bits[6:0]
      \arg        DAC_LFSR_BITS7_0: unmask the LFSR bits[7:0]
      \arg        DAC_LFSR_BITS8_0: unmask the LFSR bits[8:0]
      \arg        DAC_LFSR_BITS9_0: unmask the LFSR bits[9:0]
      \arg        DAC_LFSR_BITS10_0: unmask the LFSR bits[10:0]
      \arg        DAC_LFSR_BITS11_0: unmask the LFSR bits[11:0]
    \param[out] none
    \retval     none
*/
void dac_lfsr_noise_config(uint32_t dac_periph, uint8_t dac_out, uint32_t unmask_bits)
{
    if(DAC_OUT0 == dac_out){
        /* configure DACx_OUT0 LFSR noise mode */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DWBW0);
        DAC_CTL0(dac_periph) |= unmask_bits;
    }else if(DAC_OUT1 == dac_out){
        /* configure DACx_OUT1 LFSR noise mode */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DWBW1);
        DAC_CTL0(dac_periph) |= (unmask_bits << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC triangle noise mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  amplitude: the amplitude of the triangle
                only one parameter can be selected which is shown as below:
      \arg        DAC_TRIANGLE_AMPLITUDE_1: triangle amplitude is 1
      \arg        DAC_TRIANGLE_AMPLITUDE_3: triangle amplitude is 3
      \arg        DAC_TRIANGLE_AMPLITUDE_7: triangle amplitude is 7
      \arg        DAC_TRIANGLE_AMPLITUDE_15: triangle amplitude is 15
      \arg        DAC_TRIANGLE_AMPLITUDE_31: triangle amplitude is 31
      \arg        DAC_TRIANGLE_AMPLITUDE_63: triangle amplitude is 63
      \arg        DAC_TRIANGLE_AMPLITUDE_127: triangle amplitude is 127
      \arg        DAC_TRIANGLE_AMPLITUDE_255: triangle amplitude is 255
      \arg        DAC_TRIANGLE_AMPLITUDE_511: triangle amplitude is 511
      \arg        DAC_TRIANGLE_AMPLITUDE_1023: triangle amplitude is 1023
      \arg        DAC_TRIANGLE_AMPLITUDE_2047: triangle amplitude is 2047
      \arg        DAC_TRIANGLE_AMPLITUDE_4095: triangle amplitude is 4095
    \param[out] none
    \retval     none
*/
void dac_triangle_noise_config(uint32_t dac_periph, uint8_t dac_out, uint32_t amplitude)
{
    if(DAC_OUT0 == dac_out){
        /* configure DACx_OUT0 triangle noise mode */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DWBW0);
        DAC_CTL0(dac_periph) |= amplitude;
    }else if(DAC_OUT1 == dac_out){
        /* configure DACx_OUT1 triangle noise mode */
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DWBW1);
        DAC_CTL0(dac_periph) |= (amplitude << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC sawtooth reset trigger source
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  triggersource: external triggers of DAC
                only one parameter can be selected which is shown as below:
      \arg        DAC_SAWTOOTH_RESET_TRIGGER_EXTERNAL: external trigger from TRIGSEL
      \arg        DAC_SAWTOOTH_RESET_TRIGGER_SOFTWARE: software trigger
    \param[out] none
    \retval     none
*/
void dac_sawtooth_reset_trigger_source_config(uint32_t dac_periph, uint8_t dac_out, uint32_t triggersource)
{
    if(DAC_OUT0 == dac_out){
        /* configure DAC_OUT0 sawtooth initial trigger source */
        DAC_SAWMDR(dac_periph) &= (uint32_t)(~DAC_SAWMDR_RSTTSEL0);
        DAC_SAWMDR(dac_periph) |= triggersource;
    }else if(DAC_OUT1 == dac_out){
        /* configure DAC_OUT1 sawtooth initial trigger source */
        DAC_SAWMDR(dac_periph) &= (uint32_t)(~DAC_SAWMDR_RSTTSEL1);
        DAC_SAWMDR(dac_periph) |= (triggersource << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC step initial trigger source
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  triggersource: external triggers of DAC
                only one parameter can be selected which is shown as below:
      \arg        DAC_SAWTOOTH_STEP_TRIGGER_EXTERNAL: external trigger from TRIGSEL
      \arg        DAC_SAWTOOTH_STEP_TRIGGER_SOFTWARE: software trigger
    \param[out] none
    \retval     none
*/
void dac_sawtooth_step_trigger_source_config(uint32_t dac_periph, uint8_t dac_out, uint32_t triggersource)
{
    if(DAC_OUT0 == dac_out){
        /* configure DAC_OUT0 sawtooth step trigger source */
        DAC_SAWMDR(dac_periph) &= (uint32_t)(~DAC_SAWMDR_STEPTSEL0);
        DAC_SAWMDR(dac_periph) |= triggersource;
    }else if(DAC_OUT1 == dac_out){
        /* configure DAC_OUT1 sawtooth step trigger source */
        DAC_SAWMDR(dac_periph) &= (uint32_t)(~DAC_SAWMDR_STEPTSEL1);
        DAC_SAWMDR(dac_periph) |= (triggersource << OUT1_REG_OFFSET);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      configure DAC step initial trigger source
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  direction: direction of sawtooth step
                only one parameter can be selected which is shown as below:
      \arg        DAC_SAWTOOTH_STEP_DOWN: sawtooth step down
      \arg        DAC_SAWTOOTH_STEP_UP: sawtooth step up
    \param[out] none
    \retval     none
*/
void dac_sawtooth_step_direction_config(uint32_t dac_periph, uint8_t dac_out, uint32_t direction)
{
    if(DAC_OUT0 == dac_out){
        /* configure DAC_OUT0 sawtooth step direction */
        if(DAC_SAWTOOTH_STEP_DOWN == direction){
            DAC_OUT0_SAW(dac_periph) &= (uint32_t)(~DAC_OUT0_SAWDIR);
        }else if(DAC_SAWTOOTH_STEP_UP == direction){
            DAC_OUT0_SAW(dac_periph) |= (uint32_t)(DAC_OUT0_SAWDIR);
        }else{
        }
    }else if(DAC_OUT1 == dac_out){
        /* configure DAC_OUT1 sawtooth step direction */
        if(DAC_SAWTOOTH_STEP_DOWN == direction){
            DAC_OUT1_SAW(dac_periph) &= (uint32_t)(~DAC_OUT1_SAWDIR);
        }else if(DAC_SAWTOOTH_STEP_UP == direction){
            DAC_OUT1_SAW(dac_periph) |= (uint32_t)(DAC_OUT1_SAWDIR);
        }else{
        }
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      set DAC sawtooth wave initial value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  init_value: sawtooth initial value, 0x0..0xFFF
    \param[out] none
*/
void dac_sawtooth_initial_value_set(uint32_t dac_periph, uint8_t dac_out, uint32_t init_value)
{
    uint32_t tmp = 0U;

    if(DAC_OUT0 == dac_out){
        /* set DAC_OUT0 sawtooth step value */
        tmp = (DAC_OUT0_SAW(dac_periph) & ~(uint32_t)DAC_OUT0_SAWINIT);
        DAC_OUT0_SAW(dac_periph) = tmp | (init_value & DAC_OUT0_SAWINIT);
    }else if(DAC_OUT1 == dac_out){
        /* set DAC_OUT1 sawtooth step value */
        tmp = (DAC_OUT1_SAW(dac_periph) & ~(uint32_t)DAC_OUT1_SAWINIT);
        DAC_OUT1_SAW(dac_periph) = tmp | (init_value & DAC_OUT1_SAWINIT);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      set DAC sawtooth wave step value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  step_value: sawtooth step value, 0x0..0xFFFF
    \param[out] none
*/
void dac_sawtooth_step_value_set(uint32_t dac_periph, uint8_t dac_out, uint32_t step_value)
{
    uint32_t tmp = 0U;

    if(DAC_OUT0 == dac_out){
        /* set DAC_OUT0 sawtooth step value */
        tmp = (DAC_OUT0_SAW(dac_periph) & ~(uint32_t)DAC_OUT0_SAWSTEP);
        DAC_OUT0_SAW(dac_periph) = tmp | ((step_value << 16U) & DAC_OUT0_SAWSTEP);
    }else if(DAC_OUT1 == dac_out){
        /* set DAC_OUT1 sawtooth step value */
        tmp = (DAC_OUT1_SAW(dac_periph) & ~(uint32_t)DAC_OUT1_SAWSTEP);
        DAC_OUT1_SAW(dac_periph) = tmp | ((step_value << 16U) & DAC_OUT1_SAWSTEP);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable DAC sawtooth step software trigger
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \retval     none
*/
void dac_sawtooth_step_software_trigger_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_SWT(dac_periph) |= (uint32_t)DAC_SWT_SWSTTR0;
    }else if(DAC_OUT1 == dac_out){
        DAC_SWT(dac_periph) |= (uint32_t)DAC_SWT_SWSTTR1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable DAC concurrent mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[out] none
    \retval     none
*/
void dac_concurrent_enable(uint32_t dac_periph)
{
    uint32_t ctl = 0U;

    ctl = (uint32_t)(DAC_CTL0_DEN0 | DAC_CTL0_DEN1);
    DAC_CTL0(dac_periph) |= (uint32_t)ctl;
}

/*!
    \brief      disable DAC concurrent mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[out] none
    \retval     none
*/
void dac_concurrent_disable(uint32_t dac_periph)
{
    uint32_t ctl = 0U;

    ctl = (uint32_t)(DAC_CTL0_DEN0 | DAC_CTL0_DEN1);
    DAC_CTL0(dac_periph) &= (uint32_t)(~ctl);
}

/*!
    \brief      enable DAC concurrent software trigger
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[out] none
    \retval     none
*/
void dac_concurrent_software_trigger_enable(uint32_t dac_periph)
{
    uint32_t swt = 0U;

    swt = (uint32_t)(DAC_SWT_SWTR0 | DAC_SWT_SWTR1);
    DAC_SWT(dac_periph) |= (uint32_t)swt; 
}

/*!
    \brief      set DAC concurrent mode data holding register value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_align: DAC data alignment mode
                only one parameter can be selected which is shown as below:
      \arg        DAC_ALIGN_12B_R: 12-bit right-aligned data
      \arg        DAC_ALIGN_12B_L: 12-bit left-aligned data
      \arg        DAC_ALIGN_8B_R: 8-bit right-aligned data
    \param[in]  data0: data to be loaded(0~4095)
    \param[in]  data1: data to be loaded(0~4095)
    \param[out] none
    \retval     none
*/
void dac_concurrent_data_set(uint32_t dac_periph, uint32_t dac_align, uint16_t data0, uint16_t data1)
{
    uint32_t data = 0U;

    switch(dac_align){
    /* 12-bit right-aligned data */
    case DAC_ALIGN_12B_R:
        data = (uint32_t)(((uint32_t)data1 << DH_12BIT_OFFSET) | data0);
        DACC_R12DH(dac_periph) = (uint32_t)data;
        break;
    /* 12-bit left-aligned data */
    case DAC_ALIGN_12B_L:
        data = (uint32_t)(((uint32_t)data1 << DH_12BIT_OFFSET) | data0);
        DACC_L12DH(dac_periph) = (uint32_t)data;
        break;
    /* 8-bit right-aligned data */
    case DAC_ALIGN_8B_R:
        data = (uint32_t)(((uint32_t)data1 << DH_8BIT_OFFSET) | data0);
        DACC_R8DH(dac_periph) = (uint32_t)data;
        break;
    default:
        break;
    }
}

/*!
    \brief      enable DAC reset persisting mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_reset_persist_enable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DRSTMD0;
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) |= (uint32_t)DAC_CTL0_DRSTMD1;
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      disable DAC reset persisting mode
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[out] none
    \retval     none
*/
void dac_reset_persist_disable(uint32_t dac_periph, uint8_t dac_out)
{
    if(DAC_OUT0 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DRSTMD0);
    }else if(DAC_OUT1 == dac_out){
        DAC_CTL0(dac_periph) &= (uint32_t)(~DAC_CTL0_DRSTMD1);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      set DAC sample and keep time value
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  dac_out: DAC_OUTx(x=0,1)
    \param[in]  sample_time: DAC sample time(0x0 ~ 0x3FF)
    \param[in]  keep_time: DAC keep time(0x0 ~ 0x3FF)
    \param[in]  refresh_time: DAC refresh time(0x0 ~ 0xFF)
    \param[out] none
    \retval     none
*/
void dac_sample_keep_mode_config(uint32_t dac_periph, uint8_t dac_out, uint32_t sample_time, uint32_t keep_time, uint32_t refresh_time)
{
    uint32_t tmp = 0U;

    if(DAC_OUT0 == dac_out){
        /* configure DACx_OUT0 Sample & Keep mode */
        DAC_SKSTR0(dac_periph) = (sample_time & DAC_SKSTR0_TSAMP0);

        tmp = (DAC_SKKTR(dac_periph) & ~(uint32_t)DAC_SKKTR_TKEEP0);
        DAC_SKKTR(dac_periph) = tmp | (keep_time & DAC_SKKTR_TKEEP0);

        tmp = (DAC_SKRTR(dac_periph) & ~(uint32_t)DAC_SKRTR_TREF0);
        DAC_SKRTR(dac_periph) = tmp | (refresh_time & DAC_SKRTR_TREF0);
    }else if(DAC_OUT1 == dac_out){
        /* configure DACx_OUT1 Sample & Keep mode */
        DAC_SKSTR1(dac_periph) = (sample_time & DAC_SKSTR1_TSAMP1);

        tmp = (DAC_SKKTR(dac_periph) & ~(uint32_t)DAC_SKKTR_TKEEP1);
        DAC_SKKTR(dac_periph) = tmp | ((keep_time << 16U) & DAC_SKKTR_TKEEP1);

        tmp = (DAC_SKRTR(dac_periph) & ~(uint32_t)DAC_SKRTR_TREF1);
        DAC_SKRTR(dac_periph) = tmp | ((refresh_time << 16U) & DAC_SKRTR_TREF1);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      get DAC flag
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  flag: the DAC status flags
				only one parameter can be selected which is shown as below:
      \arg        DAC_FLAG_DDUDR0: DACx_OUT0 DMA underrun flag
      \arg        DAC_FLAG_CALF0: DACx_OUT0 calibration offset flag
      \arg        DAC_FLAG_BWT0: DACx_OUT0 sample and keep busy writing flag
      \arg        DAC_FLAG_DDUDR1: DACx_OUT1 DMA underrun flag
      \arg        DAC_FLAG_CALF1: DACx_OUT1 calibration offset flag
      \arg        DAC_FLAG_BWT1: DACx_OUT1 sample and keep busy writing flag
    \param[out] none
    \retval     FlagStatus: SET or RESET
*/
FlagStatus dac_flag_get(uint32_t dac_periph, uint32_t flag)
{
    if(flag & DAC_STAT_FLAG_MASK0){
        /* check DAC_STAT0 flag */
        if(RESET != (DAC_STAT0(dac_periph) & flag)){
            return SET;
        }else{
            return RESET;
        }
    }else{
        /* illegal parameters */
        return RESET;
    }
}

/*!
    \brief      clear DAC flag
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  flag: DAC flag
                one or more parameters can be selected which are shown as below:
      \arg        DAC_FLAG_DDUDR0: DACx_OUT0 DMA underrun flag
      \arg        DAC_FLAG_DDUDR1: DACx_OUT1 DMA underrun flag
    \param[out] none
    \retval     none
*/
void dac_flag_clear(uint32_t dac_periph, uint32_t flag)
{
    if(flag & DAC_STAT_FLAG_MASK0){
        /* check DAC_STAT0 flag */
        DAC_STAT0(dac_periph) = (uint32_t)(flag & DAC_STAT_FLAG_MASK0);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      enable DAC interrupt
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  interrupt: the DAC interrupt
                one or more parameters can be selected which are shown as below:
      \arg        DAC_INT_DDUDR0: DACx_OUT0 DMA underrun interrupt 
      \arg        DAC_INT_DDUDR1: DACx_OUT1 DMA underrun interrupt 
    \param[out] none
    \retval     none
*/
void dac_interrupt_enable(uint32_t dac_periph, uint32_t interrupt)
{
    if(interrupt & DAC_INT_EN_MASK0){
        /* enable underrun interrupt */
        DAC_CTL0(dac_periph) |= (uint32_t)(interrupt & DAC_INT_EN_MASK0);
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      disable DAC interrupt
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  interrupt: the DAC interrupt
                one or more parameters can be selected which are shown as below:
      \arg        DAC_INT_DDUDR0: DACx_OUT0 DMA underrun interrupt
      \arg        DAC_INT_DDUDR1: DACx_OUT1 DMA underrun interrupt
    \param[out] none
    \retval     none
*/
void dac_interrupt_disable(uint32_t dac_periph, uint32_t interrupt)
{
    if(interrupt & DAC_INT_EN_MASK0){
        /* disable underrun interrupt */
        DAC_CTL0(dac_periph) &= (uint32_t)(~(interrupt & DAC_INT_EN_MASK0));
    }else{
        /* illegal parameters */
    }
}

/*!
    \brief      get DAC interrupt flag
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  int_flag: DAC interrupt flag
                only one parameter can be selected which is shown as below:
      \arg        DAC_INT_FLAG_DDUDR0: DACx_OUT0 DMA underrun interrupt flag
      \arg        DAC_INT_FLAG_DDUDR1: DACx_OUT1 DMA underrun interrupt flag
    \param[out] none
    \retval     the state of DAC interrupt flag(SET or RESET)
*/
FlagStatus dac_interrupt_flag_get(uint32_t dac_periph, uint32_t int_flag)
{
    uint32_t reg1 = 0U, reg2 = 0U;

    if(int_flag & DAC_INT_FLAG_MASK0){
        /* check underrun interrupt int_flag */
        reg1 = DAC_STAT0(dac_periph) & int_flag;
        reg2 = DAC_CTL0(dac_periph) & int_flag;
    }else{
        /* illegal parameters */
    }

    /*get DAC interrupt flag status */
    if((RESET != reg1) && (RESET != reg2)){
        return SET;
    }else{
        return RESET;
    }
}

/*!
    \brief      clear DAC interrupt flag
    \param[in]  dac_periph: DACx(x=0,1,2,3)
    \param[in]  int_flag: DAC interrupt flag
                one or more parameters can be selected which are shown as below:
      \arg        DAC_INT_FLAG_DDUDR0: DACx_OUT0 DMA underrun interrupt flag
      \arg        DAC_INT_FLAG_DDUDR1: DACx_OUT1 DMA underrun interrupt flag
    \param[out] none
    \retval     none
*/
void dac_interrupt_flag_clear(uint32_t dac_periph, uint32_t int_flag)
{
    /* clear underrun interrupt int_flag */
    if(int_flag & DAC_INT_FLAG_MASK0){
        DAC_STAT0(dac_periph) = (uint32_t)(int_flag & DAC_INT_FLAG_MASK0);
    }else{
        /* illegal parameters */
    }
}

