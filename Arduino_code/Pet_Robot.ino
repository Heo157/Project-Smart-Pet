#include <SPI.h>
#include <Pixy2.h>
#include <LedControl.h>
#include <Servo.h>//빨간색 전원, 갈색 gnd
#include <DFRobotDFPlayerMini.h>

Pixy2 pixy;

// 블루투스와 MP3 모듈 핀 정의 (하드웨어 시리얼 사용)
#define BLUETOOTH_SERIAL Serial3
#define MP3_SERIAL Serial2

DFRobotDFPlayerMini myDFPlayer;

// 몸통 초음파 센서 핀 정의
#define TRIG1 30
#define ECHO1 32
// 머리 초음파 센서 핀 정의
#define TRIG2 40
#define ECHO2 42

// 모터 제어 핀 정의 (메가 보드 핀 배정)
// ENB, IN4, IN3, IN2, IN1, ENA
int myPins[] = {5, 6, 7, 8, 9, 10}; 
float deadZone = 0.15;      // 정밀 제어를 위한 데드존 값

// 도트 매트릭스 및 서보모터 설정
LedControl lc = LedControl(22, 24, 26, 1); // (DIN, CLK, CS, number of displays)
Servo myservo;

// 도트 매트릭스 모양
byte heart[] = {        //하트
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};
byte custom1[] = {      //웃음
  B00000000,
  B00011000,
  B00111100,
  B01100110,
  B11000011,
  B10000001,
  B00000000,
  B00000000
};
byte custom2[] = {      //감기
  B00000000,
  B00000000,
  B10000001, 
  B11000011,
  B01100110,
  B00111100,
  B00011000,
  B00000000
};
byte custom3[] = {      //화남
  B00000000,
  B00000000,
  B00000000, 
  B11111111,
  B11111111,
  B00000000,
  B00000000,
  B00000000
};

// Pixy 카메라 데이터
int cont = 0, signature, x, y, width, height;
float cx, cy, area;
unsigned long previousMillis = 0; // 이전 시간 기록
bool toggleCustom = true;         // 교체 상태 추적

void setup() {
  Serial.begin(9600);
  BLUETOOTH_SERIAL.begin(9600); // 블루투스 시리얼 초기화
  MP3_SERIAL.begin(9600);       // DFPlayer 시리얼 초기화
  pixy.init();

  for (int i = 0; i < 6; i++) {
    pinMode(myPins[i], OUTPUT); // 모터 핀 설정
  }

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  lc.shutdown(0, false);       // LED 도트 매트릭스 활성화
  lc.setIntensity(0, 5);       // 밝기 설정
  lc.clearDisplay(0);          // 초기화

  // DFPlayer Mini 초기화
  if (!myDFPlayer.begin(MP3_SERIAL)) {
    Serial.println("DFPlayer Mini initialization failed!");
    while (true); // DFPlayer 초기화 실패 시 무한 대기
  }
  myDFPlayer.volume(30); // 볼륨 설정

  myservo.attach(28);    // 서보모터 핀 설정
}

void loop() {
  handleBluetooth();           // 블루투스 명령 처리
  long distance = handleUltrasonic();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}

//머리 쪽 초음파센서 도트LED
long handleUltrasonic() {
  long duration, distance;
  unsigned long currentMillis = millis();

  // 초음파 거리 측정
  digitalWrite(TRIG2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG2, LOW);
  duration = pulseIn(ECHO2, HIGH, 30000); // 타임아웃 30ms 설정
  if (duration == 0) {
    Serial.println("Ultrasonic sensor timeout!");
    return; // 초음파 센서 측정 실패 시 함수 종료
  }

  distance = duration * 17 / 1000;

  // 거리 및 도트 매트릭스 제어
  if (distance > 0 && distance <= 10) {
    // 서보모터 3번 동작 관리
    static int servoMoveCount = 0;          // 서보모터 동작 횟수
    static unsigned long servoLastMove = 0; // 마지막 동작 시간
    static bool servoMoving = false;        // 서보모터 상태

    if (!servoMoving) {
      myservo.write(90);         // 서보모터 회전
      servoLastMove = currentMillis; // 동작 시작 시간 기록
      servoMoving = true;
    }

    if (servoMoving && (currentMillis - servoLastMove >= 300)) {
      myservo.write(0);          // 서보모터 초기 위치로 복귀
      servoMoveCount++;          // 동작 횟수 증가
      servoLastMove = currentMillis;

      if (servoMoveCount >= 3) { // 동작 3회 완료 시 초기화
        servoMoveCount = 0;
        servoMoving = false;
      }
    }

    showLED(heart, 1);           // 하트 출력
  } 
  else {
    myservo.write(0);            // 서보모터 초기 위치 유지

    // custom1과 custom2의 출력 시간 관리
    static unsigned long previousMillis = 0; // 이전 시간 기록
    static bool isCustom1 = true;            // 현재 출력 상태

    if (isCustom1 && currentMillis - previousMillis >= 4000) {
      previousMillis = currentMillis;
      isCustom1 = false; // custom2로 전환
    } 
    else if (!isCustom1 && currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      isCustom1 = true; // custom1로 전환
    }

    // 현재 상태에 따라 도트 매트릭스 출력
    if (isCustom1) {
      showLED(custom1, 1); // custom1 출력 (5초)
    } else {
      showLED(custom2, 1); // custom2 출력 (1초)
    }
  }

  // 디버깅 정보 출력
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  return distance;
}


// 블루투스 데이터 처리
void handleBluetooth() {
  if (BLUETOOTH_SERIAL.available()) {
    String text = BLUETOOTH_SERIAL.readStringUntil(0x0A);
    Serial.print("Received Command: ");
    Serial.println(text);

    if (text == "1") moveRobot(90, 90);         // 앞으로 이동
    else if (text == "2") moveRobot(255, -130);  // 오른쪽 회전
    else if (text == "3") moveRobot(-130, 255);  // 왼쪽 회전
    else if (text == "4") moveRobot(-90, -90);  // 뒤로 이동
    else if (text == "10"){ //왼쪽 돌아
      moveRobot(-130, 255);
      delay(6500);
      moveRobot(0, 0);
    }
    else if (text == "13"){ //오른쪽 돌아
      moveRobot(255, -130);
      delay(7000);
      moveRobot(0, 0);
    }
    else if (text == "11") {    //짖어
      // DFPlayer 트랙 재생 디버깅
      Serial.println("Attempting to play track 1");
      myDFPlayer.play(1); // 1번 트랙 재생
      Serial.println("Playing track 1");

      Serial.println("Command 11: Showing custom3 LED");
      
      // custom3 패턴을 5초 동안 출력
      unsigned long startMillis = millis(); // 시작 시간 기록
      while (millis() - startMillis < 2700) { // 5000ms (5초) 동안 반복
        showLED(custom3, 1); // custom3 출력
        delay(500);          // 짧은 지연
      }

      // custom3 패턴 종료 후 LED 초기화
      showLED(custom3, 0); // LED 초기화
    } 
    else if (text == "14") {
      showLED(heart,1);
      for(int i=0; i<3; i++){
        myservo.write(90);         // 서보모터 회전
        delay(500);          // 짧은 지연
        myservo.write(0);    // 서보모터를 원위치로
        delay(500);          // 500ms 대기
      }
    }
    else if (text == "15") handlePixyTracking(text); // Pixy2 추적 모드
    else if (text == "16"){       //노래해
      myDFPlayer.play(3);
    }
    else if (text == "18"){       //잠자기
      myDFPlayer.play(4);
      do{
        text = BLUETOOTH_SERIAL.readStringUntil(0x0A);
        showLED(custom2, 1);
      }while(text != "17"); //일어나
    }
    else if (text == "s" || text == "12") moveRobot(0, 0); // 정지
  }  
}


void handlePixyTracking(String &text) {
  do {
    long distance = torsoUltrasound();
    // 블루투스 데이터를 주기적으로 읽음
    if (BLUETOOTH_SERIAL.available()) {
      text = BLUETOOTH_SERIAL.readStringUntil(0x0A);
        
      if(text == "12"){
        moveRobot(0, 0); // 정지
        break;
      } 
      else if (text == "11"){
        myDFPlayer.play(1); // 음악 재생

      // custom3 패턴을 5초 동안 출력
        unsigned long startMillis = millis(); // 시작 시간 기록
        while (millis() - startMillis < 2700) { // 5000ms (5초) 동안 반복
          showLED(custom3, 1); // custom3 출력
          delay(500);          // 짧은 지연
        }
      }
      // custom3 패턴 종료 후 LED 초기화
      showLED(custom3, 0); // LED 초기화
    }
    
    handleUltrasonic();
    // Pixy 카메라 데이터 확인 및 제어
    float turn = pixyCheck();

    // 물체가 없으면 모터 멈춤
    if (turn == 0) {
      moveRobot(130, -90);
    }
    // 물체가 중앙에 있으면 직진
    else if (turn > -deadZone && turn < deadZone) {
      moveRobot(90, 90);  // 직진
    }
    // 물체가 왼쪽에 있으면 왼쪽 회전
    else if (turn > 0) {
      moveRobot(-130, 255); // 왼쪽 회전
    }
    // 물체가 오른쪽에 있으면 오른쪽 회전
    else if (turn < 0) {
      moveRobot(255, -130); // 오른쪽 회전
    }

    // 초음파 거리와 Pixy 카메라 데이터를 사용하여 이동 제어
    if (distance <= 60) { 
      moveRobot(0, 0); // 목표에 도달하면 정지
    }

    //delay(50); // 적절한 지연 시간 추가

  } while (text != "12"); // 정지 명령
}

//몸통 초음파 
long torsoUltrasound(){
  long duration1, distance1;

  // 초음파 거리 측정
  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);

  duration1 = pulseIn(ECHO1, HIGH);
  distance1 = duration1 * 17 / 1000; //거리계산
  
  return distance1;
}

// 맵핑 함수
float mapfloat(long x, long in_min, long in_max, long out_min, long out_max) {
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

// Pixy 카메라 데이터 읽기
float pixyCheck() {
  // Pixy2에서 감지한 블록 개수 가져오기
  int blocks = pixy.ccc.getBlocks();
  
  if (blocks > 0) {
    // 첫 번째 블록 정보 가져오기
    signature = pixy.ccc.blocks[0].m_signature;  // 블록의 서명 정보
    height = pixy.ccc.blocks[0].m_height;       // 블록의 높이
    width = pixy.ccc.blocks[0].m_width;         // 블록의 너비
    x = pixy.ccc.blocks[0].m_x;                 // 블록의 X 좌표 (화면상의 위치)
    y = pixy.ccc.blocks[0].m_y;                 // 블록의 Y 좌표 (화면상의 위치)

    // 중심 좌표 계산 (블록 중심 좌표를 바로 사용)
    cx = x;  // 블록의 X 중심 좌표
    cy = y;  // 블록의 Y 중심 좌표

    // 중심 좌표 정규화 (-1 ~ 1로 변환)
    float normalizedCX = ((float)cx - 158.0) / 158.0;  // 0~316을 -1~1로 변환 (X축)
    float normalizedCY = ((float)cy - 104.0) / 104.0;  // 0~208을 -1~1로 변환 (Y축)
    
    // 정규화된 값이 -1과 1 사이에 있도록 제한
    normalizedCX = constrain(normalizedCX, -1, 1);  
    normalizedCY = constrain(normalizedCY, -1, 1);  

    // 블록의 면적 계산 (면적 = 너비 * 높이)
    area = width * height;

    // 디버깅 출력: 정규화된 중심 좌표 출력
    Serial.print("Normalized CX: ");
    Serial.print(normalizedCX);
    Serial.print(" Normalized CY: ");
    Serial.println(normalizedCY);
    
    return normalizedCX;  // 정규화된 중심 좌표 반환 (X값만 반환)
  } else {
    // 블록이 감지되지 않았을 경우
    Serial.println("No object detected");
    return 0;  // 객체 미검출 시 0 반환
  }
}

// 로봇 모터 제어
void moveRobot(int leftSpeed, int rightSpeed) {
  digitalWrite(myPins[1], leftSpeed > 0);
  digitalWrite(myPins[2], leftSpeed <= 0);
  digitalWrite(myPins[3], rightSpeed > 0);
  digitalWrite(myPins[4], rightSpeed <= 0);
  
  analogWrite(myPins[0], abs(leftSpeed));   //왼쪽 모터 스피드
  analogWrite(myPins[5], abs(rightSpeed));  //오른쪽 모터 스피드
}

// LED 도트매트릭스 제어
void showLED(byte arr[], int a) {
  if (a == 1) {
    for (int i = 0; i < 8; i++) lc.setRow(0, i, arr[i]);
  } else {
    lc.clearDisplay(0);
  }
}
