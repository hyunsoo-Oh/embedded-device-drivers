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
	MOTOR_CTRL_Init();
	ENCODER_Update();

	HAL_Delay(1000);

	MOTOR_Angle_AbsDeg(-90.0f);
}

void apMain(void)
{
	uint32_t t10 = HAL_GetTick();   // 10ms 제어 루프
	uint32_t t100 = HAL_GetTick();  // 100ms 로깅

	while (1)
	{
		if ((int32_t)(HAL_GetTick() - t10) >= 10)
		{
			t10 += 10;
			ENCODER_Update();       // 1) 센서 갱신
			MOTOR_Angle_Step();     // 2) 속도 제어
		}
		if ((int32_t)(HAL_GetTick() - t100) >= 100)
		{
			t100 += 100;
			int32_t count = ENCODER_GetCount();
			float angle = ENCODER_GetAngleDeg();
			snprintf(msg, sizeof(msg), "Count: %ld, Angle: %.2f\r\n", (long)count, angle);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
		}
	}
}

//void Read_RPM(void)
//{
//	while (1)
//	{
//		MOTOR_SetDuty(1);
//
//        // 현재 카운트와 RPM 읽기
//        int32_t count = ENCODER_GetCount();
//        float rpm = ENCODER_GetRPM();
//
//        snprintf(msg, sizeof(msg), "Count: %ld, RPM: %.2f\r\n", (long)count, rpm);
//
//		HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
//		HAL_Delay(100);
//	}
//}

//void Set_Speed()
//{
//	void apInit(void)
//	{
//		MOTOR_Init();
//		MOTOR_CTRL_Init();
//
//		MOTOR_SetRPM(200);
//	}
//
//	void apMain(void)
//	{
//	    uint32_t t10 = HAL_GetTick();   // 10ms 제어 루프
//	    uint32_t t100 = HAL_GetTick();  // 100ms 로깅
//
//		while (1)
//		{
//			if ((int32_t)(HAL_GetTick() - t10) >= 10)
//			{
//				t10 += 10;
//				ENCODER_Update();       // 1) 센서 갱신
//				MOTOR_Speed_Step();     // 2) 속도 제어
//			}
//			if ((int32_t)(HAL_GetTick() - t100) >= 100)
//			{
//				t100 += 100;
//				int32_t count = ENCODER_GetCount();
//				float rpm = ENCODER_GetRPM();
//				snprintf(msg, sizeof(msg), "Count: %ld, RPM: %.2f\r\n", (long)count, rpm);
//				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
//			}
//		}
//	}
//}

//void Set_Angle()
//{
//	void apInit(void)
//	{
//		MOTOR_Init();
//		MOTOR_CTRL_Init();
//		ENCODER_Update();
//
//		HAL_Delay(1000);
//
//		MOTOR_Angle_AbsDeg(90.0f);
//	}
//
//	void apMain(void)
//	{
//	    uint32_t t10 = HAL_GetTick();   // 10ms 제어 루프
//	    uint32_t t100 = HAL_GetTick();  // 100ms 로깅
//
//		while (1)
//		{
//			if ((int32_t)(HAL_GetTick() - t10) >= 10)
//			{
//				t10 += 10;
//				ENCODER_Update();       // 1) 센서 갱신
//				MOTOR_Angle_Step();     // 2) 속도 제어
//			}
//			if ((int32_t)(HAL_GetTick() - t100) >= 100)
//			{
//				t100 += 100;
//				int32_t count = ENCODER_GetCount();
//				float angle = ENCODER_GetAngleDeg();
//				snprintf(msg, sizeof(msg), "Count: %ld, Angle: %.2f\r\n", (long)count, angle);
//				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
//			}
//		}
//	}
//}
