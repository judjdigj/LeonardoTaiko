#include <EEPROM.h>
// #define DEBUG


const float min_threshold = 50;  // The minimum rate on triggering a input
const int cd_length = 20; //Buffer loop times.
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.7;  //Dynamite threshold range.
const int outputDuration_pc = 7; // For PC. How long a key should be pressed when triggering a input.
const int outputDuration_ns = 22; // For NS. How long a key should be pressed when triggering a input.

//{A3, A0, A1, A2}
const uint16_t keymapping_ns[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};
const int keymapping[4] = {'f','d','j','k'};
const uint16_t keymapping_ns_extend[] = {Button::PLUS, Hat::RIGHT};


// 模式与计算按键与缓存
int mode; //0 for keyboard, 1 for switch
int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];
int threshold = min_threshold;


// 帧对齐
// uint8_t dloop[3] = {33, 33, 34};                                  // 30帧对齐
// uint8_t dloop[3] = {16, 17, 17};                                  // 60帧对齐
// uint8_t dloop[3] = {8, 8, 9};                                     // 120帧对齐
// uint8_t dloop[6] = {4, 4, 4, 4, 4, 5};                            // 240帧对齐
// uint8_t dloop[12] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3};         // 480帧对齐
// uint8_t dsize = sizeof(dloop) / sizeof(uint8_t);
// uint8_t loopc = 0;


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
  unsigned long begin = millis();
//  analogMonitor();
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
//    Serial.println(temp);
//    Serial.println(threshold);
//    Serial.println(key);
    if(mode == 0){
      Keyboard.press(keymapping[key]);
      delay(outputDuration);
      Keyboard.releaseAll();
    }
    else if(mode == 1){
      SwitchControlLibrary().pressButton(keymapping_ns[key]);
      SwitchControlLibrary().sendReport();
      delay(outputDuration);
      SwitchControlLibrary().releaseButton(keymapping_ns[key]);
      SwitchControlLibrary().sendReport();
    }
  }
  
  if(threshold < min_threshold){
    threshold = min_threshold;
  } else if(threshold > min_threshold) {
    threshold = threshold*k_decay;
    // Serial.println("DECAY");
    // Serial.println(threshold); //Check decay, in order to set proper k_increase and k_decay.
  }
  // unsigned long d = begin + dloop[loopc % dsize] - millis();
  // while (d < 0) d += dloop[(++loopc) % dsize];
  // loopc = (++loopc) % dsize;
  // if (d > 0) delay(d);
}

/*
void analogMonitor(){
  bool output = false;
  int sensorValue[] = {analogRead(A0),analogRead(A3),analogRead(A1),analogRead(A2)};
  for (int i = 0; i <= 3; i++){
    if (sensorValue[i] > threshold){
      output = true;
    }
  }
  
  if (output) {
    for (int j = 0; j < 100; j++) {
      for (int pin = A0; pin <= A3; pin++) {
        Serial.print("||");
        Serial.print(analogRead(pin));
        if (pin == A3) {
          Serial.println("||");
          Serial.println(j);
          Serial.println("===========");
        }
      }
    }
  }
}
*/
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