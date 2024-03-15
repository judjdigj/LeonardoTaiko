# LeonardoTaiko

[English](https://github.com/judjdigj/LeonardoTaiko/tree/main)

简易的太鼓达人电控盒，使用Arduino Leonardo制作，支持Pro Micro。 感谢[lty2008one](https://github.com/lty2008one)的代码，让Switch模式下的性能表现有了巨大提升。

![Senpuu no Mai](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/20240221_155149.jpg?raw=true)  
旋风之舞【天】全连

## 特性

* 支持Nintendo Switch （而且性能不错）
* 制作简单

## 硬件部分
* Arduino Leonardo/Pro Micro
* 压电传感器/震动传感器 x4  
* 1MOhm电阻 x4  
* 面包板，导线等（你也可以自行绘制pcb）


### 电路连接
四个传感器，一脚连接到GND，另一脚分别连接到A0-A3，并且并联上电阻。

**注意**:  
压电传感器正常工作时，会生成正电压和负电压。所以理论上说，压电传感器的正负极应该没有什么区别。一般来说负电压会对AnalogInput脚会造成损害。然而压电传感器产生的电流实在是太小太小了，因此我认为由其产生的负电压是大概率不会对AnalogInput产生负面影响的。当然，如果比较担心这个的话，可以使用二极管或者桥式整流器进行负电压阻断/反转。

## 使用教程
使用Arduino IDE打开，先在开发板管理器中下载Arduino AVR Board，然后在Arduino IDE的库管理器中下载[Keyboard](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/)和[NintendoSwitchLibrary](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/)两个库。  
安装完成后直接编译上传到开发板，应该就能使用了。

### Nintendo Switch支持
首先需要更改开发板的VID和PID。
在 ```board.txt```文件中，相关部分修改为：
```
leonardo.vid.1=0x0f0d
leonardo.pid.1=0x0092

leonardo.build.vid=0x0f0d
leonardo.build.pid=0x0092
```
无论是IDE是1.8还是2，通过Board Manager下载AVR Boards后，```board.txt``` 的位置都在```C:\Users\USERNAME\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6```  
对于Linux用户，位置在```~/Arduino15/packages/arduino/hardware/avr/1.8.6```

然后把Pin0和GND短接，按下Reset按键（或者Pin0和GND短接并把板子插入Switch），然后应该就可以正常使用了。

切换回PC模式的话，把Pin1和GND短接，按下Reset按键（或者Pin1和GND短接并把板子插入PC）。

### 按键扩展
取消注释```extendKey()```，```D0```和```D1```接地会分别被映射成```Button::PLUS```，```Hat::RIGHT```，在NS2中可用于进行演奏设置。然而我不确定是否会对性能产生影响。

### 按键映射
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
这部分代码为按键映射相关代码。  
**值得注意的是**，为了使Switch识别按压按键，一个按键的按压时长需要至少20ms，这会极大降低性能表现。为了突破各个瓶颈，我们使用```keyUnion```，让对应按键一个接一个被按下。
举个例子，当我们敲击左咚时，```LCLICK```会被按下，并持续```NS_BTN_DUR```时间（此处为35ms，可在```pressKey.h```中修改），在此期间内如果左咚再次被敲击，则```RIGHT```会被按下，同样的，如果期间再次被敲击，则按下```DOWN```，相当于有一个无形的手“搓”过这三个按键。
这种做法带来的是和[原本代码](https://github.com/judjdigj/LeonardoTaiko/tree/original)相比至少理论上3-4倍的性能提升，本人测试中，单手一震可达到12打。

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

## 调试
取消注释 ```analogMonitor()``` 并将后面主loop的全部内容全部注释掉，即可在串口编辑器中查看当模拟值超过 ```threshold```后的表现。可用于参考设置阈值的大小。

另外，有几行被注释掉的代码也可用于调试，请见代码注释
```
//    Serial.println("DECAY");
//    Serial.println(threshold);  //Check decay, in order to set proper k_increase and k_decay.
```


## 算法解释
敲击后，当传感器读数超过阈值```threshold```时，便会触发一个输入。

### 防串音
街机太鼓的感应区由四张木板构成，每个木板上会固定一个振动传感器。
![Taiko](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/TaikoStructure.jpg?raw=true)

然而，不同于电子鼓各个部分完全分开，太鼓的四个部分虽然分开，但是在某些地方是连在一起的。无论是后面做基盘的木板，还是橡胶鼓皮，这些部分都或多或少将四个本应分开的部分连接在了一起。哪怕使用减震器，减震海绵，或多或少都会产生影响。导致敲击时，不只是被敲击的部分，而是所有的部分都会产生震动。
![Noise](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_171911.jpg?raw=true)
其他非敲击区域产生的震动如果同样超过了阈值，那就有可能带来串音，如图所示。   

为了避免串音，我们设计一个缓存区，当识别到一次敲击的时候，我们便持续将接下来一段时间四个传感器接收到的信号存入缓存区。正确安装传感器时，被敲击区域产生的信号一定比其他区域产生的信号大，我们便对缓存区内的数据进行最大值查找，产生最大值信号的传感器一定为被敲击传感器。
![noise2](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_172301.jpg?raw=true)
也因此，缓存区的大小决定了敲击到响应的时间。如果缓存区太长，就会有一种延迟感，如果缓存区太短，就有可能没法缓存到真正最大值。具体将会在下文讨论。

### 防抖动

由于电路性质，一个开关通断时，可能会在通断的状态中产生抖动，例如按键，可能会产生按下一次，输入两次的状况。   
在本算法中，同样也会产生类似的情况，但是产生的原因完全不同。   
如果缓存区和按键按压持续时间设置得太小，那么当这个周期结束时，鼓的震动仍在持续，传感器仍然在输出信号，如果这个信号值仍然大于阈值，那么会再次出发一个按键识别周期，造成一次敲击，判定两次的抖动现象。
![noise3](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_172950.jpg?raw=true)
延长缓存时间，延长按键事件，提高触发阈值，都可以解决问题。可是，如果缓存时间设置太长，那输入会产生明显延迟；如果按键时间设置太长，则滚奏连打性能会变差，如果阈值设置过高，则鼓的灵敏度会降低。   
因此，我们引入动态阈值。
![noise4](https://github.com/judjdigj/LeonardoTaiko/blob/main/pics/Notes_240218_173352.jpg?raw=true)
由于打击力度和传感器信号大小有正相关，因此我们设计一个动态阈值，敲击后，读取到传感器的最大值，并将该值乘上参数```k_increase```。当一个按键周期结束后，根据敲击力度，触发阈值会相应提升，后续的震动产生二次判定，从而消除抖动。   
引入动态阈值后，可以在较低触发阈值，缓存时间，按键时间的情况下，实现无抖动。

### 同时输入 (大打音符)

本算法并支持大打。理论上讲，本算法并不支持同时输入，但是优化后的按键按压算法，让两个按键的输入可以发生在极短的时间内，对于NS平台来说，足够让其认为是同时输入，从而处理大打音符。

### 其他
你也可以尝试将引入一些平滑算法对原始数据进行预处理。虽然我觉得传感器的原始数据已经够用了。

## 参数解释（并附带推荐值）:

### ```min_threshold = 75```

触发阈值，设置得越低，鼓越灵敏。使用5V作为参考，取值为0-1024。但是如果太低，低于传感器本身的原始噪音，那就会产生虚空输入。

### ```cd_length = 20```
缓存区循环读取```analogValue```的循环次数。设置的越低，敲击延迟越低，但是过低的缓存区可能会导致缓存区无法读取到真正的最高值。

 ```cd_length``` 代表着四个传感器读取一遍这一周期循环的次数，所以```buffer_size```的大小为```4*cd_length```.

### ```k_increase = 0.7```
当敲击产生时，缓存中的最大值会乘上```k_increase```，实现阈值提升，避免```cd_length```/```buffer_size``` 过低时产生的抖动

### ```k_decay = 0.99```
每一次程序循环中，当前阈值都会乘上```k_decay```，直到阈值回到初始设定的阈值。


## 鸣谢
Nintendo Switch支持：
[NintendoSwitchControlLibrary](https://www.arduino.cc/reference/en/libraries/nintendoswitchcontrollibrary/) by [lefmarna](https://github.com/lefmarna).  
[lty2008one](https://github.com/lty2008one)带来的NS性能提升
算法由多个项目启发，包括：  
 [ArduinoTaikoController](https://github.com/LuiCat/ArduinoTaikoController) by [LuiCat](https://github.com/LuiCat).  
[Taiko-Input](https://github.com/sachikoxz12/Taiko-Input) by [sachikoxz12](https://github.com/sachikoxz12).
