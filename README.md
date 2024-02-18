# LeonardoTaiko

A basic and easy to use e-box with Arduino Leonardo/ProMicro.   

![Yawaraka Tank](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/main/pics/result.jpg)  
Yawaraka Tank Ura

## Feature

* Written in Arduino IDE.
* Nintendo Switch Support
* Easy to build.

## Hardware Parts
* Arduino Leonardo/Pro Micro  
* Piezo sensor x4  
* 1MOhm resistor x4  
* Bread borad and wires(you can print your own pcb if you want)  
![board](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/main/pics/wiring.png)  

Circuit pics from [ArduinoTaikoController](https://github.com/LuiCat/ArduinoTaikoController).

Note:
Piezo sensor will generate ositive and negative voltage when working, so positive/negative to GND probably doesn't matter. However I'm not sure if negative voltage will cause any damage to ADC on Arduino Leonardo. If that concern you, you can use diodes or diode bridges to fix this issue.  

## How to use
You need to download [Keyboard](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/) and [NintendoSwitchLibrary](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/) in your Arduino IDE's library manager first.  

Then upload to the board. then it should work fine. By default it should be mapping to DFJK on keyboard.

### Switch Support
You need to change the VID and PID first.   
In ```board.txt```( Arduino IDE 1.8.x ).
```
leonardo.vid.1=0x0f0d
leonardo.pid.1=0x0092

leonardo.build.vid=0x0f0d
leonardo.build.pid=0x0092
```
Location of ```board.txt``` can be various depends on your IDE version.   
Then in the ```LeonardoTaiko.ino```, change this:F
```
//Uncomment to use Switch mode.
/*
    SwitchControlLibrary().pressButton(keymapping_ns[key]);
    SwitchControlLibrary().sendReport();
    delay(outputDuration);
    SwitchControlLibrary().releaseButton(keymapping_ns[key]);
    SwitchControlLibrary().sendReport();
*/
//Uncomment to use keyboard mode.

    Keyboard.press(keymapping[key]);
    delay(outputDuration);
    Keyboard.releaseAll();
```
to this:
```
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
```
### Keymapping

```
const uint16_t keymapping_ns[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};

const int keymapping[4] = {'f','d','j','k'};
```
Change the value to change the keymapping.   

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
## Algorithm
Once a analog value is higher the ```threshold```, a input will be detected. 

### To Avoid Mistaken Input
We all know AC Taiko was built with 4 parts: left rim, left surface, right surface, right rim.
![Taiko](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/TaikoStructure.jpg)

However, unlike electronic drum set, where each piezo sensors are seperated, there will always be some connection between each part of the Taiko drum, no matter the base wooden plate or the rubber surface. So when you hit one part, the other 3 parts will also be vibrated, causing all the sensors delivering signals.
![Noise](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_171911.jpg)
If the noise signal from the other parts was detected before the actual part that got hit, a mistaken input will happen.

To prevent mistaken input, it will start storage all the analog input into buffer for a short period. Since the noise analog value on other sensor should be smaller than the one which actual got hit, comparing each value in buffer, the largest value should be generated by the actual sensor got hit.
![noise2](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_172301.jpg)
So after some calculation, you should get the right sensor got hit.

### To Avoid Double input on One Hit.

If the buffer time and key press time is too short, even shorter than the vibrating time, when the whole process was over, the sensor will still sent signal, and again once it's higher than the threshold, another input will be triggered. Causing double input in one hit. The harder you hit the drum, the more significant this issue will be.
![noise3](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_172950.jpg)
That's why I imported dynamic threshold. the harder you hit the drum, the higher the threshold will be raised. which let the threshold higher than the vibration.
![noise4](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_173352.jpg)
The dynamic threshold will be the maximun analogValue in buffer multiplied by ```k_increase```. and in every loop the current threshold will be multiplied by ```k_decay``` until it's back to the original threshold.

### Simultaneous Input (Big Notes)

This algorithm, doesn't support simultaneous input (you need that to hit big notes on console version of Taiko no Tatsujin for a higher score). However technically you can do some key mapping trick to make one hit equal to 2 key pressed.

### Others
You can also imply some smoothing filter to preprocessing the raw analog input signal. In my case, the sensors are good enough.

## Parameters Explain:

### ```threshold```

The value to trigger a input. The lower it was set, the more sensitive the drum will get. If it's lower than the idle noises which piezo sensor will definitely generated, random input will occur.

### ```outputDuration```
When an input is triggered, a key will be pressed. This parameter decide how long a key should be pressed. During the this period, there will be no other action.  

The longer it was set, the less roll you can get from

In some devices (e.g. Nintendo Switch). A really short button press time (below 20ms) will not be recognized.

### ```cd_length```
How many times will loop to read all 4 sensors' ```analogValue```. The smaller it was set, the faster response you will get after one hit. and it will more likely causing mistaken input.

Since ```cd_length``` define one loop for all 4 sensors, ```buffer_size``` should be ```4*cd_length```.

### ```k_increase```
Every time a hit was detected, the threshold will change to the largest pin value multiplied by ```k_increase```. Which can prevent double input when the ```cd_length```/```buffer_size``` was set too low.

### ```k_decay```
Every loop the current threshold will multiply ```k_decay``` in order to go back to the original threshold.


###

## Credit
Switch support from
[Nintendo Switch Library](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/) by [lefmarna](https://github.com/lefmarna).  
Algorithm inspired by multiple Taiko project, including:  
 [ArduinoTaikoController](https://github.com/LuiCat/ArduinoTaikoController) by [LuiCat](https://github.com/LuiCat).  
[Taiko-Input](https://github.com/sachikoxz12/Taiko-Input) by [sachikoxz12](https://github.com/sachikoxz12).