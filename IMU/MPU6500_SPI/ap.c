/*
 * ap.c
 *
 *  Created on: Aug 10, 2025
 *      Author: jweli
 */

#include "ap.h"
#include "usart.h"

#include "mpu6500.h"

float accel[3], gyro[3];
float temp;

char msg[128];

void apInit(void)
{
	MPU6500_Init();
}

void apMain(void)
{
	while (1)
	{
		MPU6500_ReadAccelScaled(accel);
		MPU6500_ReadGyroScaled(gyro);
		temp = MPU6500_ReadTemperature();

		snprintf(msg, sizeof(msg),
			 "ACC: X=%.2f Y=%.2f Z=%.2f | GYRO: X=%.2f Y=%.2f Z=%.2f | TEMP=%.2f\r\n",
			 accel[0], accel[1], accel[2],
			 gyro[0], gyro[1], gyro[2],
			 temp
		);

		HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);

		HAL_Delay(100);
	}
}
