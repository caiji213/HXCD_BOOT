/*!
    \file    gd32g5x3_dac.h
    \brief   definitions for the DAC

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

#ifndef GD32G5X3_DAC_H
#define GD32G5X3_DAC_H

#include "gd32g5x3.h"

/* DACx(x=0,1,2,3) definitions */
#define DAC0                              (DAC_BASE)
#define DAC1                              (DAC_BASE + 0x00000400U)
#define DAC2                              (DAC_BASE + 0x00000800U)
#define DAC3                              (DAC_BASE + 0x00000C00U)

/* registers definitions */
#define DAC_CTL0(dacx)                    REG32((dacx) + 0x00000000U)          /*!< DACx control register 0 */
#define DAC_SWT(dacx)                     REG32((dacx) + 0x00000004U)          /*!< DACx software trigger register */

#define DAC_OUT0_R12DH(dacx)              REG32((dacx) + 0x00000008U)          /*!< DACx_OUT0 12-bit right-aligned data holding register */
#define DAC_OUT0_L12DH(dacx)              REG32((dacx) + 0x0000000CU)          /*!< DACx_OUT0 12-bit left-aligned data holding register */
#define DAC_OUT0_R8DH(dacx)               REG32((dacx) + 0x00000010U)          /*!< DACx_OUT0 8-bit right-aligned data holding register */
#define DAC_OUT1_R12DH(dacx)              REG32((dacx) + 0x00000014U)          /*!< DACx_OUT1 12-bit right-aligned data holding register */
#define DAC_OUT1_L12DH(dacx)              REG32((dacx) + 0x00000018U)          /*!< DACx_OUT1 12-bit left-aligned data holding register */
#define DAC_OUT1_R8DH(dacx)               REG32((dacx) + 0x0000001CU)          /*!< DACx_OUT1 8-bit right-aligned data holding register */
#define DACC_R12DH(dacx)                  REG32((dacx) + 0x00000020U)          /*!< DACx concurrent mode 12-bit right-aligned data holding register */
#define DACC_L12DH(dacx)                  REG32((dacx) + 0x00000024U)          /*!< DACx concurrent mode 12-bit left-aligned data holding register */
#define DACC_R8DH(dacx)                   REG32((dacx) + 0x00000028U)          /*!< DACx concurrent mode 8-bit right-aligned data holding register */
#define DAC_OUT0_DO(dacx)                 REG32((dacx) + 0x0000002CU)          /*!< DACx_OUT0 data output register */
#define DAC_OUT1_DO(dacx)                 REG32((dacx) + 0x00000030U)          /*!< DACx_OUT1 data output register */

#define DAC_STAT0(dacx)                   REG32((dacx) + 0x00000034U)          /*!< DACx status register 0 */

#define DAC_CALR(dacx)                    REG32((dacx) + 0x00000038U)          /*!< DACx calibration register */
#define DAC_MDCR(dacx)                    REG32((dacx) + 0x0000003CU)          /*!< DACx mode control register */
#define DAC_SKSTR0(dacx)                  REG32((dacx) + 0x00000040U)          /*!< DACx sample and keep sample time register 0 */
#define DAC_SKSTR1(dacx)                  REG32((dacx) + 0x00000044U)          /*!< DACx sample and keep sample time register 1 */
#define DAC_SKKTR(dacx)                   REG32((dacx) + 0x00000048U)          /*!< DACx sample and keep keep time register */
#define DAC_SKRTR(dacx)                   REG32((dacx) + 0x0000004CU)          /*!< DACx sample and keep refresh time register */

#define DAC_OUT0_SAW(dacx)                REG32((dacx) + 0x00000058U)          /*!< DACx_OUT0 sawtooth register */
#define DAC_OUT1_SAW(dacx)                REG32((dacx) + 0x0000005CU)          /*!< DACx_OUT1 sawtooth register */
#define DAC_SAWMDR(dacx)                  REG32((dacx) + 0x00000060U)          /*!< DACx sawtooth mode register */

/* bits definitions */
/* DAC_CTL0 */
#define DAC_CTL0_DEN0                     BIT(0)                               /*!< DACx_OUT0 enable */
#define DAC_CTL0_DTEN0                    BIT(1)                               /*!< DACx_OUT0 trigger enable */
#define DAC_CTL0_DTSEL0                   BITS(2,3)                            /*!< DACx_OUT0 trigger selection */
#define DAC_CTL0_DWM0                     BITS(6,7)                            /*!< DACx_OUT0 noise wave mode */
#define DAC_CTL0_DWBW0                    BITS(8,11)                           /*!< DACx_OUT0 noise wave bit width */
#define DAC_CTL0_DDMAEN0                  BIT(12)                              /*!< DACx_OUT0 DMA enable */
#define DAC_CTL0_DDUDRIE0                 BIT(13)                              /*!< DACx_OUT0 DMA underrun interrupt enable */
#define DAC_CTL0_CALEN0                   BIT(14)                              /*!< DACx_OUT0 calibration enable */
#define DAC_CTL0_DRSTMD0                  BIT(15)                              /*!< DACx_OUT0 reset mode */

#define DAC_CTL0_DEN1                     BIT(16)                              /*!< DACx_OUT1 enable bit */ 
#define DAC_CTL0_DTEN1                    BIT(17)                              /*!< DACx_OUT1 trigger enable */
#define DAC_CTL0_DTSEL1                   BITS(18,19)                          /*!< DACx_OUT1 trigger selection */
#define DAC_CTL0_DWM1                     BITS(22,23)                          /*!< DACx_OUT1 noise wave mode */
#define DAC_CTL0_DWBW1                    BITS(24,27)                          /*!< DACx_OUT1 noise wave bit width */
#define DAC_CTL0_DDMAEN1                  BIT(28)                              /*!< DACx_OUT1 DMA enable */
#define DAC_CTL0_DDUDRIE1                 BIT(29)                              /*!< DACx_OUT1 DMA underrun interrupt enable */
#define DAC_CTL0_CALEN1                   BIT(30)                              /*!< DACx_OUT1 calibration enable */
#define DAC_CTL0_DRSTMD1                  BIT(31)                              /*!< DACx_OUT1 reset mode */

/* DAC_SWT */
#define DAC_SWT_SWTR0                     BIT(0)                               /*!< DACx_OUT0 software trigger */
#define DAC_SWT_SWTR1                     BIT(1)                               /*!< DACx_OUT1 software trigger */
#define DAC_SWT_SWSTTR0                   BIT(16)                              /*!< DACx_OUT0 sawtooth step-up/down software trigger */
#define DAC_SWT_SWSTTR1                   BIT(17)                              /*!< DACx_OUT1 sawtooth step-up/down software trigger */

/* DAC_OUT0_R12DH */
#define DAC_OUT0_DH_R12                   BITS(0,11)                           /*!< DACx_OUT0 12-bit right-aligned data */

/* DAC_OUT0_L12DH */
#define DAC_OUT0_DH_L12                   BITS(4,15)                           /*!< DACx_OUT0 12-bit left-aligned data */

/* DAC_OUT0_R8DH */
#define DAC_OUT0_DH_R8                    BITS(0,7)                            /*!< DACx_OUT0 8-bit right-aligned data */

/* DAC_OUT1_R12DH */
#define DAC_OUT1_DH_R12                   BITS(0,11)                           /*!< DACx_OUT1 12-bit right-aligned data */

/* DAC_OUT1_L12DH */
#define DAC_OUT1_DH_L12                   BITS(4,15)                           /*!< DACx_OUT1 12-bit left-aligned data */

/* DAC_OUT1_R8DH */
#define DAC_OUT1_DH_R8                    BITS(0,7)                            /*!< DACx_OUT1 8-bit right-aligned data */

/* DACC_R12DH */
#define DACC_OUT0_DH_R12                  BITS(0,11)                           /*!< DAC concurrent mode DACx_OUT0 12-bit right-aligned data */
#define DACC_OUT1_DH_R12                  BITS(16,27)                          /*!< DAC concurrent mode DACx_OUT1 12-bit right-aligned data */

/* DACC_L12DH */
#define DACC_OUT0_DH_L12                  BITS(4,15)                           /*!< DAC concurrent mode DACx_OUT0 12-bit left-aligned data */
#define DACC_OUT1_DH_L12                  BITS(20,31)                          /*!< DAC concurrent mode DACx_OUT1 12-bit left-aligned data */

/* DACC_R8DH */
#define DACC_OUT0_DH_R8                   BITS(0,7)                            /*!< DAC concurrent mode DACx_OUT0 8-bit right-aligned data */
#define DACC_OUT1_DH_R8                   BITS(8,15)                           /*!< DAC concurrent mode DACx_OUT1 8-bit right-aligned data */

/* DAC0_DO */
#define DAC_OUT0_DO_BITS                  BITS(0,11)                           /*!< DACx_OUT0 12-bit output data */

/* DAC1_DO */
#define DAC_OUT1_DO_BITS                  BITS(0,11)                           /*!< DACx_OUT1 12-bit output data */

/* DAC_STAT0 */
#define DAC_STAT0_DDUDR0                  BIT(13)                              /*!< DACx_OUT0 DMA underrun flag */
#define DAC_STAT0_CALF0                   BIT(14)                              /*!< DACx_OUT0 calibration offset flag */
#define DAC_STAT0_BWT0                    BIT(15)                              /*!< DACx_OUT0 SKSTR0 writing flag */
#define DAC_STAT0_DDUDR1                  BIT(29)                              /*!< DACx_OUT1 DMA underrun flag */
#define DAC_STAT0_CALF1                   BIT(30)                              /*!< DACx_OUT1 calibration offset flag */
#define DAC_STAT0_BWT1                    BIT(31)                              /*!< DACx_OUT1 SKSTR1 writing flag */

/* DAC_CALR */
#define DAC_CALR_OTV0                     BITS(0,4)                            /*!< DACx_OUT0 offset calibration value */
#define DAC_CALR_OTV1                     BITS(16,20)                          /*!< DACx_OUT1 offset calibration value */

/* DAC_MDCR */
#define DAC_MDCR_MODE0                    BITS(0,2)                            /*!< DACx_OUT0 mode */
#define DAC_MDCR_DHFMT0                   BIT(9)                               /*!< DACx_OUT0 data format register */
#define DAC_MDCR_MODE1                    BITS(16,18)                          /*!< DACx_OUT1 mode */
#define DAC_MDCR_DHFMT1                   BIT(25)                              /*!< DACx_OUT1 data format register */

/* DAC_SKSTR0 */
#define DAC_SKSTR0_TSAMP0                 BITS(0,9)                            /*!< DACx_OUT0 sample time */

/* DAC_SKSTR1 */
#define DAC_SKSTR1_TSAMP1                 BITS(0,9)                            /*!< DACx_OUT1 sample time */

/* DAC_SKKTR */
#define DAC_SKKTR_TKEEP0                  BITS(0,9)                            /*!< DACx_OUT0 keep time */
#define DAC_SKKTR_TKEEP1                  BITS(16,25)                          /*!< DACx_OUT1 keep time */

/* DAC_SKRTR */
#define DAC_SKRTR_TREF0                   BITS(0,7)                            /*!< DACx_OUT0 refresh time */
#define DAC_SKRTR_TREF1                   BITS(16,23)                          /*!< DACx_OUT1 refresh time */

/* DAC_OUT0_SAW */
#define DAC_OUT0_SAWINIT                  BITS(0,11)                           /*!< DACx_OUT0 sawtooth initial value */
#define DAC_OUT0_SAWDIR                   BIT(12)                              /*!< DACx_OUT0 sawtooth step direction */
#define DAC_OUT0_SAWSTEP                  BITS(16,31)                          /*!< DACx_OUT0 sawtooth step value */

/* DAC_OUT1_SAW */
#define DAC_OUT1_SAWINIT                  BITS(0,11)                           /*!< DACx_OUT1 sawtooth initial value */
#define DAC_OUT1_SAWDIR                   BIT(12)                              /*!< DACx_OUT1 sawtooth step direction */
#define DAC_OUT1_SAWSTEP                  BITS(16,31)                          /*!< DACx_OUT0 sawtooth step value */

/* DAC_SAWMDR */
#define DAC_SAWMDR_RSTTSEL0               BITS(0,1)                            /*!< DACx_OUT0 sawtooth reset trigger selection */
#define DAC_SAWMDR_STEPTSEL0              BITS(8,9)                            /*!< DACx_OUT0 sawtooth step-up/down trigger selection */
#define DAC_SAWMDR_RSTTSEL1               BITS(16,17)                          /*!< DACx_OUT1 sawtooth reset trigger selection */
#define DAC_SAWMDR_STEPTSEL1              BITS(24,25)                          /*!< DACx_OUT1 sawtooth step-up/down trigger selection */

/* constants definitions */
/* DAC trigger source */
#define CTL0_DTSEL(regval)                (BITS(2,3) & ((uint32_t)(regval) << 2U))
#define DAC_TRIGGER_EXTERNAL              CTL0_DTSEL(0)                        /*!< external trigger selected from TRIGSEL */
#define DAC_TRIGGER_SOFTWARE              CTL0_DTSEL(1)                        /*!< software trigger */

/* DAC noise wave mode */
#define CTL0_DWM(regval)                  (BITS(6,7) & ((uint32_t)(regval) << 6U))
#define DAC_WAVE_DISABLE                  CTL0_DWM(0)                          /*!< wave disabled */
#define DAC_WAVE_MODE_LFSR                CTL0_DWM(1)                          /*!< LFSR noise mode */
#define DAC_WAVE_MODE_TRIANGLE            CTL0_DWM(2)                          /*!< triangle noise mode */
#define DAC_WAVE_MODE_SAWTOOTH            CTL0_DWM(3)                          /*!< sawtooth noise mode */

/* DAC noise wave bit width */
#define DWBW(regval)                      (BITS(8, 11) & ((uint32_t)(regval) << 8U))
#define DAC_WAVE_BIT_WIDTH_1              DWBW(0)                              /*!< bit width of the wave signal is 1 */
#define DAC_WAVE_BIT_WIDTH_2              DWBW(1)                              /*!< bit width of the wave signal is 2 */
#define DAC_WAVE_BIT_WIDTH_3              DWBW(2)                              /*!< bit width of the wave signal is 3 */
#define DAC_WAVE_BIT_WIDTH_4              DWBW(3)                              /*!< bit width of the wave signal is 4 */
#define DAC_WAVE_BIT_WIDTH_5              DWBW(4)                              /*!< bit width of the wave signal is 5 */
#define DAC_WAVE_BIT_WIDTH_6              DWBW(5)                              /*!< bit width of the wave signal is 6 */
#define DAC_WAVE_BIT_WIDTH_7              DWBW(6)                              /*!< bit width of the wave signal is 7 */
#define DAC_WAVE_BIT_WIDTH_8              DWBW(7)                              /*!< bit width of the wave signal is 8 */
#define DAC_WAVE_BIT_WIDTH_9              DWBW(8)                              /*!< bit width of the wave signal is 9 */
#define DAC_WAVE_BIT_WIDTH_10             DWBW(9)                              /*!< bit width of the wave signal is 10 */
#define DAC_WAVE_BIT_WIDTH_11             DWBW(10)                             /*!< bit width of the wave signal is 11 */
#define DAC_WAVE_BIT_WIDTH_12             DWBW(11)                             /*!< bit width of the wave signal is 12 */

/* unmask LFSR bits in DAC LFSR noise mode */
#define DAC_LFSR_BIT0                     DAC_WAVE_BIT_WIDTH_1                 /*!< unmask the LFSR bit0 */
#define DAC_LFSR_BITS1_0                  DAC_WAVE_BIT_WIDTH_2                 /*!< unmask the LFSR bits[1:0] */
#define DAC_LFSR_BITS2_0                  DAC_WAVE_BIT_WIDTH_3                 /*!< unmask the LFSR bits[2:0] */
#define DAC_LFSR_BITS3_0                  DAC_WAVE_BIT_WIDTH_4                 /*!< unmask the LFSR bits[3:0] */
#define DAC_LFSR_BITS4_0                  DAC_WAVE_BIT_WIDTH_5                 /*!< unmask the LFSR bits[4:0] */
#define DAC_LFSR_BITS5_0                  DAC_WAVE_BIT_WIDTH_6                 /*!< unmask the LFSR bits[5:0] */
#define DAC_LFSR_BITS6_0                  DAC_WAVE_BIT_WIDTH_7                 /*!< unmask the LFSR bits[6:0] */
#define DAC_LFSR_BITS7_0                  DAC_WAVE_BIT_WIDTH_8                 /*!< unmask the LFSR bits[7:0] */
#define DAC_LFSR_BITS8_0                  DAC_WAVE_BIT_WIDTH_9                 /*!< unmask the LFSR bits[8:0] */
#define DAC_LFSR_BITS9_0                  DAC_WAVE_BIT_WIDTH_10                /*!< unmask the LFSR bits[9:0] */
#define DAC_LFSR_BITS10_0                 DAC_WAVE_BIT_WIDTH_11                /*!< unmask the LFSR bits[10:0] */
#define DAC_LFSR_BITS11_0                 DAC_WAVE_BIT_WIDTH_12                /*!< unmask the LFSR bits[11:0] */

/* triangle amplitude in DAC triangle noise mode */
#define DAC_TRIANGLE_AMPLITUDE_1          DAC_WAVE_BIT_WIDTH_1                 /*!< triangle amplitude is 1 */
#define DAC_TRIANGLE_AMPLITUDE_3          DAC_WAVE_BIT_WIDTH_2                 /*!< triangle amplitude is 3 */
#define DAC_TRIANGLE_AMPLITUDE_7          DAC_WAVE_BIT_WIDTH_3                 /*!< triangle amplitude is 7 */
#define DAC_TRIANGLE_AMPLITUDE_15         DAC_WAVE_BIT_WIDTH_4                 /*!< triangle amplitude is 15 */
#define DAC_TRIANGLE_AMPLITUDE_31         DAC_WAVE_BIT_WIDTH_5                 /*!< triangle amplitude is 31 */
#define DAC_TRIANGLE_AMPLITUDE_63         DAC_WAVE_BIT_WIDTH_6                 /*!< triangle amplitude is 63 */
#define DAC_TRIANGLE_AMPLITUDE_127        DAC_WAVE_BIT_WIDTH_7                 /*!< triangle amplitude is 127 */
#define DAC_TRIANGLE_AMPLITUDE_255        DAC_WAVE_BIT_WIDTH_8                 /*!< triangle amplitude is 255 */
#define DAC_TRIANGLE_AMPLITUDE_511        DAC_WAVE_BIT_WIDTH_9                 /*!< triangle amplitude is 511 */
#define DAC_TRIANGLE_AMPLITUDE_1023       DAC_WAVE_BIT_WIDTH_10                /*!< triangle amplitude is 1023 */
#define DAC_TRIANGLE_AMPLITUDE_2047       DAC_WAVE_BIT_WIDTH_11                /*!< triangle amplitude is 2047 */
#define DAC_TRIANGLE_AMPLITUDE_4095       DAC_WAVE_BIT_WIDTH_12                /*!< triangle amplitude is 4095 */

/* DAC sawtooth reset trigger source */
#define SAWMDR_SAWRSTTSEL(regval)                   (BITS(0,1) & ((uint32_t)(regval) << 0U))
#define DAC_SAWTOOTH_RESET_TRIGGER_EXTERNAL 	    SAWMDR_SAWRSTTSEL(0)         /*!< external trigger selected from TRIGSEL */
#define DAC_SAWTOOTH_RESET_TRIGGER_SOFTWARE         SAWMDR_SAWRSTTSEL(1)         /*!< software trigger */

/* DAC sawtooth step trigger source */
#define SAWMDR_SAWSTEPTSEL(regval)                  (BITS(8,9) & ((uint32_t)(regval) << 8U))
#define DAC_SAWTOOTH_STEP_TRIGGER_EXTERNAL          SAWMDR_SAWSTEPTSEL(0)        /*!< external trigger from TRIGSEL */
#define DAC_SAWTOOTH_STEP_TRIGGER_SOFTWARE          SAWMDR_SAWSTEPTSEL(1)        /*!< software trigger */

/* DAC sawtooth step direction */
#define SAWMDR_SAWDIR(regval)          	  (BIT(12) & ((uint32_t)(regval) << 12U))
#define DAC_SAWTOOTH_STEP_DOWN            SAWMDR_SAWDIR(0)              	    /*!< sawtooth direction is step-down */
#define DAC_SAWTOOTH_STEP_UP              SAWMDR_SAWDIR(1)             		    /*!< sawtooth direction is step-up */

/* DAC data alignment */
#define DATA_ALIGN(regval)                (BITS(0,1) & ((uint32_t)(regval) << 0U))
#define DAC_ALIGN_12B_R                   DATA_ALIGN(0)                        /*!< 12-bit right-aligned data */
#define DAC_ALIGN_12B_L                   DATA_ALIGN(1)                        /*!< 12-bit left-aligned data */
#define DAC_ALIGN_8B_R                    DATA_ALIGN(2)                        /*!< 8-bit right-aligned data */

/* DAC data format */
#define MDCR_DHFMT(regval)             	  (BIT(9) & ((uint32_t)(regval) << 9U))
#define DAC_DATA_FORMAT_UNSIGNED          MDCR_DHFMT(0)              		   /*!< input data is unsigned format */
#define DAC_DATA_FORMAT_SIGNED            MDCR_DHFMT(1)              		   /*!< input data is signed format */

/* DAC mode */
#define MODE(regval)                      (BITS(0,2) & ((uint32_t)(regval)))
#define NORMAL_PIN_BUFFON                 MODE(0)                              /*!< DACx_OUTy work in normal mode and connect to external pin with buffer enable */
#define NORMAL_PIN_PERIPH_BUFFON          MODE(1)                              /*!< DACx_OUTy work in normal mode and connect to external pin and on chip peripherals with buffer enable */
#define NORMAL_PIN_BUFFOFF                MODE(2)                              /*!< DACx_OUTy work in normal mode and connect to external pin with buffer disable */
#define NORMAL_PERIPH_BUFFOFF             MODE(3)                              /*!< DACx_OUTy work in normal mode and connect to on chip peripherals with buffer disable */
#define SAMPLEKEEP_PIN_BUFFON             MODE(4)                              /*!< DACx_OUTy work in sample and keep mode and connect to external pin with buffer enable */
#define SAMPLEKEEP_PIN_PERIPH_BUFFON      MODE(5)                              /*!< DACx_OUTy work in sample and keep mode and connect to external pin and on chip peripherals with buffer enable */
#define SAMPLEKEEP_PIN_BUFFOFF            MODE(6)                              /*!< DACx_OUTy work in sample and keep mode and connect to external pin and on chip peripherals with buffer disable */
#define SAMPLEKEEP_PERIPH_BUFFOFF         MODE(7)                              /*!< DACx_OUTy work in sample and keep mode and connect to on chip peripherals with buffer disable */

/* DAC output channel definitions */
#define DAC_OUT0                          ((uint8_t)0x00U)                     /*!< DACx_OUT0 channel */
#define DAC_OUT1                          ((uint8_t)0x01U)                     /*!< DACx_OUT1 channel */

/* DAC interrupt */
#define DAC_INT_DDUDR0                    DAC_CTL0_DDUDRIE0                    /*!< DACx_OUT0 DMA underrun interrupt enable */
#define DAC_INT_DDUDR1                    DAC_CTL0_DDUDRIE1                    /*!< DACx_OUT1 DMA underrun interrupt enable */

/* DAC interrupt flag */
#define DAC_INT_FLAG_DDUDR0               DAC_STAT0_DDUDR0                     /*!< DACx_OUT0 DMA underrun interrupt flag */
#define DAC_INT_FLAG_DDUDR1               DAC_STAT0_DDUDR1                     /*!< DACx_OUT1 DMA underrun interrupt flag */

/* DAC flags */
#define DAC_FLAG_DDUDR0                   DAC_STAT0_DDUDR0                     /*!< DACx_OUT0 DMA underrun flag */
#define DAC_FLAG_CALF0                    DAC_STAT0_CALF0                      /*!< DACx_OUT0 calibration offset flag */
#define DAC_FLAG_BWT0                     DAC_STAT0_BWT0                       /*!< DACx_OUT0 sample and keep wtire enable flag */
#define DAC_FLAG_DDUDR1                   DAC_STAT0_DDUDR1                     /*!< DACx_OUT1 DMA underrun flag */
#define DAC_FLAG_CALF1                    DAC_STAT0_CALF1                      /*!< DACx_OUT1 calibration offset flag */
#define DAC_FLAG_BWT1                     DAC_STAT0_BWT1                       /*!< DACx_OUT1 sample and keep wtire enable flag */

/* function declarations */
/* DAC initialization functions */
/* deinitialize DAC */
void dac_deinit(uint32_t dac_periph);
/* enable DAC */
void dac_enable(uint32_t dac_periph, uint8_t dac_out);
/* disable DAC */
void dac_disable(uint32_t dac_periph, uint8_t dac_out);
/* enable DAC DMA function */
void dac_dma_enable(uint32_t dac_periph, uint8_t dac_out);
/* disable DAC DMA function */
void dac_dma_disable(uint32_t dac_periph, uint8_t dac_out);

/* DAC buffer functions */
/* configure DAC mode */
void dac_mode_config(uint32_t dac_periph, uint8_t dac_out, uint32_t mode);
/* get the DACx trimming value */
uint32_t dac_trimming_value_get(uint32_t dac_periph, uint8_t dac_out);
/* set the DACx trimming value */
void dac_trimming_value_set(uint32_t dac_periph, uint8_t dac_out, uint32_t trim_value);
/* enable the DACx trimming */
void dac_trimming_enable(uint32_t dac_periph, uint8_t dac_out);
/* disable the DACx trimming */
void dac_trimming_disable(uint32_t dac_periph, uint8_t dac_out);

/* read and write operation functions */
/* get DAC output value */
uint16_t dac_output_value_get(uint32_t dac_periph, uint8_t dac_out);
/* DAC input data format config */
void dac_data_format_config(uint32_t dac_periph, uint8_t dac_out, uint32_t format);
/* set DAC data holding register value */
void dac_data_set(uint32_t dac_periph, uint8_t dac_out, uint32_t dac_align, uint16_t data);

/* DAC trigger configuration */
/* enable DAC trigger */
void dac_trigger_enable(uint32_t dac_periph, uint8_t dac_out);
/* disable DAC trigger */
void dac_trigger_disable(uint32_t dac_periph, uint8_t dac_out);
/* configure DAC trigger source */
void dac_trigger_source_config(uint32_t dac_periph, uint8_t dac_out, uint32_t triggersource);
/* enable DAC software trigger */
void dac_software_trigger_enable(uint32_t dac_periph, uint8_t dac_out);

/* DAC wave mode configuration */
/* configure DAC wave mode */
void dac_wave_mode_config(uint32_t dac_periph, uint8_t dac_out, uint32_t wave_mode);
/* configure DAC LFSR noise mode */
void dac_lfsr_noise_config(uint32_t dac_periph, uint8_t dac_out, uint32_t unmask_bits);
/* configure DAC triangle noise mode */
void dac_triangle_noise_config(uint32_t dac_periph, uint8_t dac_out, uint32_t amplitude);

/* DAC sawtooth wave mode configuration */
/* configure DAC sawtooth reset trigger source */
void dac_sawtooth_reset_trigger_source_config(uint32_t dac_periph, uint8_t dac_out, uint32_t triggersource);
/* configure DAC sawtooth step trigger source */
void dac_sawtooth_step_trigger_source_config(uint32_t dac_periph, uint8_t dac_out, uint32_t triggersource);
/* configure DAC sawtooth step direction */
void dac_sawtooth_step_direction_config(uint32_t dac_periph, uint8_t dac_out, uint32_t direction);
/* set DAC sawtooth initial value */
void dac_sawtooth_initial_value_set(uint32_t dac_periph, uint8_t dac_out, uint32_t init_value);
/* set DAC sawtooth step value */
void dac_sawtooth_step_value_set(uint32_t dac_periph, uint8_t dac_out, uint32_t step_value);
/* enable DAC sawtooth step software trigger */
void dac_sawtooth_step_software_trigger_enable(uint32_t dac_periph, uint8_t dac_out);

/* DAC concurrent mode configuration */
/* enable DAC concurrent mode */
void dac_concurrent_enable(uint32_t dac_periph);
/* disable DAC concurrent mode */
void dac_concurrent_disable(uint32_t dac_periph);
/* enable DAC concurrent software trigger */
void dac_concurrent_software_trigger_enable(uint32_t dac_periph);
/* set DAC concurrent mode data holding register value */
void dac_concurrent_data_set(uint32_t dac_periph, uint32_t dac_align, uint16_t data0, uint16_t data1);

/* DAC reset pesist mode */
/* enable DAC reset persisting mode */
void dac_reset_persist_enable(uint32_t dac_periph, uint8_t dac_out);
/* disable DAC reset persisting mode */
void dac_reset_persist_disable(uint32_t dac_periph, uint8_t dac_out);

/* DAC sample and keep functions */
/* DAC sample and keep mode config */
void dac_sample_keep_mode_config(uint32_t dac_periph, uint8_t dac_out, uint32_t sample_time, uint32_t keep_time, uint32_t refresh_time);

/* DAC interrupt and flag functions */
/* get DAC flag */
FlagStatus dac_flag_get(uint32_t dac_periph, uint32_t flag);
/* clear DAC flag */
void dac_flag_clear(uint32_t dac_periph, uint32_t flag);
/* enable DAC interrupt */
void dac_interrupt_enable(uint32_t dac_periph, uint32_t interrupt);
/* disable DAC interrupt */
void dac_interrupt_disable(uint32_t dac_periph, uint32_t interrupt);
/* get DAC interrupt flag */
FlagStatus dac_interrupt_flag_get(uint32_t dac_periph, uint32_t int_flag);
/* clear DAC interrupt flag */
void dac_interrupt_flag_clear(uint32_t dac_periph, uint32_t int_flag);

#endif /* GD32G5X3_DAC_H */
