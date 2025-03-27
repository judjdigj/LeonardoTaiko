#include <Keyboard.h>
#include <NintendoSwitchControlLibrary.h>

bool pressedStatus = 0;
const int initialThredholds = 30;
int bufferTime = 1;


int key = -1;
const int minThredholds = 0.6*initialThredholds;
int valueLK, valueLD, valueRD, valueRK = 0;
int peakLK, peakLD, peakRD, peakRK = 20;
int currentLK, currentLD, currentRD, currentRK = 0;

void setup() {

}

void loop() {
//Signal Peak Detection
  currentLK = analogRead(A0);
  currentLD = analogRead(A3);
  currentRD = analogRead(A1);
  currentRK = analogRead(A2);
  if(currentLK > peakLK){
    peakLK = currentLK;
  }
  if(currentLD > peakLD){
    peakLD = currentLD;
  }
  if(currentRD > peakRD){
    peakRD = currentRD;
  }
  if(currentRK > peakRK){
    peakRK = currentRK;
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
    unsigned long currentTime = millis();
    int maxValue = 0;
    int currentValue = 0;

    while(currentTime-previousTime <= bufferTime){
      for(int i=A0; i<=A3; i++){
        currentValue = analogRead(i);
        if(currentValue > maxValue){
          maxValue = currentValue;
          key = i;
        }
      }
      currentTime = millis();
    }
  }

//Key Output
  Serial.println(key);
  key = -1;
  pressedStatus = (0);
  delay(7);
}
