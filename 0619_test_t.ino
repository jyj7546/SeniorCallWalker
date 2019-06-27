#include <SoftwareSerial.h>
#include <Wire.h>
#include <MechaQMC5883.h>

MechaQMC5883 qmc;
boolean on = false; //호출확인

SoftwareSerial xbee(2, 3); 
 
void setup() {
  xbee.begin(9600); //XBee 통신 초기화
  Wire.begin();
  Serial.begin(9600);
  qmc.init(); //지자기센서 초기화
  pinMode(8,INPUT);
}

void loop() {
  int button = digitalRead(8);
  int x, y, z; // x,y,z 축 지자기 값
  int azimuth; // 센서 방향각

  if (button==LOW) {
    on = !on;
    }
    
  qmc.read(&x, &y, &z, &azimuth); //센서 값 read
  
  int azimuth_mapped = map(azimuth, 0, 359, 0, 255);
  //write함수는 255이상의 값을 보내지 못하므로 맵핑
  
  if (on == true) {
    xbee.write(azimuth_mapped); //호출 시 방향각 데이터 전송
    Serial.println(azimuth_mapped);
    }
  else {
    xbee.write(255); //정지 시 정지신호 255 전송
    Serial.println("stop");
    }
    
  delay(200);
}
