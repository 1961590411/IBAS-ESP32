//Dev      :  Doogle007
//Last Edit:  2026/3/12

#include "alarm/alarm.h"

#define Alarm_Pin 0

bool alarmAuto    = false;
bool alarmManual  = false;
bool alarmAutoLast = false;
bool alarmManualLast = false;


void setup_alarm()
{
  pinMode(Alarm_Pin, OUTPUT);//设置引脚为输出模式
}

void loop_alarm()
{
  if((alarmAuto && !alarmAutoLast) || (alarmManual && !alarmManualLast)){
    refresh();
  }
  //警报激活，蜂鸣器启动
  if(alarmAuto || alarmManual)
  {
    for(unsigned short i = 0; i < 200; i++)
    {
      digitalWrite(Alarm_Pin,true);
      //引脚输出电平翻转
      delayMicroseconds(250);
      //延时250us
      digitalWrite(Alarm_Pin,false);
      //引脚输出电平翻转
      delayMicroseconds(250);
      //延时250us
    }
  }

  alarmAutoLast = alarmAuto;
  alarmManualLast = alarmManual;
}

void set_alarm_auto(bool state)
{
  if(!alarmAuto && state)
  {
    rgb_mode(2,true);
    alarmAuto = state;
  }
  else if(alarmAuto && !state)
  {
    rgb_mode(2,false || alarmManual);
    alarmAuto = state;
  }
}

void set_alarm_manual(bool state)
{
  if(!alarmManual && state)
  {
    rgb_mode(2,true);
    alarmManual = state;
  }
  else if(alarmManual && !state)
  {
    rgb_mode(2,false || alarmAuto);
    alarmManual = state;
  }
}

bool get_alarm_auto()
{
  return alarmAuto;
}

bool get_alarm_manual()
{
  return alarmManual;
}