/**
 * @file    motor_driver.h
 * @brief   BTS7960 H-bridge logic: limits, ramp, dead-band compensation and
 *          safe direction reversal. Hardware access only through BSP.
 *
 * Command unit: signed permille of full PWM (-1000..+1000).
 * mot_update() must be called at a fixed period (the control tick).
 */
#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "status.h"

typedef struct
{
    uint8_t wheel_idx;                /* index in the BSP tables                  */
    int16_t max_duty_permille;        /* hard limit, 1..1000                      */
    int16_t deadband_permille;        /* duty at which the motor just starts      */
    int16_t ramp_permille_per_tick;   /* max change of output per tick, >= 1      */
    uint8_t deadtime_ticks;           /* ticks held at zero before reversing      */
} mot_cfg_t;

typedef struct
{
    mot_cfg_t cfg;
    int16_t   target;         /* requested duty                                   */
    int16_t   output;         /* duty after limit+ramp (before dead-band map)     */
    int8_t    cur_dir;        /* -1, 0, +1: what the bridge is currently set to   */
    uint8_t   deadtime_left;
} mot_t;

status_t mot_init(mot_t *m, const mot_cfg_t *cfg);
void     mot_set_target(mot_t *m, int16_t permille);
void     mot_stop(mot_t *m);              /* immediate zero + brake                */
void     mot_update(mot_t *m);            /* call every control tick               */
status_t mot_set_limit(mot_t *m, int16_t max_duty_permille);
status_t mot_set_deadband(mot_t *m, int16_t deadband_permille);
int16_t  mot_get_output(const mot_t *m);

#endif /* MOTOR_DRIVER_H */
