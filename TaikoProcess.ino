void setup() {
  Serial.begin(9600); // 初始化串口通信，波特率设置为9600
}

const int threshold = 200; // 设置初始阈值
const int cd_length = 30; //毫秒
const float k_threshold = 1.7; //抬升幅度
const float k_decay = 0.99; //衰变幅度
  
  
int min_threshold = threshold;

void loop() {
  int sensorValue[] = {analogRead(A0),analogRead(A1),analogRead(A2),analogRead(A3)};
  bool output[] = {false,false,false,false};

  // 判断A0引脚的模拟值与阈值的关系
  if (abs(sensorValue[0] - 675) > min_threshold) {
    output[0] = true; // 如果满足条件，将输出标志设为true
  }
  else if (abs(sensorValue[2] - 675) > min_threshold) {
    output[2] = true; // 如果满足条件，将输出标志设为true
  }
  else if (abs(sensorValue[1] - 675) > min_threshold) {
    output[1] = true; // 如果满足条件，将输出标志设为true
  }
  else if (abs(sensorValue[3] - 675) > min_threshold) {
    output[3] = true; // 如果满足条件，将输出标志设为true
  }

  // 如果输出标志为true，则输出全部引脚的模拟值减675的绝对值
  for (int i = 0; i <= 3; i++){
    if (output[i]){
      Serial.print("被打击的是A");
      Serial.println(i);
      if ( i==0 || i == 2){
        min_threshold = 1350;
        }
      else{
        min_threshold = abs(sensorValue[i]-675)*k_threshold;
        }
  //    Serial.print("当前更新后的阈值为");
      Serial.println(min_threshold);
      delay(cd_length);
      }
    else{
      if (min_threshold <= threshold){
        min_threshold = threshold;
//        Serial.println("阈值已经回到初始值");
        }
      else{
    //    Serial.println("开始衰减");
        min_threshold = min_threshold*k_decay;
        }
      }
    }
}
