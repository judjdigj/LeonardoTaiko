#include <EEPROM.h>
#include <NintendoSwitchControlLibrary.h>
#include <Keyboard.h>

//Parameter that can be adjusted by user depends of the sensor
const int windows_size = 25; //Step for triggering.
const int cd_length = 5;//Buffer size for misfire detection.
const int reset_threshold = 8;//threshold to release a input.
const int trigger_threshold = 150;//threshold to press a input
const int break_check_limit = 12;//Limit of break check when trying releasing a input.
const int keymapping[4] = {'d','j','k','f'};

//#define DEBUG
//#define MODE_SELECTOR

//Parameter that can't be adjusted by user
const int buffer_size = cd_length*4;////Buffer size for misfire detection.
int windowsA0[windows_size];
int windowsA1[windows_size];
int windowsA2[windows_size];
int windowsA3[windows_size];//Step array for triggering detection.
int first_index = 0;
int second_index = 0;//Index for the array. One point to latest, another point to oldest.
int buffer[buffer_size];//Buffer window for misfire detection.
int mode = 2;

int press_time[4] = {0,0,0,0};
bool button_status[4] = {0,0,0,0};

void setup() {
  analogReference(INTERNAL);
  #ifdef MODE_SELECTOR
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  int pc_status = digitalRead(0);
  int ns_status = digitalRead(1);
  if (ns_status == LOW && pc_status == HIGH) {
    mode = 1;     
    EEPROM.write(0, 1);   
  } else if (pc_status == LOW && ns_status == HIGH) {
    mode = 0;    
    EEPROM.write(0, 0); 
  } else if (pc_status == LOW && ns_status == LOW){
    mode = 2;
    EEPROM.write(0, 2);
  } else {
    mode = EEPROM.read(0);
  }
  #endif

  #ifdef DEBUG
  delay(1000);
  #endif

  #ifdef MODE_SELECTOR
  if (mode == 1) {  
    #ifdef DEBUG
    Serial.println("start with NS mode");
    #endif
    pushButton(Button::A, 500, 3); 
  } else if(mode == 0) {
    #ifdef DEBUG
    Serial.println("start with PC mode");
    #endif
    Keyboard.begin();
  } else if(mode == 2) {
    #ifdef DEBUG
    Serial.println("start with Sim mode");
    #endif
    Keyboard.begin();
  }
  #endif
  analogReference(INTERNAL);//Use 1.1V internal reference voltage for more obvious and stable signal.
}

void loop() {
  #ifdef DEBUG
  //analogMonitor();
  analogThresholdDynamicValueMonitor();
  #endif

  #ifndef DEBUG
  extendKey();//NS Key Support
  switch(mode){//Async Release Detection
    case 2://SIM mode
      Keyboard.releaseAll();
      break;
    case 1://NS Mode
      for(int i=0; i<4; i++){
        if(button_status[i] == 1 && millis() - press_time[i] > 30){
          switch(i){
            case 0:
              SwitchControlLibrary().releaseButton(Button::ZL);
              break;
            case 2:
              SwitchControlLibrary().releaseButton(Button::ZR);
              break;
            case 1:
              SwitchControlLibrary().releaseButton(Button::RCLICK);
              break;
            case 3:
              SwitchControlLibrary().releaseButton(Button::LCLICK);
              break;
            }
          SwitchControlLibrary().sendReport();
          button_status[i] = 0;
        }
      }
      break;
    case 0://PC Mode
      for(int i=0; i<4; i++){
        if(button_status[i] == 1 && millis() - press_time[i] >= 30){
          Keyboard.release(keymapping[i]);
          button_status[i] = 0;
        }
      }
      break;
  }

  //Read analog value and store them into step array.
  windowsA0[second_index] = analogRead(A0);
  windowsA1[second_index] = analogRead(A1);
  windowsA2[second_index] = analogRead(A2);
  windowsA3[second_index] = analogRead(A3);
  first_index = second_index;
  second_index=(second_index+1)%windows_size;

  //Triggering detection.
  if(
    windowsA0[first_index]-windowsA0[second_index]>trigger_threshold||
    windowsA1[first_index]-windowsA1[second_index]>trigger_threshold||
    windowsA2[first_index]-windowsA2[second_index]>trigger_threshold||
    windowsA3[first_index]-windowsA3[second_index]>trigger_threshold){

    //Misfire detection.
    int j = 0;
    while (j < buffer_size) {
      for (int pin = A0; pin < A4; pin++) {
        buffer[j] = analogRead(pin);
        j++;
      }
    }
    int temp = buffer[0];
    int count = 0;
    for (int i = 0; i < buffer_size; i++) {
      if (temp < buffer[i]) {
        temp = buffer[i];
        count = i;
      }
    }
    int key = count%4;
    int break_check = 0;

    switch(mode){
      case 2:
        Keyboard.press(keymapping[key]);
        break;
      case 1:
        switch(key){
          case 0:
            SwitchControlLibrary().pressButton(Button::ZL);
            break;
          case 2:
            SwitchControlLibrary().pressButton(Button::ZR);
            break;
          case 1:
            SwitchControlLibrary().pressButton(Button::RCLICK);
            break;
          case 3:
            SwitchControlLibrary().pressButton(Button::LCLICK);
            break;
        }
        SwitchControlLibrary().sendReport();
        press_time[key] = millis();
        button_status[key] = 1;
        break;
      case 0:
        Keyboard.press(keymapping[key]);
        press_time[key] = millis();
        button_status[key] = 1;
        break;
    }

    switch(key){
      case 0:
        while(break_check<break_check_limit){
          windowsA0[second_index] = analogRead(A0);
          windowsA1[second_index] = analogRead(A1);
          windowsA2[second_index] = analogRead(A2);
          windowsA3[second_index] = analogRead(A3);
          first_index = second_index;
          second_index=(second_index+1)%windows_size;
          if(windowsA0[first_index]-windowsA0[second_index]<reset_threshold){
            break_check++;
          }
          else{
            break_check=0;
          }
        }
        break;
      case 1:
        while(break_check<break_check_limit){
          windowsA0[second_index] = analogRead(A0);
          windowsA1[second_index] = analogRead(A1);
          windowsA2[second_index] = analogRead(A2);
          windowsA3[second_index] = analogRead(A3);
          first_index = second_index;
          second_index=(second_index+1)%windows_size;
          if(windowsA1[first_index]-windowsA1[second_index]<reset_threshold){
            break_check++;
          }
          else{
            break_check = 0;
          }
        }
        break;
      case 2:
        while(break_check<break_check_limit){
          windowsA0[second_index] = analogRead(A0);
          windowsA1[second_index] = analogRead(A1);
          windowsA2[second_index] = analogRead(A2);
          windowsA3[second_index] = analogRead(A3);
          first_index = second_index;
          second_index=(second_index+1)%windows_size;
          if(windowsA2[first_index]-windowsA2[second_index]<reset_threshold){
            break_check++;
          }
          else{
            break_check = 0;
          }
        }
        break;
      case 3:
        while(break_check<break_check_limit){
          windowsA0[second_index] = analogRead(A0);
          windowsA1[second_index] = analogRead(A1);
          windowsA2[second_index] = analogRead(A2);
          windowsA3[second_index] = analogRead(A3);
          first_index = second_index;
          second_index=(second_index+1)%windows_size;
          if(windowsA3[first_index]-windowsA3[second_index]<reset_threshold){
            break_check++;
          }
          else{
            break_check = 0;
          }
        }
        break;
    }
  }
  #endif
}

void extendKey(){
  if (mode == 1){
    if(digitalRead(0) == HIGH && digitalRead(1) == LOW){
      pushHat(Hat::LEFT);
    }
    if(digitalRead(1) == HIGH && digitalRead(0) == LOW){
      pushHat(Hat::RIGHT);
    }
    if(digitalRead(1) == LOW && digitalRead(0) == LOW){
      pushButton(Button::B);
    }
  }
}

void analogMonitor(){
  windowsA0[second_index] = analogRead(A0);
  windowsA1[second_index] = analogRead(A1);
  windowsA2[second_index] = analogRead(A2);
  windowsA3[second_index] = analogRead(A3);
  first_index = second_index;
  second_index=(second_index+1)%windows_size;
  Serial.print(windowsA0[second_index]);
  Serial.print("||");
  Serial.print(windowsA1[second_index]);
  Serial.print("||");
  Serial.print(windowsA2[second_index]);
  Serial.print("||");
  Serial.println(windowsA3[second_index]);
}

void analogThresholdDynamicValueMonitor(){
  int timer;
  windowsA0[second_index] = analogRead(A0);
  windowsA1[second_index] = analogRead(A1);
  windowsA2[second_index] = analogRead(A2);
  windowsA3[second_index] = analogRead(A3);
  first_index = second_index;
  second_index=(second_index+1)%windows_size;
  int A0_Value = windowsA0[first_index]-windowsA0[second_index];
  int A1_Value = windowsA1[first_index]-windowsA1[second_index];
  int A2_Value = windowsA2[first_index]-windowsA2[second_index];
  int A3_Value = windowsA3[first_index]-windowsA3[second_index];

  if(A0_Value >trigger_threshold||A1_Value >trigger_threshold||A2_Value >trigger_threshold||A3_Value >trigger_threshold){
    timer = millis();
  }

  if(millis() - timer < 500){
    Serial.print(A0_Value);
    Serial.print("||");
    Serial.print(A1_Value);
    Serial.print("||");
    Serial.print(A2_Value);
    Serial.print("||");
    Serial.println(A3_Value);
  }
}

void analogThresholdMonitor(){
  int value[300];
  if(analogRead(A1) > 100){
    for(int i=0;i<300;i++){
      windowsA0[second_index] = analogRead(A0);
      windowsA1[second_index] = analogRead(A1);
      windowsA2[second_index] = analogRead(A2);
      windowsA3[second_index] = analogRead(A3);
      first_index = second_index;
      second_index=(second_index+1)%windows_size;
      value[i]=windowsA1[second_index];
    }
    for(int i=0;i<300;i++){
      Serial.println(value[i]);
    }
  }
}

