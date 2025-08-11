/*
 * mpu6500.h
 *
 *  Created on: Aug 11, 2025
 *      Author: jweli
 */

#ifndef DEVICE_MPU6500_H_
#define DEVICE_MPU6500_H_

#include "def.h"

// 기본 I2C 주소 (AD0 핀 상태에 따라 결정)
#define MPU6500_I2C_ADDR     		0x68  // AD0 == LOW ? 0x68 : 0x69

// WHO_AM_I 레지스터
#define MPU6500_WHO_AM_I        	0x75  // 0x70

// 전원 관리 관련 레지스터
#define MPU6500_PWR_MGMT_1      	0x6B  // 슬립 모드 설정, 클럭 소스 선택
#define MPU6500_PWR_MGMT_2      	0x6C  // 개별 센서 전원 제어 (가속도/자이로 축별로 OFF 가능)

// 센서 구성 관련 레지스터
#define MPU6500_SMPLRT_DIV      	0x19  // 샘플링 레이트 분주기 (SampleRate = GyroRate / (1 + SMPLRT_DIV))
#define MPU6500_CONFIG          	0x1A  // DLPF 설정 (Digital Low Pass Filter)
#define MPU6500_GYRO_CONFIG     	0x1B  // 자이로 감도 범위 설정 (±250 ~ ±2000 dps)
#define MPU6500_ACCEL_CONFIG    	0x1C  // 가속도 감도 범위 설정 (±2g ~ ±16g)
#define MPU6500_ACCEL_CONFIG2   	0x1D  // 가속도용 DLPF 설정

// 가속도 출력 데이터 (16비트, 고/저 바이트 분리)
#define MPU6500_ACCEL_XOUT_H    	0x3B
#define MPU6500_ACCEL_XOUT_L    	0x3C
#define MPU6500_ACCEL_YOUT_H    	0x3D
#define MPU6500_ACCEL_YOUT_L    	0x3E
#define MPU6500_ACCEL_ZOUT_H    	0x3F
#define MPU6500_ACCEL_ZOUT_L    	0x40

// 온도 센서 출력
#define MPU6500_TEMP_OUT_H      	0x41
#define MPU6500_TEMP_OUT_L      	0x42
// 공식: Temperature in °C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.53

// 자이로 출력 데이터 (16비트, 고/저 바이트 분리)
#define MPU6500_GYRO_XOUT_H     	0x43
#define MPU6500_GYRO_XOUT_L     	0x44
#define MPU6500_GYRO_YOUT_H     	0x45
#define MPU6500_GYRO_YOUT_L     	0x46
#define MPU6500_GYRO_ZOUT_H     	0x47
#define MPU6500_GYRO_ZOUT_L     	0x48

// 인터럽트 관련 레지스터
#define MPU6500_INT_ENABLE      	0x38  // 인터럽트 활성화 설정
#define MPU6500_INT_STATUS      	0x3A  // 발생한 인터럽트 확인

// 외부 센서(예: AK8963 자기센서 (MPU-9250 시 내장) 접근용 레지스터
#define MPU6500_I2C_MST_CTRL    	0x24  // I2C 마스터 클럭 설정 등
#define MPU6500_I2C_SLV0_ADDR   	0x25  // 외부 슬레이브 주소
#define MPU6500_I2C_SLV0_REG    	0x26  // 슬레이브에서 읽을 레지스터 주소
#define MPU6500_I2C_SLV0_CTRL   	0x27  // 데이터 길이, 읽기/쓰기 설정
#define MPU6500_EXT_SENS_DATA_00	0x49  // 외부 센서로부터 읽은 데이터 시작 주소

// 사용자 제어
#define MPU6500_USER_CTRL      		0x6A  // FIFO, I2C 마스터 모드 등 기능 제어
#define MPU6500_SIGNAL_PATH_RESET	0x68 // 자이로/가속도/온도 신호 경로 리셋

typedef enum
{
	_250dps,	_500dps,	_1000dps,	_2000dps
} gscale_t;

typedef enum
{
	_2g,	_4g, 	_8g, 	_16g
} ascale_t;

typedef struct {
	ascale_t 	accel_fs_sel;
	gscale_t 	gyro_fs_sel;
	float		accel_sens;
	float 		gyro_sens;
} MPU6500_Config_t;

// 초기화 및 설정 함수
void MPU6500_Init(void);

// 데이터 읽기 함수 (Raw 값)
void MPU6500_ReadAccel(int16_t accel[3]);
void MPU6500_ReadGyro(int16_t gyro[3]);
float MPU6500_ReadTemperature(void);

// 데이터 읽기 함수 (실제 물리값)
void MPU6500_ReadAccelScaled(float accel[3]);
void MPU6500_ReadGyroScaled(float gyro[3]);

// 오차 관련 함수
void MPU6500_OffsetMeasure(void);

#endif /* DEVICE_MPU6500_H_ */
