/*
 * vehicle_config.h
 * Motor/encoder/wheel parameters . Replace with MESURED values
 *
 *  Created on: Oct 5, 2026
 *      Author: hieule
 */

#ifndef CONFIG_VEHICLE_CONFIG_H_
#define CONFIG_VEHICLE_CONFIG_H_

/* measure! Turn the wheel by hand exactly 1 turn and read counts. */
#define VEH_ENC_PPR            (11.0f)   /* pulses per motor-shaft rev, per channel */
#define VEH_GEAR_RATIO         (30.0f)   /* motor revs per wheel rev                */
#define VEH_ENC_QUAD_FACTOR    (4.0f)    /* TI1+TI2 encoder mode counts x4          */
#define VEH_WHEEL_CPR          (VEH_ENC_PPR * VEH_ENC_QUAD_FACTOR * VEH_GEAR_RATIO)

#define VEH_ENC_DIRECTION      (1)       /* +1 or -1: flips counting direction      */



#endif /* CONFIG_VEHICLE_CONFIG_H_ */
