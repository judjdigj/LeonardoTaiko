#include <Keyboard.h>
#include <KeyboardLayout.h>
#include <NintendoSwitchControlLibrary.h>


const int min_threshold = 200;  // The minimum rate on triggering a input
const int outputDuration = 25;  // How long a key should be pressed when triggering a input.
const int cd_length = 25; //Buffer loop times.
const float k_decay = 0.99; //decay speed on the dynamite threshold.
const float k_increase = 0.7;  //Dynamite threshold range.

const uint16_t keymapping_ns[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};

const int keymapping[4] = {'f','d','j','k'};

int key;
const int buffer_size = cd_length*4;
int buffer[buffer_size];
int threshold = min_threshold;

void setup() {
  pushButton(Button::B, 500, 5); //initialize on switch
  Serial.begin(9600);
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

//Uncomment to use Switch mode.
    SwitchControlLibrary().pressButton(keymapping_ns[key]);
    SwitchControlLibrary().sendReport();
    delay(outputDuration);
    SwitchControlLibrary().releaseButton(keymapping_ns[key]);
    SwitchControlLibrary().sendReport();

//Uncomment to use keyboard mode.
/*
    Keyboard.press(keymapping[key]);
    delay(outputDuration);
    Keyboard.releaseAll();
*/

//
  if(threshold < min_threshold){
    threshold = min_threshold;
  }
  if(threshold > min_threshold){
    threshold = threshold*k_decay;
    }
  }
}
