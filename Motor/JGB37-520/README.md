# JGB37-520 STM32 HAL 드라이버

STM32 HAL 기반의 JGB37-520 엔코더 모터 제어용 드라이버입니다.  
STM32의 Encoder Mode와 L298D 모터 드라이버를 활용하여 엔코더 모터 제어했습니다.

Encoder의 Pulse 신호를 사용해 모터의 회전 상태를 1ms 마다 받습니다.  
받는 신호를 통해 회전 속도, 회전 방향, 누적 거리를 측정 가능합니다.  
자율 주행 차량에서 특정 거리만큼 이동하거나 속도를 유지해야 할 때 유용합니다.

## 📂 프로젝트 구조
```
ProjectRoot/
├── App/
│   ├── device/        # 외부 디바이스(센서 등) 드라이버 폴더
│   │   ├── motor_encoder.h
│   │   └── motor_encoder.c
│   └── ap.c           # 실제 동작을 위한 예제 코드
```

## 

## 📜 데이터 시트

![alt text](../../img/JGB37-520.png)
![alt text](../../img/Encoder.png)

출처 : https://www.aslongdcmotor.com/sale-14417223-aslong-jgb37-520gb-electric-dc-gear-motor-hall-encoder-1600rpm.html  
출처 : https://ko.aliexpress.com/item/1005005437166978.html?  

---

## CubeMX 설정
### JGB37-520 설정
```
Motor Supply Voltage : 12V
Encoder Supply Voltage : 3.3V / 5V
```

#### TIM 32bit
```
Mode: Combined Channels > Encoder Mode

Parameter Settings:
- Encoder Mode: TI1 and TI2
- Counter Mode: Up
- Counter Period: 0xFFFFFFFF (32bit 최대값)
- Prescaler: 0
- Polarity: Rising Edge

NVIC: Global Interrupt Enable
```
#### 모터 PWM 제어용
```
Mode: 
- Channel1: PWM Generation CH1
- Channel2: PWM Generation CH2  

Parameter Settings:
- Prescaler: 899 (APB2 클럭이 180MHz일 때)
- Counter Mode: Up
- Counter Period: 999 (1kHz PWM 주파수)
- Pulse: 0 (초기 듀티사이클 0%)
- PWM Mode: PWM mode 1
```

#### L298N 제어를 위한 추가 GPIO 설정
```
GPIO Output pins:
- PB0: Motor1_DIR1 (IN1)
- PB1: Motor1_DIR2 (IN2)

GPIO Mode: Output Push Pull
GPIO Pull-up/Pull-down: No pull-up and no pull-down
Maximum output speed: High
```

## Encoder 모터 원리

```
2배 증배 (Rising Edge만):
A: ___┌─────┐_______┌─────┐___
B: ______┌─────┐_______┌─────┐_
        1     2       3     4
        ↑     ↑       ↑     ↑
      A상승  B상승    A상승  B상승

4배 증배 (Rising + Falling Edge):
A: ___┌─────┐_______┌─────┐___
B: ______┌─────┐_______┌─────┐_
         1 2 3 4       5 6 7 8
      ↑  ↑  ↓  ↓    ↑  ↑  ↓  ↓
       모든 엣지에서 카운트
```

### JGB37-520 Encoder Specification

- 동작 전압 : 12V  
- 기어비 옵션 : 
- 엔코더 해상도 : 모터 1회전 당 -> 11 Pulse x 기어비 x 4채배

