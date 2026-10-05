/**
 * @file    bsp_encoder.h
 * @brief   Raw access to the timers used in quadrature-encoder mode.
 *          No filtering, no unit conversion: that lives in Components/encoder.
 */
#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>
#include "status.h"

status_t bsp_encoder_start(uint8_t wheel);
status_t bsp_encoder_read(uint8_t wheel, uint32_t *raw);
uint8_t  bsp_encoder_bits(uint8_t wheel);      /* 16 or 32 */

/**
 * Sanity check of the timer registers:
 *  - CR1.CEN   : counter is running
 *  - SMCR.SMS  : encoder mode 3 (TI1 and TI2, x4 counting)
 *  - ARR       : 0xFFFF (16-bit) or 0xFFFFFFFF (32-bit), needed for wrap handling
 */
status_t bsp_encoder_check(uint8_t wheel);

#endif /* BSP_ENCODER_H */
