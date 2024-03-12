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
  if (pressed == nullptr) return;
  unsigned long current = millis();
  Node** lastPtr = &pressed;
  Node* select = pressed;
  bool nsTriggered = false;
  while (select != nullptr) {
    if (select->key->unlock <= current) {
      switch (select->key->mode) {
        case PC_BTN: {
          Keyboard.release((uint8_t) select->key->key);
          select->key->pressed = false;
          *lastPtr = select->next;
        }; break;
        case NS_BTN: {
          SwitchControlLibrary().releaseButton(select->key->key);
          select->key->pressed = false;
          *lastPtr = select->next;
          nsTriggered = true;
        }; break;
        case NS_HAT: {
          uint8_t result = hat_sub(current_hat, (uint8_t) (select->key->key));
          if (result == Hat::NEUTRAL) {
            SwitchControlLibrary().releaseHatButton();
          } else {
            SwitchControlLibrary().pressHatButton(result);
          }
          select->key->pressed = false;
          *lastPtr = select->next;
          nsTriggered = true;
        }; break;
      }
    }
    select = select->next;
    lastPtr = &(select->next);
  }
  if (nsTriggered) SwitchControlLibrary().sendReport();
}

bool press(int size, KeyUnion* keys) {
  for (int i = 0; i < size; i++) {
    bool pressed = press0(keys + i);
    if (pressed) return true;
  }
  return false;
}

bool press0(KeyUnion* key) {
  if (key == nullptr) return false;
  if (key->pressed == true) return false;
  switch (key->mode) {
    case PC_BTN: {
      Keyboard.press((uint8_t) (key->key));
      unsigned long time = millis();
      key->unlock = time + key->delay;
      preppend(key);
      return true;
    };
    case NS_BTN: {
      SwitchControlLibrary().pressButton(key->key);
      SwitchControlLibrary().sendReport();
      unsigned long time = millis();
      key->unlock = time + key->delay;
      preppend(key);
      return true;
    };
    case NS_HAT: {
      uint8_t result = hat_add(current_hat, (uint8_t) (key->key));
      if (result != current_hat) {
        current_hat = result;
        SwitchControlLibrary().pressHatButton(result);
        SwitchControlLibrary().sendReport();
        unsigned long time = millis();
        key->unlock = time + key->delay;
        preppend(key);
        return true;
      } else return false;
    };
  }
  return false;
}

void preppend(KeyUnion* key) {
  Node node = {key, pressed};
  pressed = &node;
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

