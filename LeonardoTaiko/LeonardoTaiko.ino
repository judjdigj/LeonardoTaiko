#include <NintendoSwitchControlLibrary.h>
#include <Keyboard.h>
#include <EEPROM.h>

//fjkddfjkdfjkdfjdfjkkdfjdfjkkdfjk
const float min_threshold = 40;  // The minimum rate on triggering a input
const int cd_length = 20; //Buffer loop times.
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.8;  //Dynamite threshold range.
const int outputDuration_pc = 20; // For PC. How long a key should be pressed when triggering a input.
const int outputDuration_ns = 20; // For NS. How long a key should be pressed when triggering a input.

//{A3, A0, A1, A2}

const int keymapping[4] = {'f','d','j','k'};


// 模式与计算按键与缓存
int mode; //0 for keyboard, 1 for ns2pc
int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];
int threshold = min_threshold;

bool switchMode = 0;

int buttonPressed = -1;

int buttonStatusLK = -1;
int buttonStatusLD = -1;
int buttonStatusRD = -1;
int buttonStatusRK = -1;

unsigned long currentMillis = 0;

unsigned long previousMillisLK_1 = 0;
unsigned long previousMillisLD_1 = 0;
unsigned long previousMillisRD_1 = 0;
unsigned long previousMillisRK_1 = 0;

void setup() {
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
  unsigned long currentMillis = millis();
//Keyboard_NS2===============================================  
  //LK===================
  if (mode == 0){
    if(buttonStatusLK == 1 && currentMillis - previousMillisLK_1 >= outputDuration_pc){
      Keyboard.release('d');
      buttonStatusLK = -1;
    }
  //LD====================================================================
    if(buttonStatusLD == 1 && currentMillis - previousMillisLD_1 >= outputDuration_pc){
      Keyboard.release('f');
      buttonStatusLD = -1;
    }
  //RD=============================================================
    if(buttonStatusRD == 1 && currentMillis - previousMillisRD_1 >= outputDuration_pc){
      Keyboard.release('j');
      buttonStatusRD = -1;
    }
//RK======================================================
    if(buttonStatusRK == 1 && currentMillis - previousMillisRK_1 >= outputDuration_pc){
      Keyboard.release('k');
      buttonStatusRK = -1;
    }
  }

  else if (mode == 1){
    if(buttonStatusLK == 1 && currentMillis - previousMillisLK_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::ZL);
      SwitchControlLibrary().sendReport();
      buttonStatusLK = -1;
    }
  //LD====================================================================
    if(buttonStatusLD == 1 && currentMillis - previousMillisLD_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::LCLICK);
      SwitchControlLibrary().sendReport();
      buttonStatusLD = -1;
    }
  //RD=============================================================
    if(buttonStatusRD == 1 && currentMillis - previousMillisRD_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::RCLICK);
      SwitchControlLibrary().sendReport();
      buttonStatusRD = -1;
    }
//RK======================================================
    if(buttonStatusRK == 1 && currentMillis - previousMillisRK_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::ZR);
      SwitchControlLibrary().sendReport();
      buttonStatusRK = -1;
    }
  }

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
    if(temp >= min_threshold && mode == 1){
      switch(key){
        case 1:
          if(buttonStatusLK == -1){
            buttonStatusLK = 1;
            SwitchControlLibrary().pressButton(Button::ZL);
            previousMillisLK_1 = currentMillis;
          }
          break;
        case 0:
          if(buttonStatusLD == -1){
            buttonStatusLD = 1;
            SwitchControlLibrary().pressButton(Button::LCLICK);
            previousMillisLD_1 = currentMillis;
          }
          break;
        case 2:
          if(buttonStatusRD == -1){
            buttonStatusRD = 1;
            SwitchControlLibrary().pressButton(Button::RCLICK);
            previousMillisRD_1 = currentMillis;
          }
          break;
        case 3:
          if(buttonStatusRK == -1){
            buttonStatusRK = 1;
            SwitchControlLibrary().pressButton(Button::ZR);
            previousMillisRK_1 = currentMillis;
          }
          break;
      }
      SwitchControlLibrary().sendReport();
      delay(10);
    }

    else if(temp >= min_threshold && mode == 0){
      switch(key){
        case 1:
          if(buttonStatusLK == -1){
            buttonStatusLK = 1;
            Keyboard.press('d');
            previousMillisLK_1 = currentMillis;
          }
          break;
        case 0:
          if(buttonStatusLD == -1){
            buttonStatusLD = 1;
            Keyboard.press('f');
            previousMillisLD_1 = currentMillis;
          }
          break;
        case 2:
          if(buttonStatusRD == -1){
            buttonStatusRD = 1;
            Keyboard.press('j');
            previousMillisRD_1 = currentMillis;
          }
          break;
        case 3:
          if(buttonStatusRK == -1){
            buttonStatusRK = 1;
            Keyboard.press('k');
            previousMillisRK_1 = currentMillis;
          }
          break;
      }
      delay(10);
    }
  }
  
  if(threshold < min_threshold){
    threshold = min_threshold;
  }
  else if(threshold > min_threshold) {
    threshold = threshold*k_decay;
  }
}
