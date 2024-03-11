#include <EEPROM.h>
#include "pressKey.h"

const float min_threshold = 75;  // The minimum rate on triggering a input
const int cd_length = 20; //Buffer loop times.
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.7;  //Dynamite threshold range.

const int outputDuration_pc = 7; // For PC. How long a key should be pressed when triggering a input.
const int outputDuration_ns = 35; // For NS. How long a key should be pressed when triggering a input.

// {LK, LD, RD, RK}
const KeyUnion NS_LEFT_KATSU[4]  = {{Button::ZL, NS_BTN, NS_BTN_DUR, 0, false}, {Button::L, NS_BTN, NS_BTN_DUR, 0, false}, {Hat::UP, NS_HAT, NS_HAT_DUR, 0, false}, {Hat::LEFT, NS_HAT, NS_HAT_DUR, 0, false}};
const KeyUnion NS_LEFT_DON[3]    = {{Button::LCLICK, NS_BTN, NS_BTN_DUR, 0, false}, {Hat::RIGHT, NS_HAT, NS_HAT_DUR, 0, false}, {Hat::DOWN, NS_HAT, NS_HAT_DUR, 0, false}};
const KeyUnion NS_RIGHT_DON[3]   = {{Button::RCLICK, NS_BTN, NS_BTN_DUR, 0, false}, {Button::Y, NS_BTN, NS_BTN_DUR, 0, false}, {Button::B, NS_BTN, NS_BTN_DUR, 0, false}};
const KeyUnion NS_RIGHT_KATSU[4] = {{Button::ZR, NS_BTN, NS_BTN_DUR, 0, false}, {Button::R, NS_BTN, NS_BTN_DUR, 0, false}, {Button::X, NS_BTN, NS_BTN_DUR, 0, false}, {Button::A, NS_BTN, NS_BTN_DUR, 0, false}};
const KeyUnion PC_LEFT_KATSU[1]  = {{'d', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_LEFT_DON[1]    = {{'f', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_RIGHT_DON[1]   = {{'j', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_RIGHT_KATSU[1] = {{'k', PC_BTN, PC_BTN_DUR, 0, false}};
const int NS_SIZE[4] = {sizeof(NS_LEFT_KATSU) / sizeof(KeyUnion), sizeof(NS_LEFT_DON) / sizeof(KeyUnion), sizeof(NS_RIGHT_DON) / sizeof(KeyUnion), sizeof(NS_RIGHT_KATSU) / sizeof(KeyUnion)};
const int PC_SIZE[4] = {sizeof(PC_LEFT_KATSU) / sizeof(KeyUnion), sizeof(PC_LEFT_DON) / sizeof(KeyUnion), sizeof(PC_RIGHT_DON) / sizeof(KeyUnion), sizeof(PC_RIGHT_KATSU) / sizeof(KeyUnion)};

//{A3, A0, A1, A2}
// const uint16_t keymapping_ns[4] = {Button::A, Hat::UP, Hat::DOWN, Button::B};
// const int keymapping[4] = {'k','d','f','j'};
const uint16_t keymapping_ns_extend[] = {Button::PLUS, Hat::RIGHT};

int mode; //0 for keyboard, 1 for switch
int outputDuration;
int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];
int threshold = min_threshold;

void setup() {
  Serial.begin(9600);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  int pc_status = digitalRead(0);
  int ns_status = digitalRead(1);

  // 初始化读取按键电平
  if (ns_status == LOW && pc_status == HIGH){
    mode = 1;     // 按下NS按键，初始化为NS模式
    outputDuration = outputDuration_ns; 
    EEPROM.write(0, 1);   // 写入EEPROM
    }
  if (pc_status == LOW && ns_status == HIGH){
    mode = 0;     // 按下PC按键，初始化为PC模式
    outputDuration = outputDuration_pc; 
    EEPROM.write(0, 0);   // 写入EEPROM
    }
  else{
    // 没有按任何按键，从EEPROM中读取之前的控制状态
    mode = EEPROM.read(0);
    if(mode == 0){    // 默认值与PC状态
      outputDuration = outputDuration_pc;
      }
    if(mode == 1){    // NS状态
      outputDuration = outputDuration_ns;
      }
    }

  if(mode == 1){  
    pushButton(Button::A, 500, 3); // 初始化时按键自动连接到SWITCH
    }
  if(mode == 0){
    Keyboard.begin();              // 初始化启动按键输入
  }
}

void loop() {

//  analogMonitor();
  extendKey();
  release();
    
  bool output = false;
  int sensorValue[] = {analogRead(A0),analogRead(A3),analogRead(A1),analogRead(A2)};
  for (int i = 0; i <= 3; i++){
    if (sensorValue[i] > threshold){
      output = true;
    }
  }

  if (output){
    //Storage pin value into buffer.
    int j = 0;
    while(j < buffer_size){
      for (int pin = A0; pin < A4; pin++){
        buffer[j] = analogRead(pin);
        j++;
      }
    }
    //finding the largest value
    int temp = buffer[0];
    int count = 1;
    for(int i = 0; i < buffer_size; i++){
      if (temp < buffer[i]){
        temp = buffer[i];
        count = i+1;
      }
    }
    threshold = temp*k_increase;
    key = (count + 1) % 4;          // 处理后，变成  0:LK  1:LD  2:RD  3:RK
    bool pressed = false;
    if (mode == 0) {      // PC模式输出
      switch (key) {
        case 0: pressed = press(PC_SIZE[key], PC_LEFT_KATSU); break;
        case 1: pressed = press(PC_SIZE[key], PC_LEFT_DON); break;
        case 2: pressed = press(PC_SIZE[key], PC_RIGHT_DON); break;
        case 3: pressed = press(PC_SIZE[key], PC_RIGHT_KATSU); break;
      }
    } else {              // NS模式输出
      switch (key) {
        case 0: pressed = press(NS_SIZE[key], NS_LEFT_KATSU); break;
        case 1: pressed = press(NS_SIZE[key], NS_LEFT_DON); break;
        case 2: pressed = press(NS_SIZE[key], NS_RIGHT_DON); break;
        case 3: pressed = press(NS_SIZE[key], NS_RIGHT_KATSU); break;
      }
    }
    if (pressed) {
      Serial.println(temp);
      Serial.println(threshold);
      Serial.println(key);
    }
  }
  if(threshold < min_threshold){
    threshold = min_threshold;
  }
  else if(threshold > min_threshold){
    threshold = threshold*k_decay;
//    Serial.println("DECAY");
//    Serial.println(threshold); //Check decay, in order to set proper k_increase and k_decay.
  }
}


void analogMonitor(){
  bool output = false;
  int sensorValue[] = {analogRead(A0),analogRead(A3),analogRead(A1),analogRead(A2)};
  for (int i = 0; i <= 3; i++){
    if (sensorValue[i] > threshold){
      output = true;
    }
  }
  
  if (output){
    int j = 0;
    while(j<100){
      for (int pin = A0; pin <= A3; pin++){
        Serial.print("||");
        Serial.print(analogRead(pin));
        if(pin == A3){
          Serial.println("||");
          Serial.println(j);
          Serial.println("===========");
        }
      }
    j++;
    }
  }
}

void extendKey(){
  if (mode == 1){
    if(digitalRead(0) == LOW || digitalRead(1) == LOW){
      for(int pin = 0; pin <= 1; pin++){
        if (digitalRead(pin) == LOW){
          if( pin == 1 ){
            SwitchControlLibrary().pressHatButton(keymapping_ns_extend[pin]);
            SwitchControlLibrary().sendReport();
            delay(300);
            SwitchControlLibrary().releaseHatButton();
            SwitchControlLibrary().sendReport();
          }
          else{
            SwitchControlLibrary().pressButton(keymapping_ns_extend[pin]);
            SwitchControlLibrary().sendReport();
            delay(300);
            SwitchControlLibrary().releaseButton(keymapping_ns_extend[pin]);
            SwitchControlLibrary().sendReport();
            }
          }
        }
      }
  }
}








