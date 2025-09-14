#include <NintendoSwitchControlLibrary.h>
#include <Keyboard.h>
#include <EEPROM.h>


//#define DEBUG

#define MODE_SELECTOR

const float min_threshold = 50;  // The minimum rate on triggering a input 
const int cd_length = 20;///Buffer loop times. 
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.8;  //Dynamite threshold range.

const int outputDuration_pc = 30; // For PC. How long a key should be pressed when triggering a input.
const int outputDuration_ns = 30; // For NS. How long a key should be pressed when triggering a input.
const int outputDuration_sim = 10; // For NS. How long a key should be pressed when triggering a input.


//{A3, A0, A1, A2}
const uint16_t keymapping_ns[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};
const uint16_t keymapping_ns_2[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};
const uint16_t keymapping_ns_3[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};

const int keymapping[4] = {'f','d','j','k'};

int mode = 0; //0 for steam, 1 for switch, 2 for simulator; 
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

unsigned long previousMillisLK_2 = 0;
unsigned long previousMillisLD_2 = 0;
unsigned long previousMillisRD_2 = 0;
unsigned long previousMillisRK_2 = 0;

unsigned long previousMillisLK_3 = 0;
unsigned long previousMillisLD_3 = 0;
unsigned long previousMillisRD_3 = 0;
unsigned long previousMillisRK_3 = 0;


void setup() {
//  analogReference(INTERNAL);
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
}


void loop() {
  #ifdef DEBUG
  analogMonitor();
  #endif 

  #ifndef DEBUG
  unsigned long currentMillis = millis();
  //Keyboard=============================================== 
  if (mode == 0){
    if(buttonStatusLK != -1 && currentMillis - previousMillisLK_1 >= outputDuration_pc){
      Keyboard.release(keymapping[1]);
      buttonStatusLK = -1;
    }
    if(buttonStatusLD != -1 && currentMillis - previousMillisLD_1 >= outputDuration_pc){
      Keyboard.release(keymapping[0]);
      buttonStatusLD = -1;
    }
    if(buttonStatusRD != -1 && currentMillis - previousMillisRD_1 >= outputDuration_pc){
      Keyboard.release(keymapping[2]);
      buttonStatusRD = -1;
    }
    if(buttonStatusRK != -1 && currentMillis - previousMillisRK_1 >= outputDuration_pc){
      Keyboard.release(keymapping[3]);
      buttonStatusRK = -1;
    }
  }
//Switch===============================================  
  //LK===================
  else if (mode == 1){
    if(buttonStatusLK == 1 && currentMillis - previousMillisLK_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::ZL);
      SwitchControlLibrary().sendReport();
      buttonStatusLK = -1;
    }
    else if(buttonStatusLK == 2){
      if(currentMillis - previousMillisLK_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::ZL);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisLK_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::L);
        SwitchControlLibrary().sendReport();
        buttonStatusLK = -1;
      }
    }
    else if(buttonStatusLK == 3){
      if(currentMillis - previousMillisLK_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::ZL);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisLK_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::L);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisLK_3 >= outputDuration_ns){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport();
        buttonStatusLK = -1;
      }
    }
  //LD====================================================================
    if(buttonStatusLD == 1 && currentMillis - previousMillisLD_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::LCLICK);
      SwitchControlLibrary().sendReport();
      buttonStatusLD = -1;
    }
    else if(buttonStatusLD == 2){
      if(currentMillis - previousMillisLD_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::LCLICK);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisLD_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport();
        buttonStatusLD = -1;
      }
    }
    else if(buttonStatusLD == 3){
      if(currentMillis - previousMillisLD_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::LCLICK);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisLD_2 >= outputDuration_ns){
        SwitchControlLibrary().pressHatButton(Hat::DOWN);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisLD_3 >= outputDuration_ns){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport();
        buttonStatusLD = -1;
      }
    }
  //RD=============================================================
    if(buttonStatusRD == 1 && currentMillis - previousMillisRD_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::RCLICK);
      SwitchControlLibrary().sendReport();
      buttonStatusRD = -1;
    }
    else if(buttonStatusRD == 2){
      if(currentMillis - previousMillisRD_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::RCLICK);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisRD_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::Y);
        SwitchControlLibrary().sendReport();
        buttonStatusRD = -1;
      }
    }
    else if(buttonStatusRD == 3){
      if(currentMillis - previousMillisRD_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::RCLICK);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisRD_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::Y);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisRD_3 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::B);
        SwitchControlLibrary().sendReport();
        buttonStatusRD = -1;
      }
    }
//RK======================================================
    if(buttonStatusRK == 1 && currentMillis - previousMillisRK_1 >= outputDuration_ns){
      SwitchControlLibrary().releaseButton(Button::ZR);
      SwitchControlLibrary().sendReport();
      buttonStatusRK = -1;
    }
    else if(buttonStatusRK == 2){
      if(currentMillis - previousMillisRK_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::ZR);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisRK_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::R);
        SwitchControlLibrary().sendReport();
        buttonStatusRK = -1;
      }
    }
    else if(buttonStatusRK == 3){
      if(currentMillis - previousMillisRK_1 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::ZR);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisRK_2 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::R);
        SwitchControlLibrary().sendReport();
      }
      if(currentMillis - previousMillisRK_3 >= outputDuration_ns){
        SwitchControlLibrary().releaseButton(Button::X);
        SwitchControlLibrary().sendReport();
        buttonStatusRK = -1;
      }
    }
  }

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
    if(temp >= min_threshold*0.6 && mode == 0){
      switch(key){
        case 1:
          buttonStatusLK = 1;
          Keyboard.press(keymapping[key]);
          previousMillisLK_1 = currentMillis;
          break;
        case 0:
          buttonStatusLD = 1;
          Keyboard.press(keymapping[key]);
          previousMillisLD_1 = currentMillis;
          break;
        case 2:
          buttonStatusRD = 1;
          Keyboard.press(keymapping[key]);
          previousMillisRD_1 = currentMillis;
          break;
        case 3:
          buttonStatusRK = 1;
          Keyboard.press(keymapping[key]);
          previousMillisRK_1 = currentMillis;
          break;
      }
      delay(10);
    }
    else if(temp >= min_threshold*0.6 && mode == 1){
      switch(key){
        case 1:
          if(buttonStatusLK == -1){
            buttonStatusLK = 1;
            SwitchControlLibrary().pressButton(Button::ZL);
            previousMillisLK_1 = currentMillis;
            break;
          }
          else if(buttonStatusLK == 1){
            buttonStatusLK = 2;
            SwitchControlLibrary().pressButton(Button::L);
            previousMillisLK_2 = currentMillis;
            break;
          }
          else if(buttonStatusLK == 2){
            buttonStatusLK = 3;
            SwitchControlLibrary().pressHatButton(Hat::UP);
            previousMillisLK_3 = currentMillis;
            break;
          }
        case 0:
          if(buttonStatusLD == -1){
            buttonStatusLD = 1;
            SwitchControlLibrary().pressButton(Button::LCLICK);
            previousMillisLD_1 = currentMillis;
            break;
          }
          else if(buttonStatusLD == 1){
            buttonStatusLD = 2;
            SwitchControlLibrary().pressHatButton(Hat::RIGHT);
            previousMillisLD_2 = currentMillis;
            break;
          }
          else if(buttonStatusLD == 2){
            buttonStatusLD = 3;
            SwitchControlLibrary().pressHatButton(Hat::DOWN_RIGHT);
            previousMillisLD_3 = currentMillis;
            break;
          }
        case 2:
          if(buttonStatusRD == -1){
            buttonStatusRD = 1;
            SwitchControlLibrary().pressButton(Button::RCLICK);
            previousMillisRD_1 = currentMillis;
            break;
          }
          else if(buttonStatusRD == 1){
            buttonStatusRD = 2;
            SwitchControlLibrary().pressButton(Button::Y);
            previousMillisRD_2 = currentMillis;
            break;
          }
          else if(buttonStatusRD == 2){
            buttonStatusRD = 3;
            SwitchControlLibrary().pressButton(Button::B);
            previousMillisRD_3 = currentMillis;
            break;
          }
        case 3:
          if(buttonStatusRK == -1){
            buttonStatusRK = 1;
            SwitchControlLibrary().pressButton(Button::ZR);
            previousMillisRK_1 = currentMillis;
            break;
          }
          else if(buttonStatusRK == 1){
            buttonStatusRK = 2;
            SwitchControlLibrary().pressButton(Button::R);
            previousMillisRK_2 = currentMillis;
            break;
          }
          else if(buttonStatusRK == 2){
            buttonStatusRK = 3;
            SwitchControlLibrary().pressButton(Button::X);
            previousMillisRK_3 = currentMillis;
            break;
          }
      }

      SwitchControlLibrary().sendReport();
      delay(10);
    }
    else if(temp >= min_threshold*0.6 && mode == 2){
      Keyboard.press(keymapping[key]);
      delay(outputDuration_sim);
      Keyboard.release(keymapping[key]);
    }
  }
  
  if(threshold < min_threshold){
    threshold = min_threshold;
  }
  else if(threshold > min_threshold) {
    threshold = threshold*k_decay;
  }
  #endif
}

void extendKey(){
  if (mode == 1){
    if(digitalRead(0) == HIGH && digitalRead(1) == LOW){
      pushHat(Hat::UP);
    }
    if(digitalRead(1) == HIGH && digitalRead(0) == LOW){
      pushHat(Hat::DOWN);
    }

    if(digitalRead(1) == LOW && digitalRead(0) == LOW){
      pushButton(Button::B);
    }
  }
}

void analogMonitor(){
  Serial.print("||");
  Serial.print(analogRead(A0));
  Serial.print("||");
  Serial.print(analogRead(A1));
  Serial.print("||");
  Serial.print(analogRead(A2));
  Serial.print("||");
  Serial.print(analogRead(A3));
  Serial.println("||");
}