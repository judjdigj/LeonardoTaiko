# LeonardoTaiko

[中文文档](https://github.com/judjdigj/LeonardoTaiko/blob/main/README_CN.md)

A easy to build e-box with Arduino Leonardo/ProMicro. Huge thanks to [lty2008one](https://github.com/lty2008one) for improving performance on Switch.

![Senpuu no Mai](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/20240221_155149.jpg?raw=true)  
Play demo: Senpuu no Mai [Heaven] Full Combo.

## Feature

* Nintendo Switch Support (with good performace)
* Easy to build.

## Hardware Parts
* Arduino Leonardo/Pro Micro  
* Piezo sensor x4  
* 1MOhm resistor x4  
* Bread borad and wires(you can print your own pcb if you want)  

### Circuit Connection
4 piezo sensors, one pin to the GND, and the other to the A0-A3. Connect 1MOhm resistor to each sensor parallelly.

**Note**:
Piezo sensor will generate ositive and negative voltage when working, so positive/negative to GND probably doesn't matter. You've heard that negative voltage will probably damage the analog pin. However since the current generated by the sensor is really really low, I would say it's most likely safe. If that concern you, you can use diodes or diode bridges to fix this issue.

## How to use
In Arduino IDE, board manager, download Arduino AVR Board.

Then, you need to download [Keyboard](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/) and [NintendoSwitchLibrary](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/) in your Arduino IDE's library manager.

Then compile and upload the code to the board. then it should work fine.

### Nintendo Switch Support
You need to change the VID and PID first.   
In ```board.txt```( Arduino IDE 1.8.x ).
```
leonardo.vid.1=0x0f0d
leonardo.pid.1=0x0092

leonardo.build.vid=0x0f0d
leonardo.build.pid=0x0092
```
Location of ```board.txt``` can be various depends on your IDE version. For Arudino IDE 1.8.19, it should be in ```C:\Users\USERNAME\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6```

Then connect Pin1 to GND, hit reset button. (Or plug into the Switch while connecting) It should work fine now.

To switch back to PC mode, connect Pin0 to GND and hit reset button. (or plug into the PC while connecting).

### Extend Key
Uncomment ```extendKey()``` can map ```D0``` and ```D1``` to ```Button::PLUS``` and ```Hat::RIGHT```. In case you want configuration in NS2. However I'm sure sure if there will be any negative effect on the perfermance.

### Keymapping

```
const KeyUnion NS_LEFT_KATSU[4]  = {{Button::ZL, NS_BTN, NS_BTN_DUR, 0, false}, {Button::L, NS_BTN, NS_BTN_DUR, 0, false}, {Hat::UP, NS_HAT, NS_HAT_DUR, 0, false}, {Hat::LEFT, NS_HAT, NS_HAT_DUR, 0, false}};
const KeyUnion NS_LEFT_DON[3]    = {{Button::LCLICK, NS_BTN, NS_BTN_DUR, 0, false}, {Hat::RIGHT, NS_HAT, NS_HAT_DUR, 0, false}, {Hat::DOWN, NS_HAT, NS_HAT_DUR, 0, false}};
const KeyUnion NS_RIGHT_DON[3]   = {{Button::RCLICK, NS_BTN, NS_BTN_DUR, 0, false}, {Button::Y, NS_BTN, NS_BTN_DUR, 0, false}, {Button::B, NS_BTN, NS_BTN_DUR, 0, false}};
const KeyUnion NS_RIGHT_KATSU[4] = {{Button::ZR, NS_BTN, NS_BTN_DUR, 0, false}, {Button::R, NS_BTN, NS_BTN_DUR, 0, false}, {Button::X, NS_BTN, NS_BTN_DUR, 0, false}, {Button::A, NS_BTN, NS_BTN_DUR, 0, false}};
const KeyUnion PC_LEFT_KATSU[1]  = {{'d', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_LEFT_DON[1]    = {{'f', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_RIGHT_DON[1]   = {{'j', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_RIGHT_KATSU[1] = {{'k', PC_BTN, PC_BTN_DUR, 0, false}};
```
This part of the codes represent the keymapping.  

Note that since Switch needs at least 20ms pressing time to recongnized button, which can slow down the performance, we use ```keyUnion``` to let the key be pressed one by one.  
For examples, on Switch version of the game, left don was usually mapped to LCLICK, RIGHT and DOWN, so when left don was hit, the LCLICK will be pressed for 35ms.  
During this time, if another hit on the left don was detected, the RIGHT key will be pressed for another 35ms. So as the DOWN key after another hit. It feels like it "rolls" through all the possible button mapping to left don.  
With this extra step, you can get threotically 3 times performance than the [original code](https://github.com/judjdigj/LeonardoTaiko/tree/original) on Switch, which result in one roll for 12 hit.

Switch button definition list (more information at [Nintendo Switch Library](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/)):
```
Button::Y
Button::B
Button::A
Button::X
Button::L
Button::R
Button::ZL
Button::ZR
Button::MINUS
Button::PLUS
Button::LCLICK
Button::RCLICK
Button::HOME
Button::CAPTURE
Hat::UP
Hat::UP_RIGHT
Hat::RIGHT
Hat::DOWN_RIGHT
Hat::DOWN
Hat::DOWN_LEFT
Hat::LEFT
Hat::UP_LEFT
Hat::NEUTRAL
```

## Debug
uncomment ```analogMonitor()``` and comment all the section in main loop below. Then you can check each sensors analog value after passing the ```threshold```.

Also some commented line can be found for different debug purposes.
```
//    Serial.println("DECAY");
//    Serial.println(threshold);  //Check decay, in order to set proper k_increase and k_decay.
```

## Algorithm
Once a analog value is higher the ```threshold```, a input will be detected. 

### To Avoid Mistaken Input
We all know AC Taiko was built with 4 parts: left rim, left surface, right surface, right rim.
![Taiko](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/TaikoStructure.jpg?raw=true)

However, unlike electronic drum set, where each piezo sensors are seperated, there will always be some connection between each part of the Taiko drum, no matter the base wooden plate or the rubber surface. So when you hit one part, the other 3 parts will also be vibrated, causing all the sensors delivering signals.
![Noise](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_171911.jpg?raw=true)
If the noise signal from the other parts was detected before the actual part that got hit, a mistaken input will happen.

To prevent mistaken input, we need to create a buffer window. It will start storage all the analog input into buffer for a short period. Since the noise analog value on other sensor should be smaller than the one which actual got hit, comparing each value in buffer, the largest value should be generated by the actual sensor got hit.
![noise2](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_172301.jpg?raw=true)

The parameter ```cd_length``` will effect the buffer size. If it was set too large, it will take a significant time to find the largest number, causing input latency. If it was too small, the buffer windows may not cover the largest value from the sensor.

### To Avoid Double input on One Hit.

If the buffer time and key press time is too short, even shorter than the vibrating time, when the whole input process is over, the sensor will still sent signal, and again once it's higher than the threshold, another input will be triggered. Causing double input in one hit. The harder you hit the drum, the more significant this issue will be.
![noise3](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_172950.jpg?raw=true)
That's why I imported dynamic threshold. the harder you hit the drum, the higher the threshold will be raised. which let the threshold higher than the vibration.
![noise4](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_173352.jpg?raw=true)
The dynamic threshold will be the maximun analogValue in buffer multiplied by ```k_increase```. and in every loop the current threshold will be multiplied by ```k_decay``` until it's back to the original threshold.

### Simultaneous Input (Big Notes)

This algorithm technicallysupport simultaneous input (you need that to hit big notes on console version of Taiko no Tatsujin for a higher score). Technically you can't do that which this code. However with the new button pressing code, 2 input can happened at a really short time. Which is good enough for Switch to recongnized it as simultaneous input.

### Others
You can also imply some smoothing filter to preprocessing the raw analog input signal. In my case, the sensors are good enough.

## Parameters (and the recommended value):

### ```min_threshold = 75```

The value to trigger a input. use 5V as reference, divided the signal from 0 to 1024. The lower it was set, the more sensitive the drum will get. If it's lower than the idle noises which piezo sensor will definitely generated, random input will occur.

### ```cd_length = 20```
How many loops to read all 4 sensors' ```analogValue```. Since ```cd_length``` define one loop for all 4 sensors, ```buffer_size``` should be ```4*cd_length```.  
Which means you can change this value to adjust the buffer size. The smaller it was set, the faster response you will get after hit the drum. 

### ```k_increase = 0.7```
Every time a hit was detected, the threshold will change to the largest pin value multiplied by ```k_increase```. Which can prevent double input when the ```cd_length```/```buffer_size``` was set too low.

### ```k_decay = 0.99```
Every loop the current threshold will multiply ```k_decay``` in order to go back to the original threshold.


###

## Credit
Nintendo Switch support from
[NintendoSwitchControlLibrary](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/) by [lefmarna](https://github.com/lefmarna).  
Algorithm inspired by multiple Taiko project, including:  
 [ArduinoTaikoController](https://github.com/LuiCat/ArduinoTaikoController) by [LuiCat](https://github.com/LuiCat).  
[Taiko-Input](https://github.com/sachikoxz12/Taiko-Input) by [sachikoxz12](https://github.com/sachikoxz12).
