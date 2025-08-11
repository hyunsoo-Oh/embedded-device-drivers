/*
 * motor_encoder.c
 *
 *  Created on: Aug 11, 2025
 *      Author: USER
 */

#include "motor_encoder.h"
#include "tim.h"

#define MOTOR_C1_PORT		GPIOC
#define MOTOR_C2_PORT		GPIOC

#define MOTOR_C1_PIN		GPIO_PIN_0
#define MOTOR_C2_PIN		GPIO_PIN_1

#define MOTOR_TIM			&htim1
#define ENCODER_TIM			&htim2

#define PPR					11
#define GEAR_RATIO			30

static volatile float s_rpm = 0.0f;

void MOTOR_Init(void)
{
	HAL_TIM_PWM_Start(MOTOR_TIM, TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(ENCODER_TIM, TIM_CHANNEL_ALL);
	__HAL_TIM_SET_COUNTER(ENCODER_TIM, 0);

	HAL_GPIO_WritePin(MOTOR_C1_PORT, MOTOR_C1_PIN, RESET);
	HAL_GPIO_WritePin(MOTOR_C2_PORT, MOTOR_C2_PIN, RESET);
}

//   입력값 제한: u는 -1.0 ~ +1.0 범위만 허용
//   - 부호(sign) → 회전 방향
//   - 절댓값(magnitude) → 속도 비율(PWM 듀티)
void MOTOR_SetDuty(float u)
{
    if (u > 1.0f)  u = 1.0f;
    if (u < -1.0f) u = -1.0f;

    GPIO_PinState in1, in2;
    float mag = (u >= 0) ? u : -u;

    if (u >= 0) { in1 = GPIO_PIN_SET;   in2 = GPIO_PIN_RESET; }
    else        { in1 = GPIO_PIN_RESET; in2 = GPIO_PIN_SET;   }

    HAL_GPIO_WritePin(MOTOR_C1_PORT, MOTOR_C1_PIN, in1);
    HAL_GPIO_WritePin(MOTOR_C2_PORT, MOTOR_C2_PIN, in2);

    // 한 주기 카운트 수 * 듀티 비율 + float → int 변환 시 반올림을 위한 상수
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(MOTOR_TIM);
    uint32_t ccr = (uint32_t)((arr + 1) * mag + 0.5f);
    __HAL_TIM_SET_COMPARE(MOTOR_TIM, TIM_CHANNEL_1, ccr);
}

int32_t ENCODER_GetCount(void)
{
    return (int32_t)__HAL_TIM_GET_COUNTER(ENCODER_TIM);
}

float ENCODER_GetRPM(void)
{
    return s_rpm;
}

// 1kHz(=1ms) 주기에서 호출해 속도 계산
void ENCODER_Update(void)
{
    static int32_t prev = 0;
    int32_t now  = ENCODER_GetCount();
    int32_t diff = now - prev;
    prev = now;

    const float CPR = PPR * 4.0f * GEAR_RATIO;
    const float Ts  = 0.01f;  // 1ms
    float rps  = (float)diff / (CPR * Ts);
    s_rpm = rps * 60.0f;
}
