#include <Keyboard.h>
#include <KeyboardLayout.h>
#include <NintendoSwitchControlLibrary.h>
#include <EEPROM.h>

const float min_threshold = 100;  // The minimum rate on triggering a input
const int cd_length = 20; //Buffer loop times.
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.7;  //Dynamite threshold range.
const int outputDuration_pc = 7; // For PC. How long a key should be pressed when triggering a input.
const int outputDuration_ns = 25; // For NS. How long a key should be pressed when triggering a input.

//{A3, A0, A1, A2}
const uint16_t keymapping_ns[4] = {Button::A, Hat::UP, Hat::DOWN, Button::B};
const int keymapping[4] = {'k','d','f','j'};
const uint16_t keymapping_ns_extend[] = {Button::PLUS, Hat::RIGHT};

int mode; //0 for keyboard, 1 for switch
int outputDuration;
int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];
int threshold = min_threshold;

void setup() {
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  int pc_status = digitalRead(0);
  int ns_status = digitalRead(1);
  if (ns_status == LOW && pc_status == HIGH){
    mode = 1;
    outputDuration = outputDuration_ns; 
    EEPROM.write(0, 1);
    }
  if (pc_status == LOW && ns_status == HIGH){
    mode = 0;
    outputDuration = outputDuration_pc; 
    EEPROM.write(0, 0);
    }
  else{
    mode = EEPROM.read(0);
    if(mode == 0){
      outputDuration = outputDuration_pc;
      }
    if(mode == 1){
      outputDuration = outputDuration_ns;
      }
    }
//  Serial.begin(9600);
  if(mode == 1){  
    pushButton(Button::A, 500, 3); //initialize on switch
    }
  if(mode == 0){
    Keyboard.begin();
  }
}

void loop() {

//  analogMonitor();
//  extendKey();
    
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
    key = count%4;
    Serial.println(temp);
    Serial.println(threshold);
    Serial.println(key);
    if(mode == 0){
      Keyboard.press(keymapping[key]);
      delay(outputDuration);
      Keyboard.releaseAll();
    }
    else if(mode == 1){
      if( key ==1 || key == 2 ){
        SwitchControlLibrary().pressHatButton(keymapping_ns[key]);
        SwitchControlLibrary().sendReport();
        delay(outputDuration);
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport();
      }
else{
        SwitchControlLibrary().pressButton(keymapping_ns[key]);
        SwitchControlLibrary().sendReport();
        delay(outputDuration);
        SwitchControlLibrary().releaseButton(keymapping_ns[key]);
        SwitchControlLibrary().sendReport();
      }
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
