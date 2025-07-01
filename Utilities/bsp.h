/*!
    \file    bsp.h
    \brief   This is the main file of the underlying hardware driver. 

    \version 2025-06-05 V1.1.0, firmware for GD32G5x3
*/

#ifndef BSP_H
#define BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32g5x3.h"
#include "gd32g5x3_libopt.h"

#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_adc.h"
#include "bsp_rs232.h"
#include "bsp_rs485.h"
#include "bsp_tim_pwm.h"
#include "bsp_deinit.h"

void bsp_deinit(void);
void nvic_config(void);
void bsp_init(unsigned int baudrate);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
