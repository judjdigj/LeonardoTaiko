#include <Keyboard.h>
#include <EEPROM.h>
#include <NintendoSwitchControlLibrary.h>



const float min_threshold = 50;  // The minimum rate on triggering a input
const int cd_length = 20; //Buffer loop times.
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.8;  //Dynamite threshold range.
const int outputDuration = 7;

//{A3, A0, A1, A2}
const int keymapping[4] = {'f','d','j','k'};
const uint16_t keymapping_ns_extend[] = {Button::PLUS, Hat::RIGHT};

const uint16_t keymapping_ns_lka[3] = {Button::ZL, Button::L, Hat::UP};
const uint16_t keymapping_ns_ldon[3] = {Button::LCLICK, Hat::DOWN, Hat::RIGHT};
const uint16_t keymapping_ns_rdon[3] = {Button::RCLICK, Button::Y, Button::B};
const uint16_t keymapping_ns_rka[3] = {Button::ZR, Button::R, Button::X};

int keyStatus_lka = 0;
int keyStatus_ldon = 0;
int keyStatus_rdon = 0;
int keyStatus_rka = 0;

int previousMillis = 0;
int previousMillis2 = 0;
int previousMillis3 = 0;

// 模式与计算按键与缓存
int mode; //0 for keyboard, 1 for switch
int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];
int threshold = min_threshold;

unsigned long currentMillis = 0;
int switchMode = 0;

void setup() {
//  Serial.begin(9600);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  int pc_status = digitalRead(0);
  int ns_status = digitalRead(1);

  // 初始化读取按键电平
  if (ns_status == LOW && pc_status == HIGH) {
    mode = 1;     // 按下NS按键，初始化为NS模式
    EEPROM.write(0, 1);   // 写入EEPROM
  } else if (pc_status == LOW && ns_status == HIGH) {
    mode = 0;     // 按下PC按键，初始化为PC模式
    EEPROM.write(0, 0);   // 写入EEPROM
  } else {
    // 没有按任何按键，从EEPROM中读取之前的控制状态
    mode = EEPROM.read(0);
  }

  // 初始化开始连接
  if (mode == 1) {  
    #ifdef DEBUG
    Serial.println("start with NS mode");
    #endif
    pushButton(Button::A, 500, 3); // 初始化时按键自动连接到SWITCH
  } else if(mode == 0) {
    #ifdef DEBUG
    Serial.println("start with PC mode");
    #endif
    Keyboard.begin();              // 初始化启动按键输入
  }
}


void loop() {
  currentMillis = millis();
  extendKey();
  bool output = false;
  int sensorValue[] = {analogRead(A0),analogRead(A3),analogRead(A1),analogRead(A2)};
  for (int i = 0; i <= 3; i++) {
    if (sensorValue[i] > threshold) {
      output = true;
    }
  }

  if (output){
    //Storage pin value into buffer.
    int j = 0;
    while (j < buffer_size) {
      for (int pin = A0; pin < A4; pin++) {
        buffer[j] = analogRead(pin);
        j++;
      }
    }
    //finding the largest value
    int temp = buffer[0];
    int count = 1;
    for (int i = 0; i < buffer_size; i++) {
      if (temp < buffer[i]) {
        temp = buffer[i];
        count = i+1;
      }
    }
    threshold = temp*k_increase;
    key = count%4;

    if(mode == 0){
      Keyboard.press(keymapping[key]);
      delay(outputDuration);
      Keyboard.releaseAll();
      
    }
    else if(mode == 1){
      switchMode = 1;
    }
  }

  if(switchMode = 1){
    switchPress();
  }
  else{
    previousMillis = 0;
    previousMillis2 = 0;
    previousMillis3 = 0;
  }
  
  if(threshold < min_threshold){
    threshold = min_threshold;
  } else if(threshold > min_threshold) {
    threshold = threshold*k_decay;
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
          } else {
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




void switchPress(){
  if(keyStatus_ldon == 0){
    if(previousMillis != 0 && currentMillis - previousMillis > 30){
      SwitchControlLibrary().releaseButton(keymapping_ns_ldon[keyStatus_ldon]);
      keyStatus_ldon = 0;
      switchMode = 0;
    }
    else if(previousMillis == 0){
      SwitchControlLibrary().pressButton(keymapping_ns_ldon[keyStatus_ldon]);
      previousMillis = currentMillis;
      keyStatus_ldon = 1;
    }
  }

  else if(keyStatus_ldon = 1){
    if(previousMillis2 != 0 && currentMillis - previousMillis2 > 30){
      SwitchControlLibrary().releaseHatButton();
      keyStatus_ldon = 0;
    }
    else{
      SwitchControlLibrary().pressHatButton(keymapping_ns_ldon[keyStatus_ldon]);
      if(previousMillis2 == 0){
        previousMillis2 = currentMillis;
      }
      keyStatus_ldon = 2;
    }
  }

  else if(keyStatus_ldon = 2){
    if(previousMillis3 != 0 && currentMillis - previousMillis3 > 30){
      SwitchControlLibrary().releaseHatButton();
      keyStatus_ldon = 0;
    }
    else{
      SwitchControlLibrary().pressHatButton(keymapping_ns_ldon[keyStatus_ldon]);
      if(previousMillis3 == 0){
        previousMillis3 = currentMillis;
      }
      keyStatus_ldon = 0;
    }
  }
  SwitchControlLibrary().sendReport();

}