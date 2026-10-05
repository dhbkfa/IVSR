///*
// * motor_encoder.h
// *
// *  Created on: Oct 4, 2026
// *      Author: hieule
// */
//
//#ifndef INC_MOTOR_ENCODER_H_
//#define INC_MOTOR_ENCODER_H_
//#include <stdint.h>
//#include "main.h"
//typedef struct{
//	int32_t velocity;	// Number of pulses in one sampling cycle
//	int64_t position;	// Sum pulses
//	uint32_t last_counter_value; // CNT lastest
//	uint8_t initialized;	// A unique Flag for each encoder
//}encoder_instance;
//typedef enum{
//	ENC_OK = 0,
//	ENC_ERR_NULL,
//	ENC_ERR_NOT_RUNNING,        // HAL_TIM_Encoder_Start has not been called yet
//	ENC_ERR_NOT_ENCODER_MODE,   // The timer is not encode mode
//	ENC_ERR_BAD_ARR,            // ARR is not 0xFFFF or 0xFFFFFFFF
//	ENC_WARN_OVERSPEED          // A large change between the two calls may result in an incorrect answer
//
//}enc_status_t;
///* Determine and control the current and past encoder values */
//enc_status_t update_encoder(encoder_instance *e, TIM_HandleTypeDef *htim);
//enc_status_t reset_encoder(encoder_instance *e, TIM_HandleTypeDef *htim);
//int64_t encoder_get_position(const encoder_instance *e); // Read safety
//
//
//
//
//
//
//#endif /* INC_MOTOR_ENCODER_H_ */
