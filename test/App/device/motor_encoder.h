/*
 * motor_encoder.h
 *
 *  Created on: Aug 11, 2025
 *      Author: USER
 */

#ifndef DEVICE_MOTOR_ENCODER_H_
#define DEVICE_MOTOR_ENCODER_H_

#include "def.h"

typedef struct {
    int32_t position;      // 현재 위치 (카운트)
    int32_t last_count;    // 이전 카운트 값
    float velocity_rpm;    // 속도 (RPM)
    float angle_degrees;   // 각도 (도)
} encoder_data_t;

void MOTOR_Init(void);

// -1.0 ~ +1.0 입력 (부호=방향, 크기=듀티)
void MOTOR_SetDuty(float u);

// 엔코더 카운트/속도 읽기
int32_t ENCODER_GetCount(void);
float   ENCODER_GetRPM(void);

void ENCODER_Update(void);

#endif /* DEVICE_MOTOR_ENCODER_H_ */
