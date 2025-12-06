# 🐶 Project Smart Pet – 로봇 반려견

> Pixy2 카메라, 초음파 센서, 블루투스, 도트 LED, 서보모터, MP3 모듈을 활용해  
> 사용자를 따라다니고 소리·표정·꼬리로 반응하는 로봇 반려견 캡스톤 프로젝트

Arduino 기반으로 **타깃 추적 + 거리 유지 + 앱 조종 + 감정 표현**을 한 번에 구현한 프로젝트입니다.  
이 프로젝트로 학교 **캡스톤 디자인 우수상**을 수상했습니다.

---

## 🧾 Project Info

| 항목       | 내용                                                                 |
|------------|----------------------------------------------------------------------|
| 기간       | **2024.09 ~ 2024.12**                                               |
| 유형       | 대학교 캡스톤 디자인                                                |
| 수상       | 캡스톤 디자인 **🏆우수상**                                            |
| 역할       | 펌웨어 메인 개발, 센서·모터 제어, Bluetooth 연동, 시스템 통합       |
| 한 줄 요약 | “사물을 인식하고 일정 거리를 유지하며 따라오는 스마트 로봇 반려견” |

---

## 🛠 Tech Stack

### 🔹 Languages & MCU

- C / C++ (Arduino)
- Arduino Mega 2560

### 🔹 Sensors & Actuators

- Pixy2 Camera – 색/물체 인식
- Ultrasonic Sensor (HC-SR04) – 거리 측정
- DC Motor + L298N Motor Driver – 이동 제어
- Servo Motor – 꼬리 움직임
- Dot Matrix LED – 눈/표정 출력
- DFPlayer Mini + Speaker – 짖는 소리/효과음 재생

### 🔹 Communication & App

- HC-06 Bluetooth – 스마트폰 앱과 통신
- App Inventor – Android 조종 앱 제작

---

## ✨ Key Features

- 🎯 **Target Following**  
  Pixy2 카메라로 특정 색/타깃을 인식하고, 초음파 센서로 거리를 측정해  
  약 **50cm 전후를 유지**하며 전진/후진/회전을 제어합니다.

- 🎮 **Bluetooth Remote Control**  
  App Inventor로 만든 앱에서  
  `전진 / 후진 / 좌회전 / 우회전 / 제자리 회전 / 정지` 및  
  `따라와 / 꼬리 흔들어 / 짖어 / 일어나 / 잠자기` 같은 동작 명령을 전송합니다.

- 😄 **Emotional Expression**  
  도트 LED로 눈/표정을 바꾸고, 서보 모터로 꼬리를 흔들며,  
  DFPlayer Mini로 짖는 소리를 재생해 반응합니다.

- ⚙️ **Stable Power Design**  
  여러 모듈이 동시에 동작할 때 전압 강하 문제를 해결하기 위해  
  **모터 전원과 로직 전원을 분리한 전원 설계**를 적용했습니다.

---

## 🧩 System Overview

```text
[사용자]
   └─ 스마트폰 앱 (App Inventor)
        └─ Bluetooth (HC-06)
             └─ Arduino Mega (메인 제어)
                  ├─ Pixy2 Camera        : 타깃 인식 (X 위치, 크기)
                  ├─ Ultrasonic Sensor   : 거리 측정
                  ├─ L298N Motor Driver  : 좌/우 모터 제어
                  ├─ Dot Matrix LED      : 눈/표정 출력
                  ├─ Servo Motor (Tail)  : 꼬리 움직임
                  └─ DFPlayer Mini       : 사운드 출력
