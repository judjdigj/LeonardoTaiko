#include <Keyboard.h>
#include <KeyboardLayout.h>

void setup() {
  Serial.begin(9600); // 初始化串口通信，波特率设置为9600
  Keyboard.begin();
}

const int threshold_rim = 100; // 设置鼓边初始阈值
const int threshold = 100; //设置初始阈值
const int cd_length = 120; //毫秒
const float k_threshold_rim = 2; //鼓边抬升幅度
const float k_threshold = 2; //抬升幅度
const float k_decay_rim = 0.99; //鼓边衰变幅度
const float k_decay = 0.99; //鼓面衰变速度
  
int min_threshold_rim = threshold_rim;
int min_threshold = threshold;
int timer = cd_length;

void loop() {
  if ( timer <= 0 ){
    Keyboard.releaseAll();
  }
  else{
    timer--;
  }
  int sensorValue[] = {analogRead(A0),analogRead(A1),analogRead(A2),analogRead(A3)};

  // 判断A0引脚的模拟值与阈值的关系
//  Serial.println(timer);
  if (timer==0 && abs(sensorValue[0] - 675) > min_threshold_rim) {
    min_threshold_rim = abs(sensorValue[0]-675)*k_threshold_rim;
    min_threshold = abs(sensorValue[0]-675)*k_threshold;
    Serial.print("A0值为");
    Serial.println(abs(sensorValue[0] - 675));
    Serial.print("A3值为");
    Serial.println(abs(sensorValue[3] - 675));
    Serial.print("A1值为");
    Serial.println(abs(sensorValue[1] - 675));
    Serial.print("A2值为");
    Serial.println(abs(sensorValue[2] - 675));
    Serial.println("====================");
    Keyboard.press('d');
    timer = cd_length;
  }
  else if (timer==0 && abs(sensorValue[2] - 675) > min_threshold_rim) {
    min_threshold_rim = abs(sensorValue[2]-675)*k_threshold_rim;
    min_threshold = abs(sensorValue[2]-675)*k_threshold;

    Keyboard.press('k');
    timer = cd_length;

  }
  else if (timer==0 && abs(sensorValue[1] - 675) > min_threshold) {
    min_threshold = abs(sensorValue[1] - 675)*k_threshold;
    Keyboard.press('j');
    timer = cd_length;

  }
  else if (timer==0 && abs(sensorValue[3] - 675) > min_threshold) {
    min_threshold = abs(sensorValue[3] - 675)*k_threshold;
    Serial.print("A0值为");
    Serial.println(abs(sensorValue[0] - 675));
    Serial.print("A3值为");
    Serial.println(abs(sensorValue[3] - 675));
    Serial.print("A1值为");
    Serial.println(abs(sensorValue[1] - 675));
    Serial.print("A2值为");
    Serial.println(abs(sensorValue[2] - 675));
    Serial.println("====================");
    Keyboard.press('f');
    timer = cd_length;

  } 
  else{
    if (min_threshold < threshold){
      min_threshold = threshold;
    }
    else{
      min_threshold = min_threshold*k_decay;
    }
    if (min_threshold_rim < threshold_rim){
      min_threshold_rim = threshold_rim;
    }
    else{
      min_threshold_rim = min_threshold_rim*k_decay_rim;
    }
  }
}