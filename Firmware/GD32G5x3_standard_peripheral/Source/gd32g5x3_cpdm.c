/*!
    \file    gd32g5x3_cpdm.c
    \brief   CPDM driver

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

#include "gd32g5x3_cpdm.h"

#define CPDM_CPSEL_MASK                        ((uint32_t)0xFFFFFFF0U)  /*!< CPDM output clock phase seclection mask */
#define CPDM_DLSTCNT_MASK                      ((uint32_t)0xFFFF80FFU)  /*!< CPDM delay step count for a uint delay UINT mask */
#define CPDM_DLLENF_MASK                       ((uint32_t)0x80000000U)  /*!< CPDM delay line length valid flag mask */
#define CPDM_DLLEN_MASK                        ((uint32_t)0x0FFF0000U)  /*!< CPDM delay line length mask */

#define CPDM_DLLEN_OFFSET                      ((uint32_t)16U)          /*!< CPDM delay line length offset */
#define CPDM_DLSTCNT_OFFSET                    ((uint32_t)8U)           /*!< CPDM delay step count for a uint delay UINT offset */
#define CPDM_DLLEN_11                          ((uint32_t)0x04000000U)  /*!< CPDM delay line length bit 11 */
#define CPDM_DLLEN_10                          ((uint32_t)0x08000000U)  /*!< CPDM delay line length bit 10 */
#define CPDM_DLLEN_10_0_MASK                   ((uint32_t)0x7FFU)       /*!< CPDM delay line length bit 10 to bit 0 mask */

#define CPDM_MAX_TIMEOUT                       ((uint32_t)0x0000FFFFU)  /*!< count to judge of CPDM timeout */
/*!
    \brief      enable CPDM
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cpdm_enable(void)
{
    /* enable CPDM */
    CPDM_CTL |= (uint32_t)CPDM_CTL_CPDMEN;
}

/*!
    \brief      disable CPDM
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cpdm_disable(void)
{
    /* disable CPDM */
    CPDM_CTL &= ~(uint32_t)CPDM_CTL_CPDMEN;
}

/*!
    \brief      enable CPDM delay line sample module
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cpdm_delayline_sample_enable(void)
{
    /* enable CPDM delay line sample module */
    CPDM_CTL |= (uint32_t)CPDM_CTL_DLSEN;
}

/*!
    \brief      disable CPDM delay line sample module
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cpdm_delayline_sample_disable(void)
{
    /* disable CPDM delay line sample module */
    CPDM_CTL &= ~(uint32_t)CPDM_CTL_DLSEN;
}

/*!
    \brief      select CPDM output clock phase
    \param[in]  output_clock_phase: the output clock phase, refer to cpdm_output_phase_enum
                only one parameter can be selected which is shown as below:
      \arg        CPDM_OUTPUT_PHASE_SELECTION_0: output clock phase = input clock
      \arg        CPDM_OUTPUT_PHASE_SELECTION_1: output clock phase = input clock + 1 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_2: output clock phase = input clock + 2 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_3: output clock phase = input clock + 3 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_4: output clock phase = input clock + 4 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_5: output clock phase = input clock + 5 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_6: output clock phase = input clock + 6 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_7: output clock phase = input clock + 7 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_8: output clock phase = input clock + 8 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_9: output clock phase = input clock + 9 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_10: output clock phase = input clock + 10 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_11: output clock phase = input clock + 11 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_12: output clock phase = input clock + 12 * UNIT delay
    \param[out] none
    \retval     none
*/
void cpdm_output_clock_phase_select(cpdm_output_phase_enum output_clock_phase)
{
    uint32_t reg = 0U;

    reg = CPDM_CFG;
    reg &= CPDM_CPSEL_MASK;
    /* select CPDM output clock phase */
    reg |= output_clock_phase;
    CPDM_CFG = (uint32_t)reg;
}

/*!
    \brief      configure CPDM delay step
    \param[in]  delay_step: 0 ~ 127
    \param[out] none
    \retval     none
*/
void cpdm_delay_step_config(uint8_t delay_step)
{
    uint32_t reg = 0U;

    reg = CPDM_CFG;
    reg &= CPDM_DLSTCNT_MASK;
    /* configure delay step */
    reg |= ((uint32_t)delay_step << CPDM_DLSTCNT_OFFSET);
    CPDM_CFG = (uint32_t)reg;
}

/*!
    \brief      get delay line length valid flag
    \param[in]  none
    \param[out] none
    \retval     FlagStatus: SET or RESET
*/
FlagStatus cpdm_delayline_length_valid_flag_get(void)
{
    uint32_t reg = 0U;

    reg = CPDM_CFG;
    if(reg & CPDM_DLLENF_MASK) {
        return SET;
    } else {
        return RESET;
    }
}

/*!
    \brief      get delay line length
    \param[in]  none
    \param[out] none
    \retval     the value of delay line length, 0x00~0xFFF
*/
uint16_t cpdm_delayline_length_get(void)
{
    return (uint16_t)((CPDM_CFG & CPDM_DLLEN_MASK) >> CPDM_DLLEN_OFFSET);
}

/*!
    \brief      configure CPDM clock output
    \param[in]  output_clock_phase: the output clock phase, refer to cpdm_output_phase_enum
                only one parameter can be selected which is shown as below:
      \arg        CPDM_OUTPUT_PHASE_SELECTION_0: output clock phase = input clock
      \arg        CPDM_OUTPUT_PHASE_SELECTION_1: output clock phase = input clock + 1 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_2: output clock phase = input clock + 2 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_3: output clock phase = input clock + 3 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_4: output clock phase = input clock + 4 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_5: output clock phase = input clock + 5 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_6: output clock phase = input clock + 6 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_7: output clock phase = input clock + 7 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_8: output clock phase = input clock + 8 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_9: output clock phase = input clock + 9 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_10: output clock phase = input clock + 10 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_11: output clock phase = input clock + 11 * UNIT delay
      \arg        CPDM_OUTPUT_PHASE_SELECTION_12: output clock phase = input clock + 12 * UNIT delay
    \param[out] none
    \retval     none
*/
ErrStatus cpdm_clock_output(cpdm_output_phase_enum output_clock_phase)
{
    uint32_t reg = 0U;
    uint32_t reg_cfg = 0U;
    uint32_t delay_count = 0U;
    uint32_t timeout = 0U;

    /* enable CPDM and delay line sample module */
    CPDM_CTL = 0U;
    CPDM_CTL = CPDM_CTL_CPDMEN | CPDM_CTL_DLSEN;
    /* configure CPDM output clock phase to the max value (12) */
    reg = CPDM_CFG;
    reg &= CPDM_CPSEL_MASK;
    reg |= CPDM_MAX_PHASE;
    CPDM_CFG = (uint32_t)reg;

    for(delay_count = 0U; delay_count <= CPDM_MAX_DELAY_STEP_COUNT; delay_count++) {
        reg = CPDM_CFG;
        reg &= CPDM_DLSTCNT_MASK;
        /* configure delay line step count */
        reg |= (delay_count << CPDM_DLSTCNT_OFFSET);
        CPDM_CFG = (uint32_t)reg;

        while(RESET == (CPDM_CFG & CPDM_CFG_DLLENF)) {
            timeout++;

            if(timeout > CPDM_MAX_TIMEOUT)
            {
                return ERROR;
            }
        }

        reg_cfg = CPDM_CFG;
        if((((reg_cfg >> CPDM_DLLEN_OFFSET) & CPDM_DLLEN_10_0_MASK) > 0U) &&
                ((RESET == (reg_cfg & CPDM_DLLEN_11)) || (RESET == (reg_cfg & CPDM_DLLEN_10)))) {
            break;
        }
    }

    /* enable CPDM and delay line sample module */
    CPDM_CTL = 0U;
    CPDM_CTL = CPDM_CTL_CPDMEN | CPDM_CTL_DLSEN;
    /* select the output clock phase */
    reg = CPDM_CFG;
    reg &= CPDM_CPSEL_MASK;
    reg |= output_clock_phase;
    CPDM_CFG = (uint32_t)reg;
    /* disable delay line sample module */
    CPDM_CTL = CPDM_CTL_CPDMEN;
    
    return SUCCESS;
}
