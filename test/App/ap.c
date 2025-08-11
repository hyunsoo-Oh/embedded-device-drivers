/*
 * ap.c
 *
 *  Created on: Aug 10, 2025
 *      Author: jweli
 */

#include "ap.h"
#include "usart.h"

#include "motor_encoder.h"

char msg[128];

void apInit(void)
{
	MOTOR_Init();
}

void apMain(void)
{
	while (1)
	{
		MOTOR_SetDuty(0.7);

        // 현재 카운트와 RPM 읽기
        int32_t count = ENCODER_GetCount();
        float rpm = ENCODER_GetRPM();

        snprintf(msg, sizeof(msg), "Count: %ld, RPM: %.2f\r\n", (long)count, rpm);

		HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
		HAL_Delay(100);
	}
}
