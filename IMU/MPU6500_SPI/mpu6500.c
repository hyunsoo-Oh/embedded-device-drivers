/*
 * mpu6500.c
 *
 *  Created on: Aug 11, 2025
 *      Author: jweli
 */

#include "mpu6500.h"

#include "spi.h"
#include "usart.h"
/*
 *  📋 MPU-6500 주요 레지스터 설정값 정리
 *
 *  레지스터 이름   	  | 설명                   	| 주요 필드              		| 참고/의미
 *  ------------------|-------------------------|---------------------------|------------------------------
 *  CONFIG            | 자이로 DLPF 설정          	| DLPF_CFG (0~7)          	| 대역폭/지연 설정 (자이로 기준)
 *  GYRO_CONFIG       | 자이로 범위, DLPF 사용 유무	| FS_SEL, FCHOICE_B       	| FS_SEL: 감도 범위, FCHOICE_B: DLPF 우회
 *  ACCEL_CONFIG      | 가속도 범위 설정            | AFS_SEL (0~3)           	| ±2g, ±4g, ±8g, ±16g
 *  ACCEL_CONFIG2     | 가속도 DLPF 설정          	| ACCEL_FCHOICE_B, DLPF_CFG	| 고속 필터 설정 포함
 *  SMPLRT_DIV        | 샘플링 속도 제어           	| 값: 0~255               	| Sample Rate = Gyro Output Rate / (1 + div)
 *
 * 	DLPF_CFG 값 	| 대역폭 (Bandwidth) 		| 지연 시간 (Delay) 	| Gyro 출력 주기 	| DLPF 상태
 *  ------------| --------------------- | ----------------- | ------------- | ----------
 *    0x00      | 250Hz              	|   0.97ms          |    8kHz       | 비활성화
 *    0x01      | 184Hz              	|   2.9ms           |    1kHz       | 활성화
 *    0x02      | 92Hz               	|   3.9ms           |    1kHz       | 활성화
 *    0x03      | 41Hz               	|   5.9ms           |    1kHz       | 활성화
 *    0x04      | 20Hz               	|   9.9ms           |    1kHz       | 활성화
 *    0x05      | 10Hz               	|   17.85ms         |    1kHz       | 활성화
 *    0x06      | 5Hz                	|   33.48ms         |    1kHz       | 활성화
 *    0x07      | 3600Hz (GYRO only) 	|   0.17ms          |    8kHz       | 비활성화
 *
 *  ACCEL_DLPF_CFG 값 |  대역폭 (Bandwidth)	| 지연 시간 (Delay) | 출력 주기 (Output Rate) | 비고
 * ------------------ |---------------------|---------------- |---------------------- |-----------------------------
 *       0x00         | 460Hz               | 1.94ms          | 1kHz                  | DLPF 활성화
 *       0x01         | 184Hz               | 5.80ms          | 1kHz                  | DLPF 활성화
 *       0x02         | 92Hz                | 7.80ms          | 1kHz                  | DLPF 활성화
 *       0x03         | 41Hz                | 11.80ms         | 1kHz                  | DLPF 활성화
 *       0x04         | 20Hz                | 19.80ms         | 1kHz                  | DLPF 활성화
 *       0x05         | 10Hz                | 35.70ms         | 1kHz                  | DLPF 활성화
 *       0x06         | 5Hz                 | 66.96ms         | 1kHz                  | DLPF 활성화
 *       0x07         | 460Hz               | 1.94ms          | 1kHz                  | DLPF 비활성화 (ACCEL_FCHOICE_B = 1)
 *
 *  ✅ 조정 팁
 *  - DLPF를 활성화하면 자이로는 1kHz, 비활성화하면 8kHz 출력
 *  - ACCEL_FCHOICE_B가 0이면 DLPF 활성, 1이면 DLPF 비활성 (고속)
 *  - CONFIG와 ACCEL_CONFIG2는 노이즈 필터링에 매우 중요
 *  - SMPLRT_DIV는 센서 데이터 속도를 줄이거나 맞출 때 사용
 */

/*
 * 	자이로 범위 : ±250°/s, ±500°/s, ±1000°/s, ±2000°/s
 * 	가속도 범위 : ±2g, 	±4g, 	 ±8g, 	   ±16g
 *
 */

#define SPI_BUS			&hspi1
#define SPI_NCS_PORT 	GPIOB
#define SPI_NCS_PIN 	GPIO_PIN_6

#define UART_BUS		&huart2

// 감도 설정 및 변환값
MPU6500_Config_t cfg = {
	.accel_fs_sel 	= _2g,   		// 가속도 범위 : ±2g, ±4g, ±8g, ±16g
	.accel_sens 	= 0.0f,       	// 가속도 LSB/g (예: 16384.0f)
	.gyro_fs_sel 	= _250dps,    	// 자이로 범위 : ±250°/s, ±500°/s, ±1000°/s, ±2000°/s
	.gyro_sens 		= 0.0f        	// 자이로 LSB/°/s (예: 131.0f)
};

//static int16_t accel_offset[3] 	= {0, 0, 0};  // X, Y, Z 가속도 오프셋
//static int16_t gyro_offset[3]	= {0, 0, 0};  // X, Y, Z 자이로 오프셋
int16_t accel_offset[3] 	= {0, 0, 0};  // X, Y, Z 가속도 오프셋
int16_t gyro_offset[3]	= {0, 0, 0};  // X, Y, Z 자이로 오프셋

static void MPU6500_SPIWrite(uint8_t reg, uint8_t data)
{
    uint8_t tx_data[2];

    tx_data[0] = reg & 0x7F;  	// Write 명령 (MSB = 0)
    tx_data[1] = data;			// 보낼 실제 데이터

    HAL_GPIO_WritePin(SPI_NCS_PORT, SPI_NCS_PIN, GPIO_PIN_RESET);  	// CS LOW
    HAL_SPI_Transmit(SPI_BUS, tx_data, 2, 100);				// 2바이트 전송: [Register Address, Data]
    HAL_Delay(1);
    HAL_GPIO_WritePin(SPI_NCS_PORT, SPI_NCS_PIN, GPIO_PIN_SET);    	// CS HIGH
}
static void MPU6500_SPIRead(uint8_t reg, uint8_t *data, uint8_t length)
{
	   uint8_t tx_reg = reg | 0x80;  // Read 명령 (MSB = 1)

	    HAL_GPIO_WritePin(SPI_NCS_PORT, SPI_NCS_PIN, GPIO_PIN_RESET);  	// CS LOW
	    HAL_SPI_Transmit(SPI_BUS, &tx_reg, 1, 100);				// 1바이트 전송: 읽을 레지스터 주소 (Read 명령 포함)
	    HAL_SPI_Receive(SPI_BUS, data, length, 100);			// 지정한 길이만큼 데이터를 수신 (Read)
	    HAL_Delay(1);
	    HAL_GPIO_WritePin(SPI_NCS_PORT, SPI_NCS_PIN, GPIO_PIN_SET);    	// CS HIGH
}
static void MPU6500_SetSensitivity(void)
{
	// 가속도 감도 설정 (LSB/g)
	switch(cfg.accel_fs_sel)
	{
		case _2g:	cfg.accel_sens = 16384.0f; break;
		case _4g:	cfg.accel_sens = 8192.0f;  break;
		case _8g:	cfg.accel_sens = 4096.0f;  break;
		case _16g:	cfg.accel_sens = 2048.0f;  break;
	}

	// 자이로 감도 설정 (LSB/°/s)
	switch(cfg.gyro_fs_sel)
	{
		case _250dps:	cfg.gyro_sens = 131.0f;   break;
		case _500dps:	cfg.gyro_sens = 65.5f;    break;
		case _1000dps:	cfg.gyro_sens = 32.8f;    break;
		case _2000dps:	cfg.gyro_sens = 16.4f;    break;
	}
}

static void MPU6500_Calibrate(void)
{
	switch (cfg.accel_fs_sel)
	{
		case _2g:
			accel_offset[0] = -1374;	accel_offset[1] = -92;		accel_offset[2] = 146;
			break;
		case _4g:
			accel_offset[0] = -688;		accel_offset[1] = -48;		accel_offset[2] = 70;
			break;
		case _8g:
			accel_offset[0] = -344;		accel_offset[1] = -23;		accel_offset[2] = 36;
			break;
		case _16g:
			accel_offset[0] = -172;		accel_offset[1] = -11;		accel_offset[2] = 19;
			break;
	}
	switch (cfg.gyro_fs_sel)
	{
		case _250dps:
			gyro_offset[0] = 46;	gyro_offset[1] = 316;	gyro_offset[2] = -38;
			break;
		case _500dps:
			gyro_offset[0] = 23;	gyro_offset[1] = 157;	gyro_offset[2] = -18;
			break;
		case _1000dps:
			gyro_offset[0] = 11;	gyro_offset[1] = 79;	gyro_offset[2] = -9;
			break;
		case _2000dps:
			gyro_offset[0] = 6;		gyro_offset[1] = 39;	gyro_offset[2] = -4;
			break;
	}
}
void MPU6500_Init(void)
{
	MPU6500_SPIWrite(MPU6500_PWR_MGMT_1, 0x01);	 // Auto select clock source
	MPU6500_SPIWrite(MPU6500_PWR_MGMT_2, 0x00);  // 모든 축 활성화
	HAL_Delay(100);

	uint8_t who;
	MPU6500_SPIRead(MPU6500_WHO_AM_I, &who, 1);

	if (who == 0x70)
		HAL_UART_Transmit(UART_BUS, (uint8_t*)"Success\r\n", sizeof("Success\r\n") - 1, 100);
	else
		HAL_UART_Transmit(UART_BUS, (uint8_t*)"Failed\r\n", sizeof("Failed\r\n") - 1, 100);

	// 센서 구성 설정 (0x03, 0x01, 0x07)
	// Gyro Filter (5.9ms), Accel Filter (2.0ms) -> 출력 주기 = 8ms마다 한 번 (이전 시점에서 약간 지연된 데이터)
	MPU6500_SPIWrite(MPU6500_CONFIG, 0x03);			// DLPF 41Hz (노이즈 필터링 강화)
	MPU6500_SPIWrite(MPU6500_ACCEL_CONFIG2, 0x01);	// DLPF 184Hz (노이즈 필터링 강화)
	MPU6500_SPIWrite(MPU6500_SMPLRT_DIV, 0x07);		// 샘플링 레이트 125Hz (1kHz / (1+7))

	MPU6500_SPIWrite(MPU6500_ACCEL_CONFIG, cfg.accel_fs_sel << 3);
	MPU6500_SPIWrite(MPU6500_GYRO_CONFIG, cfg.gyro_fs_sel << 3);

	MPU6500_SetSensitivity();
	MPU6500_Calibrate();
	HAL_Delay(10);
}

// 데이터 읽기 함수 (Raw 값)
void MPU6500_ReadAccel(int16_t accel[3])
{
	uint8_t buff[6];
	MPU6500_SPIRead(MPU6500_ACCEL_XOUT_H, buff, 6);

	accel[0] = (int16_t)((buff[0] << 8) | buff[1]);
	accel[1] = (int16_t)((buff[2] << 8) | buff[3]);
	accel[2] = (int16_t)((buff[4] << 8) | buff[5]);
}
void MPU6500_ReadGyro(int16_t gyro[3])
{
	uint8_t buff[6];
	MPU6500_SPIRead(MPU6500_GYRO_XOUT_H, buff, 6);

	gyro[0] = (int16_t)((buff[0] << 8) | buff[1]);
	gyro[1] = (int16_t)((buff[2] << 8) | buff[3]);
	gyro[2] = (int16_t)((buff[4] << 8) | buff[5]);
}
float MPU6500_ReadTemperature(void)
{
	uint8_t buff[2];
	uint8_t temp;
	MPU6500_SPIRead(MPU6500_TEMP_OUT_H, buff, 2);

	temp = (int16_t)((buff[0] << 8) | buff[1]);
	return ((float)temp / 340.f) + 36.53f;
}

// 데이터 읽기 함수 (실제 물리값)
void MPU6500_ReadAccelScaled(float accel[3])
{
	int16_t raw_data[3];
	MPU6500_ReadAccel(raw_data);

	accel[0] = (float)(raw_data[0] - accel_offset[0]) / cfg.accel_sens;
	accel[1] = (float)(raw_data[1] - accel_offset[1]) / cfg.accel_sens;
	accel[2] = (float)(raw_data[2] - accel_offset[2]) / cfg.accel_sens;
}
void MPU6500_ReadGyroScaled(float gyro[3])
{
	int16_t raw_data[3];
	MPU6500_ReadGyro(raw_data);

	gyro[0] = (float)(raw_data[0] - gyro_offset[0]) / cfg.gyro_sens;
	gyro[1] = (float)(raw_data[1] - gyro_offset[1]) / cfg.gyro_sens;
	gyro[2] = (float)(raw_data[2] - gyro_offset[2]) / cfg.gyro_sens;
}

void MPU6500_OffsetMeasure(void)
{
    int32_t accel_sum[3] 	= {0, 0, 0};
    int32_t gyro_sum[3]		= {0, 0, 0};
    int16_t accel_data[3];
    int16_t gyro_data[3];
    const int samples = 1000;  // 캘리브레이션 샘플 수

    int16_t expected_z = 0;
    switch (cfg.accel_fs_sel) {
        case _2g: 	expected_z = 16384; break; // ±2g
        case _4g: 	expected_z = 8192;  break; // ±4g
        case _8g: 	expected_z = 4096;  break; // ±8g
        case _16g: 	expected_z = 2048;  break; // ±16g
        default:   	expected_z = 4096;  break;
    }

    // 안정화를 위한 대기
    HAL_Delay(1000);

    // 여러 샘플을 읽어서 평균값 계산
    for (int i = 0; i < samples; i++)
    {
    	MPU6500_ReadAccel(accel_data);
    	MPU6500_ReadGyro(gyro_data);

        // 가속도 데이터 누적
        accel_sum[0] += accel_data[0];
        accel_sum[1] += accel_data[1];
        accel_sum[2] += accel_data[2];

        // 자이로 데이터 누적
        gyro_sum[0] += gyro_data[0];
        gyro_sum[1] += gyro_data[1];
        gyro_sum[2] += gyro_data[2];

        HAL_Delay(10);  // 샘플 간격
    }
    // 평균값 계산하여 오프셋으로 설정
    accel_offset[0] = accel_sum[0] / samples;
    accel_offset[1] = accel_sum[1] / samples;
    accel_offset[2] = accel_sum[2] / samples - expected_z; // Z축은 중력(1g) 보정

    gyro_offset[0] = gyro_sum[0] / samples;
    gyro_offset[1] = gyro_sum[1] / samples;
    gyro_offset[2] = gyro_sum[2] / samples;
}
