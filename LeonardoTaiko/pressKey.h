#include <Keyboard.h>
#include <KeyboardLayout.h>
#include <NintendoSwitchControlLibrary.h>

#define PC_BTN 0
#define NS_BTN 1
#define NS_HAT 2

#define PC_BTN_DUR 7
#define NS_BTN_DUR 25
#define NS_HAT_DUR 35


struct KeyUnion {
  uint16_t key;
  uint8_t mode;
  unsigned long delay;
  unsigned long unlock;
  bool pressed;
};

struct Node {
  KeyUnion* key;
  Node* next;
};


uint8_t current_hat = Hat::NEUTRAL;
Node* pressed = nullptr;

bool press(int size, KeyUnion* keys);           // 选择并按下按键
bool press0(KeyUnion* key);                     // 执行按下按键操作
void release();                                 // 释放过期按键
void preppend(KeyUnion* key);                   // 追加按键到待过期列表
uint8_t hat_add(uint8_t cur, uint8_t add);      // 方向键相加
uint8_t hat_sub(uint8_t cur, uint8_t sub);      // 方向键相减


void release() {
  if (pressed == nullptr) return;     // 检测添加到pressed链表中的按键
  unsigned long current = millis();   // 获取一次时间作为当前时间
  Node** lastPtr = &pressed;          // 保存上个节点的指针，用于移除中间节点
  Node* select = pressed;             // 取当前节点指针，用于遍历链表
  bool nsTriggered = false;           // 判断是否触发了ns按键操作，最后统一执行 sendReport 方法
  while (select != nullptr) {
    bool modifyTriggered = false;     // 判断是否触发了修改，用于最后处理节点遍历
    if (select->key->unlock <= current) {   // 判断节点已经过了锁定时间（不用判断锁定状态，因为只有锁定才会加入链表）
      switch (select->key->mode) {          // 根据按键的模式，判断应该如何抬起按键
        case PC_BTN: {
          Keyboard.release((uint8_t) select->key->key);   // 键盘抬起按键
          select->key->pressed = false;                   // 重置按下状态
          *lastPtr = select->next;                        // 将指向本节点的指针指向下个节点
          Node* toDel = select;
          select = select->next;                          // 前进到下一节点
          modifyTriggered = true;
          free(toDel);                                    // 释放节点内存
        }; break;
        case NS_BTN: {
          SwitchControlLibrary().releaseButton(select->key->key); // NS松开按键
          select->key->pressed = false;                   // 重置按下状态
          *lastPtr = select->next;                        // 将指向本节点的指针指向下个节点
          Node* toDel = select;
          select = select->next;                          // 前进到下一节点
          modifyTriggered = true;
          nsTriggered = true;                             // 确认触发了ns操作，统一发送report
          free(toDel);                                    // 释放节点内存
        }; break;
        case NS_HAT: {
          uint8_t result = hat_sub(current_hat, (uint8_t) (select->key->key));  // NS计算方向键
          if (result == Hat::NEUTRAL) {                     // 如果没有按下向任意方向
            SwitchControlLibrary().releaseHatButton();      // 松开方向键
          } else {                                          // 否则
            SwitchControlLibrary().pressHatButton(result);  // 变成另一个方向（例如左上松掉上变成左）
          }
          select->key->pressed = false;                   // 重置按下状态
          *lastPtr = select->next;                        // 将指向本节点的指针指向下个节点
          Node* toDel = select;
          select = select->next;                          // 前进到下一节点
          modifyTriggered = true;
          nsTriggered = true;                             // 确认触发了ns操作，统一发送report
          free(toDel);                                    // 释放节点内存
        }; break;
      }
    }
    if (!modifyTriggered) {               // 如果没有发生修改
      lastPtr = &(select->next);          // 保存上个节点的指针位置转移
      select = select->next;              // 前进到下一节点
    }
  }
  if (nsTriggered) SwitchControlLibrary().sendReport();
}


bool press(int size, KeyUnion* keys) {
  for (int i = 0; i < size; i++) {        // 遍历所有的key
    bool pressed = press0(keys + i);      // 直到尝试按下这个按键成功
    if (pressed) return true;             // 为止
  }
  return false;                           // 否则按下按键失败
}


bool press0(KeyUnion* key) {
  if (key == nullptr) return false;
  if (key->pressed == true) return false;     // 已经按下的按键不能按下了
  switch (key->mode) {
    case PC_BTN: {
      Keyboard.press((uint8_t) (key->key));   // 按下当前键盘按键
      key->unlock = millis() + key->delay;    // 添加抬起时间戳
      key->pressed = true;                    // 将当前按键的状态置为按下，避免再按到它
      preppend(key);                          // 追加到抬起链表中
      return true;
    };
    case NS_BTN: {
      SwitchControlLibrary().pressButton(key->key);   // 按下当前手柄按钮
      SwitchControlLibrary().sendReport();    // 发送按下回报
      key->unlock = millis() + key->delay;    // 添加抬起时间戳
      key->pressed = true;                    // 将当前按键的状态置为按下，避免再按到它
      preppend(key);                          // 追加到抬起链表中
      return true;
    };
    case NS_HAT: {
      uint8_t result = hat_add(current_hat, (uint8_t) (key->key));  // 方向键合并运算
      if (result != current_hat) {            // 能够合并
        current_hat = result;                 // 将当前方向键状态改为合并后的状态
        SwitchControlLibrary().pressHatButton(result);  // 按下当前(组合?)手柄方向键
        SwitchControlLibrary().sendReport();  // 发送按下回报
        key->unlock = millis() + key->delay;  // 添加抬起时间戳
        key->pressed = true;                  // 将当前按键的状态置为按下，避免再按到它
        preppend(key);                        // 追加到抬起链表中
        return true;
      } else return false;
    };
  }
  return false;
}


void preppend(KeyUnion* key) {
  Node *node = (Node*)malloc(sizeof(Node));
  node -> key = key;
  node -> next = pressed;
  pressed = node;
}

// 00000000 UP
// 00000001 UP_RIGHT
// 00000010 RIGHT
// 00000011 DOWN_RIGHT
// 00000100 DOWN
// 00000101 DOWN_LEFT
// 00000110 LEFT
// 00000111 UP_LEFT
// 00001000 NEUTRAL
uint8_t hat_add(uint8_t cur, uint8_t add) {
  if (cur == 0x08) return add;   // 如果当前为全部释放状态，则按下什么按键就是什么按键
  if (cur & 0x01) return cur;   // 如果当前按键值为奇数，则必然已经是两个按键之和，返回无法按下
  if (cur ^ 0x04 == add) return cur;  // 如果两个按键差值为4，则必然互斥
  if (cur == 0x00 && add == 0x06) return 0x07;
  if (cur == 0x06 && add == 0x00) return 0x07;
  return (cur + add) >> 1;
}

uint8_t hat_sub(uint8_t cur, uint8_t sub) {
  if (cur == 0x08) return cur;   // 如果当前为全部释放状态，则无法释放按键
  if (cur == sub) return 0x08;  // 如果当前按键等于移除按键，则释放后按键为完全释放状态
  if (cur & 0x01) {
    if (cur - sub == 1 || sub - cur == 1) return (cur * 2 - sub) % 0x08;
    if (cur == 0x07 && cur == 0x00) return 0x06;
  }
  return cur;     // 其余不能减的情况不减
}

