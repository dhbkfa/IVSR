/*
 * encoder.h
 * Quadrature encoder position/speed from a raw hardware counter.
 * Pure C: no HAL, no RTOS. Fully unit-testable on a PC.
 *  Created on: Oct 5, 2026
 *      Author: hieule
 */

#ifndef COMPONENTS_ENCODER_ENCODER_H_
#define COMPONENTS_ENCODER_ENCODER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "status.h"
typedef struct{
	uint8_t counter_bits;		/* 16 or 32 : width of hardware counter 				*/
	int8_t direction;			/* +1 or -1 : fligs the sign of the counting 			*/
	float count_per_rev;		/* counts per OUTPUT shaft rev(x4 and gearbox incl.)	*/
}enc_cfg_t;
typedef struct{
	enc_cfg_t 	cfg;
	uint32_t	last_raw;
	int32_t		total_counts;		/* saturating accumulator (range +-2.1e9 counts) 		*/
	int32_t		delta_counts;		/* counts during the last update						*/
	bool		intialized;			/* flag comfirm state motor								*/
}enc_t;
status_t enc_init(enc_t *e,const enc_cfg_t *cfg);
/**
 * Update with the raw counter value.
 * @return ST_OK, ST_WARN_OVERSPEED if |delta| > 1/4 of the counter range
 *         (result may be wrong), ST_ERR_PARAM on NULL.
 */
status_t enc_update(enc_t *e,uint32_t raw);

/** Zero the position and resynchronise with the current raw counter. */
void     enc_reset(enc_t *e, uint32_t raw_now);

/** Change counts-per-rev at run time (used while calibrating). */
status_t enc_set_counts_per_rev(enc_t *e, float cpr);

int32_t  enc_get_count(const enc_t *e);
int32_t  enc_get_delta(const enc_t *e);
float    enc_get_angle_deg(const enc_t *e);
float    enc_get_speed_rad_s(const enc_t *e, float dt_s);







#endif /* COMPONENTS_ENCODER_ENCODER_H_ */
