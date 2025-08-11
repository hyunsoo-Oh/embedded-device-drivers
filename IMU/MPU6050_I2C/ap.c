/*
 * ap.c
 *
 *  Created on: Aug 10, 2025
 *      Author: jweli
 */

#include "ap.h"
#include "usart.h"

#include "mpu6050.h"

float accel[3], gyro[3];
float temp;

char msg[128];

void apInit(void)
{
	MPU6050_Init();
}

void apMain(void)
{
	while (1)
	{
		MPU6050_GetAccelG(accel);
		MPU6050_GetGyroDPS(gyro);
		temp = MPU6050_GetTempCelsius();

		snprintf(msg, sizeof(msg),
			 "ACC: X=%f Y=%f Z=%f | GYRO: X=%f Y=%f Z=%f | TEMP=%.2f\r\n",
			 accel[0], accel[1], accel[2],
			 gyro[0], gyro[1], gyro[2],
			 temp
		);

		HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
	}
}
