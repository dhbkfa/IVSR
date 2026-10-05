/*
 * encoder.c
 *
 *  Created on: Oct 5, 2026
 *      Author: hieule
 */
#include "encoder.h"

#define ENC_TWO_PI		(6.28318531f)
#define ENC_DEG_PER_REV	(360.0f)

static int32_t sat_i64_to_i32(int64_t v)
{
	int32_t r;
	if(v > (int64_t)INT32_MAX){
		r = INT32_MAX;
	}else if (v < (int64_t)(-INT32_MAX)) {
		r = -INT32_MAX;
	}else {
		r = (int32_t)v;
	}
	return r;
}

status_t enc_init(enc_t *e,const enc_cfg_t *cfg){
	status_t st = ST_OK;
	if(e == NULL || (cfg == NULL)){
		st = ST_ERR_PARAM;
	}

}

