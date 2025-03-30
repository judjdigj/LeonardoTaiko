#include <Keyboard.h>
#include <NintendoSwitchControlLibrary.h>

#define DEBUG

bool pressedStatus = 0;
const int initialThredholds = 30;
int bufferTime = 1;

int key = -1;
const int minThredholds = 0.6*initialThredholds;
int valueLK, valueLD, valueRD, valueRK = 0;
int peakLK, peakLD, peakRD, peakRK = 0;
//int currentLK, currentLD, currentRD, currentRK = 0;
int previousLK, previousLD, previousRD, previousRK = 0;

void setup() {
  Serial.begin(115200);
//  ADCSRA = (ADCSRA & 0xf8) | 0x04;
}

void loop() {

#ifdef DEBUG
  analogMonitorA0();
#endif

#ifndef DEBUG
//Signal Peak Detection
  int leftLK = analogRead(A0);
  int currentLK = analogRead(A0);
  int rightLK = analogRead(A0);

  if(currentLK >= rightLK && currentLK >= leftLK){
    peakLK = currentLK;
  }


//Hit Detection
  valueLK = analogRead(A0);
  valueLD = analogRead(A3);
  valueRD = analogRead(A1);
  valueRK = analogRead(A2);

  if(valueLK - peakLK >= initialThredholds){
    pressedStatus = 1;
  }
  if(valueLD - peakLD >= initialThredholds){
    pressedStatus = 1;
  }
  if(valueRD - peakRD >= initialThredholds){
    pressedStatus = 1;
  }
  if(valueRK - peakRK >= initialThredholds){
    pressedStatus = 1;
  }
  
//Buffer

  if(pressedStatus){
    unsigned long previousTime = millis();
    unsigned long currentTime = previousTime;
    int maxValueLK = 0;
    int currentValueLK = 0;

    while(currentTime - previousTime <= bufferTime){
      currentValueLK = analogRead(A0);
      if(maxValueLK < currentValueLK){
        maxValueLK = currentValueLK;
      }
      currentTime = millis();
    }
    peakLK = maxValueLK;
  }

//Key Output
  if(key != -1){
    Serial.println(key);
  }
  key = -1;
  pressedStatus = 0;
  delay(7);

#endif

}

void analogMonitorA0(){
  int value = analogRead(A0);
  if(value > 10){
    int previousTime = millis();
    int currentTime = previousTime;
    while(currentTime - previousTime < 1){
      Serial.println(analogRead(A0));
      currentTime = millis();
    }
  //  Serial.println("==============");
  }
}
