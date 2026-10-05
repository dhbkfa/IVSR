/**
 * @file    encoder.h
 * @brief   Quadrature encoder position/speed from a raw hardware counter.
 *          Pure C: no HAL, no RTOS. Fully unit-testable on a PC.
 *
 * Usage (every control tick, single writer):
 *     enc_update(&e, raw_counter);
 *     speed = enc_get_speed_rad_s(&e, dt_s);
 */
#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "status.h"

typedef struct
{
    uint8_t counter_bits;    /* 16 or 32: width of the hardware counter          */
    int8_t  direction;       /* +1 or -1: flips the sign of the counting         */
    float   counts_per_rev;  /* counts per OUTPUT shaft rev (x4 and gearbox incl.) */
} enc_cfg_t;

typedef struct
{
    enc_cfg_t cfg;
    uint32_t  last_raw;
    int32_t   total_counts;  /* saturating accumulator (range +-2.1e9 counts)    */
    int32_t   delta_counts;  /* counts during the last update                    */
    bool      initialized;
} enc_t;

status_t enc_init(enc_t *e, const enc_cfg_t *cfg);

/**
 * Update with the raw counter value.
 * @return ST_OK, ST_WARN_OVERSPEED if |delta| > 1/4 of the counter range
 *         (result may be wrong), ST_ERR_PARAM on NULL.
 */
status_t enc_update(enc_t *e, uint32_t raw);

/** Zero the position and resynchronise with the current raw counter. */
void     enc_reset(enc_t *e, uint32_t raw_now);

/** Change counts-per-rev at run time (used while calibrating). */
status_t enc_set_counts_per_rev(enc_t *e, float cpr);

int32_t  enc_get_count(const enc_t *e);
int32_t  enc_get_delta(const enc_t *e);
float    enc_get_angle_deg(const enc_t *e);
float    enc_get_speed_rad_s(const enc_t *e, float dt_s);

#endif /* ENCODER_H */
