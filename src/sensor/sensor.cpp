#include "sensor/sensor.h"

#define Sensor_AO A0//36引脚
#define Sensor_DO 39

unsigned int sensorValue = 0;

void setup_sensor(void)
{
  pinMode(Sensor_DO, INPUT);
}

void loop_sensor(void)
{
  sensorValue = analogRead(Sensor_AO);
  if(digitalRead(Sensor_DO) == LOW && !get_alarm_auto())
  {
    Serial.println("传感器激活，即将启动自动警报。");
    set_alarm_auto(true);
  }
}