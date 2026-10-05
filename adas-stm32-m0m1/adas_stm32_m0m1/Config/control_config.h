/**
 * @file    control_config.h
 * @brief   Timing and bring-up safety limits of the control loop.
 */
#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

#define CTL_PERIOD_MS            (5U)     /* control loop period (200 Hz)             */
#define CTL_TEL_DECIMATION       (4U)     /* publish telemetry every 4 ticks          */
#define CTL_CMD_TIMEOUT_MS       (2000U)  /* no duty command this long -> SAFE_STOP   */

/* Bring-up safety limits (increase only after the wheel is verified). */
#define MOT_DEFAULT_MAX_DUTY     (300)    /* permille: 30 % of full PWM               */
#define MOT_DEFAULT_DEADBAND     (0)      /* permille: measure in M1 and set          */
#define MOT_DEFAULT_RAMP         (10)     /* permille per tick (2000 permille/s)      */
#define MOT_DEFAULT_DEADTIME     (4U)     /* ticks at zero on direction change        */

#endif /* CONTROL_CONFIG_H */
