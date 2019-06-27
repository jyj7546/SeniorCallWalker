#include <SoftwareSerial.h>
#include <Wire.h>
#include <MechaQMC5883.h>
SoftwareSerial xbee(2, 3); 

MechaQMC5883 qmc;
boolean turn = true; //턴 방향 결정 true: 시계 , false: 반시계 

int R_echo_pin = 13;
int R_trig_pin = 12;
int L_echo_pin = 5;
int L_trig_pin = 4;
 
void setup() {
  Serial.begin(9600);
  xbee.begin(9600); 
  Wire.begin();
  qmc.init();

  pinMode(R_echo_pin,INPUT);
  pinMode(R_trig_pin,OUTPUT);
  pinMode(L_echo_pin,INPUT);
  pinMode(L_trig_pin,OUTPUT);
  pinMode(A1,INPUT); //Front_echo_pin
  pinMode(A0,OUTPUT); //Front_trig_pin

  pinMode(9, OUTPUT);       // Motor A 방향설정1
  pinMode(7, OUTPUT);       // Motor B 방향설정1
  
}

void loop() {
  int x, y, z;
  int my_azimuth; // 자신의 방향각
  int other_azimuth; // 상대편 방향각
  int angle; // 각도 차이

  float duration_R, duration_L, duration_F;

   /*초음파 센서 장애물 감지*/
   
   digitalWrite(R_trig_pin,HIGH);
   delayMicroseconds(100);
   digitalWrite(R_trig_pin,LOW);
   duration_R = pulseIn(R_echo_pin,HIGH);
   duration_R = (duration_R*340)/20000; // 오른쪽 장애물 거리 측정

   digitalWrite(L_trig_pin,HIGH);
   delayMicroseconds(100);
   digitalWrite(L_trig_pin,LOW);
   duration_L = pulseIn(L_echo_pin,HIGH);
   duration_L = (duration_L*340)/20000; // 왼쪽 장애물 거리 측정

   digitalWrite(A0,HIGH);
   delayMicroseconds(100);
   digitalWrite(A0,LOW);
   duration_F = pulseIn(A1,HIGH);
   duration_F = (duration_F*340)/20000; // 중앙 장애물 거리 측정
   
  qmc.read(&x, &y, &z, &my_azimuth);
  
  if(xbee.available()){
    int mapped_azimuth = xbee.read();
    
    if (mapped_azimuth == 255) {
      stop();
      }
      
    else {
      
      other_azimuth = map(mapped_azimuth, 0, 255, 0, 359);
      //수신된 방향각 맵핑 복원
    
      angle = my_azimuth - other_azimuth;
      angle = abs(angle);
      //양측 장치의 방향각 차이(abs: 절대값)

      /*
      Serial.print("MY:");
      Serial.println(my_azimuth);
      Serial.print("OTHER:");
      Serial.println(other_azimuth);
      */

      //회전방향 결정 - 4가지 경우
      if (angle < 180) {
        if (other_azimuth > my_azimuth)
          turn = true;
        else
          turn = false;
        }

      else {
        if (other_azimuth > my_azimuth)
          turn = false;
        else
          turn = true;
        }
      //Serial.println(turn);

      //각도가 20 이상 틀어졌을 경우
      if (angle > 25) {
        //시계 방향 턴
        if (turn == true) {
          turn_r();
          }
        //반시계방향 턴  
        else {
          turn_l();
          }
        }
      
        /*========각도 차이가 20 이하일 경우 -> 회피주행========*/
        else {
          // 장애물 양쪽 거리 25이하
          
          if(duration_R < 23 && duration_L < 23) {  
          backward();
          delay(500);
          }
        
          // 장애물 오른쪽 거리 20이하
          else if(duration_R < 23) { 
          //정지-우회전
          right();
          }
        
          // 장애물 왼쪽 거리 20이하
          else if(duration_L < 23) {
          //정지-좌회전
          left();
          }
          
          // 장애물 가운데
          else if(duration_F < 23) {
          //정지-후진
          backward();
          }

          else if(duration_F < 23 && duration_R < 23) {
          right();
          delay(500);
          }

          else if(duration_F < 23 && duration_L < 23) {
          left();
          delay(500);
          }

          // 장애물 없음  
          else {   
          //전진
          forward();
          }
        }
      }
    }
}


/*============================모터 구동 함수=============================*/
/* Motor A = 오른쪽 바퀴, Motor B = 왼쪽 바퀴 */
void forward() // 전진
{
    /*모터A설정 정회전*/
    digitalWrite(9, LOW);     // Motor A 방향설정1
    analogWrite(11, 200);       // Motor A 속도조절 (0~255)

    /*모터B설정 정회전*/
    digitalWrite(7, LOW);      // Motor B 방향설정1
    analogWrite(6, 200);        // Motor B 속도조절 (0~255)
}

void backward() // 후진
{
    /*모터A설정 역회전*/
    digitalWrite(9, HIGH);     // Motor A 방향설정1
    analogWrite(11, 200);       // Motor A 속도조절 (0~255)

    /*모터B설정 역회전*/
    digitalWrite(7, HIGH);      // Motor B 방향설정1
    analogWrite(6, 200);        // Motor B 속도조절 (0~255)

    //delay(1000);
}

void stop() // 정지
{
    /*모터A설정 정지*/
    digitalWrite(9, LOW);     // Motor A 방향설정1
    analogWrite(11, 0);       // Motor A 속도조절 (0~255)
    
    /*모터B설정 정지*/
    digitalWrite(7, LOW);      // Motor B 방향설정1
    analogWrite(6, 0);        // Motor B 속도조절 (0~255)
}

void right() // 우회전
{
    /*모터A설정 정지*/
    digitalWrite(9, LOW);     // Motor A 방향설정1
    analogWrite(11, 0);       // Motor A 속도조절 (0~255)
    
    /*모터B설정 정회전*/
    digitalWrite(7, HIGH);      // Motor B 방향설정1
    analogWrite(6, 200);        // Motor B 속도조절 (0~255)

    //delay(1000);
}

void left() // 좌회전
{
    /*모터A설정 정회전*/
    digitalWrite(9, HIGH);     // Motor A 방향설정1
    analogWrite(11, 200);       // Motor A 속도조절 (0~255)

    /*모터B설정 정지*/
    digitalWrite(7, LOW);      // Motor B 방향설정1
    analogWrite(6, 0);       // Motor A 속도조절 (0~255)
    //delay(1000);
}

void turn_r() {
  /*모터A설정 역회전*/
  digitalWrite(9, HIGH);     // Motor A 방향설정1
  analogWrite(11,200);       // Motor A 속도조절 (0~255)

  /*모터B설정 정회전*/
  digitalWrite(7, LOW);      // Motor B 방향설정1
  analogWrite(6, 200);        // Motor B 속도조절 (0~255)
  }

void turn_l() {
  /*모터A설정 정회전*/
  digitalWrite(9, LOW);     // Motor A 방향설정1
  analogWrite(11,200);       // Motor A 속도조절 (0~255)

  /*모터B설정 역회전*/
  digitalWrite(7, HIGH);      // Motor B 방향설정1
  analogWrite(6, 200);        // Motor B 속도조절 (0~255)
  }

