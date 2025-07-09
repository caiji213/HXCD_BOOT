/*!
    \file    bsp_deinit.h
    \brief   This file provides the declarations for all peripheral deinitialization functions.
             Designed for GD32G5x3 series firmware.

    \version 2025-06-05, V1.0.0
*/

#ifndef BSP_DEINIT_H
#define BSP_DEINIT_H

#ifdef __cplusplus
extern "C" {
#endif

void dma_deinit_all(void);
void uart_deinit_all(void);
void gpio_deinit_all(void);
void timer_deinit_all(void);
void i2c_deinit_all(void);
void ccr_deinit_all(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DEINIT_H */