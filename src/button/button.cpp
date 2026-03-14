#include "button/button.h"

const uint8_t PinButton1 = 14;
const uint8_t PinButton2 = 27;
const uint8_t PinButton3 = 26;

void setup_button()
{
  pinMode(PinButton1, INPUT_PULLUP);//蓝牙按钮
  pinMode(PinButton2, INPUT_PULLUP);//警报按钮
  pinMode(PinButton3, INPUT_PULLUP);//复位按钮
}

void loop_button(void)
{
  unsigned char k = check_button();

  if(k == 1)
  {
    delay(10);
    //消抖
    Serial.println("按键激活，即将打开蓝牙调试模式。");
    setBTMode(true);
    //打开蓝牙调试模式
  }
  else if(k == 2)
  {
    delay(10);
    Serial.println("按键激活，即将启动手动警报。");
    set_alarm_manual(true);
  }
  else if(k == 3)
  {
    delay(10);
    Serial.println("按键激活，即将把警报复位。");
    set_alarm_auto(false);
    set_alarm_manual(false);
  }
}

unsigned char check_button(void)
{
  bool k1,k2,k3;
  k1 = !digitalRead(PinButton1);
  k2 = !digitalRead(PinButton2);
  k3 = !digitalRead(PinButton3);

  if(k1 || k2 || k3)
  {
    if((k1 || k2) && (k1 || k3) && (k2 || k3))
    {
      Serial.println("不要同时按下多个按钮！");
      return 0;
    }
    else if(k1)
      return 1;
    else if(k2)
      return 2;
    else if(k3)
      return 3;
  }
  return 0;
}