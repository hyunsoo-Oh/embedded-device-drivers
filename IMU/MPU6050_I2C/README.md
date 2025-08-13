# MPU6050 STM32 HAL 드라이버

STM32 HAL 기반의 **MPU6050** IMU 센서 제어용 드라이버입니다.  
I2C 통신을 이용하여 가속도, 자이로, 온도 데이터를 읽고, 오프셋 보정과 물리 단위 변환 기능을 제공합니다.

---

## 📂 프로젝트 구조
```
ProjectRoot/
├── App/
│   ├── device/        # 외부 디바이스(센서 등) 드라이버 폴더
│   │   ├── mpu6050.h  # MPU6050 센서 드라이버 헤더 (레지스터 정의, API 선언)
│   │   └── mpu6050.c  # MPU6050 센서 드라이버 구현부 (I2C 통신, 데이터 읽기/보정 함수)
│   └── ap.c           # 실제 동작을 위한 예제 코드
```

---

## 📜 주요 기능

- WHO_AM_I 확인 및 초기화
- 가속도/자이로/온도 원시 데이터 읽기
- 오프셋 보정
- 물리 단위 변환 (g, °/s, ℃)
- FIFO, 인터럽트 설정 지원

---

## 작업 순서

- void MPU6050_OffsetMeasure(void); 함수를 사용해 오차를 측정
- void MPU6050_Calibrate(void); 측정된 오차를 Switch에 기록하고 실행

---

## 원시 데이터 읽기

```
void MPU6050_ReadAccel(int16_t ac_data[3]);
void MPU6050_ReadGyro(int16_t gy_data[3]);
void MPU6050_ReadTemp(int16_t *te_data);
```

## 물리 단위 변환

```
void  MPU6050_GetAccelG(float accel_g[3]);     // [g]
void  MPU6050_GetGyroDPS(float gyro_dps[3]);   // [°/s]
float MPU6050_GetTempCelsius(void);            // [℃]
```

## 오차 보정

```
void MPU6050_Calibrate(void);
void MPU6050_OffsetMeasure(void);
```

## 결과
![alt text](../../img/mpu6050_data.png)