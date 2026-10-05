/**
 * @file    vehicle_config.h
 * @brief   Motor/encoder/wheel parameters. Replace with MEASURED values.
 */
#ifndef VEHICLE_CONFIG_H
#define VEHICLE_CONFIG_H

/* TODO(M1): measure! Turn the wheel by hand exactly 1 turn and read counts. */
#define VEH_ENC_PPR            (11.0f)   /* pulses per motor-shaft rev, per channel */
#define VEH_GEAR_RATIO         (30.0f)   /* motor revs per wheel rev                */
#define VEH_ENC_QUAD_FACTOR    (4.0f)    /* TI1+TI2 encoder mode counts x4          */
#define VEH_WHEEL_CPR          (VEH_ENC_PPR * VEH_ENC_QUAD_FACTOR * VEH_GEAR_RATIO)

#define VEH_ENC_DIRECTION      (1)       /* +1 or -1: flips counting direction      */

#endif /* VEHICLE_CONFIG_H */
