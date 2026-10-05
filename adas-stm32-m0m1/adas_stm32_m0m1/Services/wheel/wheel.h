/**
 * @file    wheel.h
 * @brief   One wheel = encoder + motor driver. Reusable for any motor/encoder:
 *          only wheel_cfg_t changes. Create one wheel_t per wheel (array of 4
 *          in M3).
 */
#ifndef WHEEL_H
#define WHEEL_H

#include <stdint.h>
#include "encoder.h"
#include "motor_driver.h"
#include "status.h"

typedef struct
{
    enc_cfg_t enc;     /* counter_bits is overwritten from the BSP table */
    mot_cfg_t mot;     /* wheel_idx   is overwritten from wheel_init()   */
} wheel_cfg_t;

typedef struct
{
    uint8_t idx;
    enc_t   enc;
    mot_t   mot;
} wheel_t;

status_t wheel_init(wheel_t *w, uint8_t idx, const wheel_cfg_t *cfg);

/**
 * One control tick: read encoder, update estimator, update motor output.
 * @return ST_OK / ST_WARN_OVERSPEED (motor still updated), or an error (motor
 *         is stopped and braked).
 */
status_t wheel_step(wheel_t *w);

void     wheel_set_duty(wheel_t *w, int16_t permille);
void     wheel_stop(wheel_t *w);
status_t wheel_reset_encoder(wheel_t *w);

#endif /* WHEEL_H */
