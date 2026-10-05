/*
 * motor_encoder.c
 *
 *  Created on: Oct 4, 2026
 *      Author: hieule
 */
#include "motor_encoder.h"
/* Update motor velocity and position current*/
void update_encoder(encoder_instance *encoder_value, TIM_HandleTypeDef *htim)
{
	uint32_t now = __HAL_TIM_GET_COUNTER(htim);
	if(!encoder_value->initialized) 	// First time : Just remember
	{
		encoder_value->last_counter_value = now;
		encoder_value->velocity = 0;
		encoder_value->initialized = 1;
		return;
	}
	else
	{
		if(temp_counter == encoder_value->last_counter_value)
		{
			encoder_value->velocity = 0;
		}
		else if (temp_counter > encoder_value -> last_counter_value)
		{
			if(__HAL_TIM_IS_TIM_COUNTING_DOWN(htim))
			{
				// overflow ARR , counter > ARR
				encoder_value->velocity = encoder_value->last_counter_value -
						(__HAL_TIM_GET_AUTORELOAD(htim) - temp_counter);
			}
			else
			{
				encoder_value->velocity = temp_counter - encoder_value->last_counter_value;


			}

		}
		else
		{
			if(__HAL_TIM_IS_TIM_COUNTING_DOWN(htim))
			{
				// overflow ARR
				encoder_value->velocity = temp_counter - encoder_value->last_counter_value;

			}
			else
			{
				encoder_value->velocity = temp_counter +
										(__HAL_TIM_GET_AUTORELOAD(htim) - encoder_value->last_counter_value);

			}
		}
	}
	encoder_value->position += encoder_value->velocity;
	encoder_value->last_counter_value = temp_counter;

}
void reset_encoder(encoder_instance *encoder_value, TIM_HandleTypeDef *htim)
{
	encoder_value->velocity =0;
	encoder_value->position = 0;
	encoder_value->last_counter_value = 0;
}
