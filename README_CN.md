# LeonardoTaiko

基础且简易的太鼓达人电控盒，使用Arduino Leonardo制作，支持Pro Micro。

![Yawaraka Tank](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/main/pics/result.jpg)  
软软战车里魔王

[English](https://github.com/judjdigj/LeonardoTaiko/tree/main)

## 特性

* 使用Arduino IDE编写
* 支持Nintendo Switch
* 制作简单

## 硬件部分
* Arduino Leonardo/Pro Micro
* 压电传感器/震动传感器 x4  
* 1MOhm电阻 x4  
* 面包板，导线等（你也可以自行绘制pcb）


### 电路连接
四个传感器，一脚连接到GND，另一脚分别连接到A0-A3，并且并联上电阻。

注意:  
压电传感器正常工作时，会生成正电压和负电压。所以理论上说，压电传感器的正负极应该没有什么区别。然而我也不太确定负电压会对Arduino的ADC产生什么影响。如果比较担心这个的话，可以使用二极管或者桥式整流器进行负电压阻断/反转。

## 使用教程
先在Arduino IDE的库管理器中下载[Keyboard](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/) and [NintendoSwitchLibrary](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/)两个库。  
安装完成后直接编译上传到开发板，应该就能使用了。默认的键盘按键映射是DFJK。

### Nintendo Switch支持
首先需要更改开发板的VID和PID。   
在 ```board.txt```( Arduino IDE 1.8.x )文件中，相关部分修改为：
```
leonardo.vid.1=0x0f0d
leonardo.pid.1=0x0092

leonardo.build.vid=0x0f0d
leonardo.build.pid=0x0092
```
 ```board.txt``` 文件的地址，根据IDE版本的不同而不同。   
然后在```LeonardoTaiko.ino```文件中，修改如下代码：
```
//#define SWITCH
#define KEYBOARD
```
改成：
```
#define SWITCH
//#define KEYBOARD
```
### 按键映射

```
const uint16_t keymapping_ns[4] = {Button::LCLICK, Button::ZL, Button::RCLICK, Button::ZR};

const int keymapping[4] = {'f','d','j','k'};
```
更改这两个数组里的值来实现按键映射。  

这里附上Switch的按键定义(具体可参考[Nintendo Switch Library](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/)):
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
## 算法解释
敲击后，当传感器读数超过阈值```threshold```时，便会触发一个输入。

### 防串音
街机太鼓的感应区由四张木板构成，每个木板上会固定一个振动传感器。
![Taiko](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/TaikoStructure.jpg)

然而，不同于电子鼓各个部分完全分开，太鼓的四个部分虽然分开，但是在某些地方是连在一起的。无论是后面做基盘的木板，还是橡胶鼓皮，这些部分都或多或少将四个本应分开的部分连接在了一起。哪怕使用减震器，减震海绵，或多或少都会产生影响。导致敲击时，不只是被敲击的部分，而是所有的部分都会产生震动。
![Noise](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_171911.jpg)
其他非敲击区域产生的震动如果同样超过了阈值，那就有可能带来串音，如图所示。   

为了避免串音，我们设计一个缓存区，当识别到一次敲击的时候，我们便持续将接下来一段时间四个传感器接收到的信号存入缓存区。正确安装传感器时，被敲击区域产生的信号一定比其他区域产生的信号大，我们便对缓存区内的数据进行最大值查找，产生最大值信号的传感器一定为被敲击传感器。
![noise2](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_172301.jpg)
也因此，缓存区的大小决定了敲击到响应的时间。如果缓存区太长，就会有一种延迟感，如果缓存区太短，就有可能没法缓存到真正最大值。具体将会在下文讨论。

### 防抖动

由于电路性质，一个开关通断时，可能会在通断的状态中产生抖动，例如按键，可能会产生按下一次，输入两次的状况。   
在本算法中，同样也会产生类似的情况，但是产生的原因完全不同。   
如果缓存区和按键按压持续时间设置得太小，那么当这个周期结束时，鼓的震动仍在持续，传感器仍然在输出信号，如果这个信号值仍然大于阈值，那么会再次出发一个按键识别周期，造成一次敲击，判定两次的抖动现象。
![noise3](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_172950.jpg)
延长缓存时间，延长按键事件，提高触发阈值，都可以解决问题。可是，如果缓存时间设置太长，那输入会产生明显延迟；如果按键时间设置太长，则滚奏连打性能会变差，如果阈值设置过高，则鼓的灵敏度会降低。   
因此，我们引入动态阈值。
![noise4](https://raw.githubusercontent.com/judjdigj/LeonardoTaiko/develop/pics/Notes_240218_173352.jpg)
由于打击力度和传感器信号大小有正相关，因此我们设计一个动态阈值，敲击后，读取到传感器的最大值，并将该值乘上参数```k_increase```。当一个按键周期结束后，根据敲击力度，触发阈值会相应提升，后续的震动产生二次判定，从而消除抖动。   
引入动态阈值后，可以在较低触发阈值，缓存时间，按键时间的情况下，实现无抖动。

### 同时输入 (大打音符)

本算法并不支持真正意义上的同时输入。然而通过修改按键映射部分的代码，可以实现敲击一次，同时输入两个按键，从而实现大打效果，比较邪道。

### 其他
你也可以尝试将引入一些平滑算法对原始数据进行预处理。虽然我觉得传感器的原始数据已经够用了。

## 参数解释:

### ```threshold```

触发阈值，设置得越低，鼓越灵敏。但是如果太低，低于传感器本身的原始噪音，那就会产生虚空输入。

### ```outputDuration```
当一个输入被触发时，会映射到按键按压。这个参数决定这个按压时间有多长。设置得越短，连打性能越优秀。

然而某些设备中（例如Nintendo Switch），过低的按压时间（20ms以下）会无法被设备识别。

### ```cd_length```
缓存区循环读取```analogValue```的循环次数。设置的越低，敲击延迟越低，但是过低的缓存区可能会导致缓存区无法读取到真正的最高值。

 ```cd_length``` 代表着四个传感器读取一遍这一周期循环的次数，所以```buffer_size```的大小为```4*cd_length```.

### ```k_increase```
当敲击产生时，缓存中的最大值会乘上```k_increase```，实现阈值提升，避免```cd_length```/```buffer_size``` 过低时产生的抖动

### ```k_decay```
每一次程序循环中，当前阈值都会乘上```k_decay```，直到阈值回到初始设定的阈值。


## 鸣谢
Nintendo Switch支持：
[Nintendo Switch Library](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/) by [lefmarna](https://github.com/lefmarna).  
算法由多个项目启发，包括：  
 [ArduinoTaikoController](https://github.com/LuiCat/ArduinoTaikoController) by [LuiCat](https://github.com/LuiCat).  
[Taiko-Input](https://github.com/sachikoxz12/Taiko-Input) by [sachikoxz12](https://github.com/sachikoxz12).