/*
 * hw.c
 *
 *  Created on: Aug 10, 2025
 *      Author: jweli
 */

#include "hw.h"
#include "tim.h"

#include "motor_encoder.h"

void hwInit(void)
{
	HAL_TIM_Base_Start_IT(&htim10);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM10)
	{
		ENCODER_Update();
	}
}
