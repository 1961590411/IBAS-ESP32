#ifndef _BUTTON_H
#define _BUTTON_H

#include "Arduino.h"
#include "alarm/alarm.h"
#include "bluetooth/bluetooth.h"

/**
 * 按钮功能的初始化。
 */
void setup_button(void);

/**
 * 按钮功能的循环。
 */
void loop_button(void);

/**
 * 检查按下的按钮编号，若没有按下任何按钮或按钮冲突则为0。
 */
unsigned char check_button(void);

#endif