# MPU6500 STM32 HAL 드라이버

STM32 HAL 기반의 **MPU6500** IMU 센서 제어용 드라이버입니다.  
SPI 통신을 이용하여 가속도, 자이로, 온도 데이터를 읽고, 오프셋 보정과 물리 단위 변환 기능을 제공합니다.

---

## 📂 프로젝트 구조
```
ProjectRoot/
├── App/
│   ├── device/        # 외부 디바이스(센서 등) 드라이버 폴더
│   │   ├── mpu6500.h  # MPU6500 센서 드라이버 헤더 (레지스터 정의, API 선언)
│   │   └── mpu6500.c  # MPU6500 센서 드라이버 구현부 (SPI 통신, 데이터 읽기/보정 함수)
│   └── ap.c           # 실제 동작을 위한 예제 코드
```

## Pin 배치
| 보드 실크 | 🔌 I2C 연결 | 🔌 SPI 연결 |
| --- | --- | --- |
| VCC | 3.3V 또는 5V | 3.3V 또는 5V |
| GND | GND | GND |
| SCL | **I2C SCL** | **SPI SCK (클럭)** |
| SDA | **I2C SDA** | **SPI MOSI (데이터 출력)** |
| EDA | 사용 안함 | 사용 안함 |
| ECL | 사용 안함 | 사용 안함 |
| ADO | I2C 주소 선택 (0x68/69) | **SPI MISO (데이터 입력)** |
| INT | (선택) 인터럽트 핀 | (선택) 인터럽트 핀 |
| NCS | 사용 안함 | **SPI CS (칩 선택)** |
| FSYNC | (선택) 프레임 동기화 | (선택) 프레임 동기화 |

---

## 📜 주요 기능

### 센서 초기화 및 설정

- WHO_AM_I 확인 및 초기화 (ID: 0x70)
- 가속도 범위 설정: ±2g, ±4g, ±8g, ±16g
- 자이로 범위 설정: ±250°/s, ±500°/s, ±1000°/s, ±2000°/s
- DLPF (Digital Low Pass Filter) 설정

### 데이터 읽기

- 가속도/자이로/온도 원시 데이터 읽기 (16비트)
- 물리 단위 변환 (g, °/s, ℃)
- 오프셋 보정 기능

### 필터 설정

- 자이로 DLPF: 41Hz (CONFIG 레지스터)
- 가속도 DLPF: 184Hz (ACCEL_CONFIG2 레지스터)
- 샘플링 레이트: 125Hz (SMPLRT_DIV = 0x07)

---

## 작업 순서

- void MPU6500_OffsetMeasure(void); 코드로 오차를 측정
- static void MPU6500_Calibrate(void) 코드에 오차를 기록하고 Init()에서 오차를 제거하도록 작성

---

## 원시 데이터 읽기

```
void MPU6500_ReadAccel(int16_t accel[3]);
void MPU6500_ReadGyro(int16_t gyro[3]);
```

## 물리 단위 변환

```
void MPU6500_ReadAccelScaled(float accel[3]);
void MPU6500_ReadGyroScaled(float gyro[3]);
float MPU6500_ReadTemperature(void);
```

## 오차 보정
```
void MPU6500_OffsetMeasure(void);
```

## 결과
![alt text](../../img/mpu6500_data.png)