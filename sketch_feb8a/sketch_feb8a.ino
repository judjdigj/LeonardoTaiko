#include <Keyboard.h>
#include <KeyboardLayout.h>

const int threshold = 100;  // The minimum rate on triggering a input
const int outputDuration = 30;  // How long a key should be pressed when triggering a input.
const int cd_length = 50; //Buffer loop times.

const int keymapping[4] = {'f','d','j','k'};

int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];


void setup() {
  Serial.begin(9600);  // 初始化串口通信
  Keyboard.begin();
}

void loop() {
  bool output = false;
  int sensorValue[] = {analogRead(A0),analogRead(A3),analogRead(A1),analogRead(A2)};
  for (int i = 0; i <= 3; i++){
    if (sensorValue[i] > threshold){
      output = true;
    }
  }

  if (output){
    //触发后，将数值存入缓存
    int j = 0;
    while(j < buffer_size){
      int buffer_size = cd_length*4;
      for (int pin = A0; pin < A4; pin++){
        buffer[j] = analogRead(pin);
        j++;
      }
    }
    //判断是哪个脚被触发
    int temp = buffer[0];
    int count = 1;
    for(int i = 0; i < buffer_size; i++){
      if (temp < buffer[i]){
        temp = buffer[i];
        count = i+1;
      }
    }
    key = count%4;
    Keyboard.press(keymapping[key]);
    delay(outputDuration);
    Keyboard.releaseAll();
  }
}

/*
void loop() {
  bool output = false;
  int sensorValue[] = {analogRead(A0),analogRead(A3),analogRead(A1),analogRead(A2)};
  for (int i = 0; i <= 3; i++){
    if (sensorValue[i] > threshold){
      output = true;
    }
  }

  if (output) {
    for(int pin = A0; pin < A4; pin++){
      Serial.print(analogRead(pin));
      Serial.print(" ");
      }
    Serial.println("========");
  }
}
*/