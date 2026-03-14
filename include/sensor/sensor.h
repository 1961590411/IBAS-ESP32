#ifndef _SENSOR_H
#define _SENSOR_H

#include "Arduino.h"
#include "alarm/alarm.h"

/**
 * 传感器功能的初始化。
 */
void setup_sensor(void);

/**
 * 传感器功能的循环。
 */
void loop_sensor(void);

#endif