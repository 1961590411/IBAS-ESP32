#ifndef _AlARM_H
#define _ALARM_H

#include "Arduino.h"
#include "rgb/myrgb.h"
#include "network/mqtt.h"

/**
 * 警报功能的初始化。
 * @param pin : 默认设置pin0为输出模式，用于激活蜂鸣器
 */
void setup_alarm();

/**
 * 警报功能的循环。
 */
void loop_alarm();

/**
 * 设置自动警报状态。
 * @param state : 传递警报状态
 */
void set_alarm_auto(bool);

/**
 * 设置手动警报状态。
 * @param state : 传递警报状态
 */
void set_alarm_manual(bool);

/**
 * 设置自动警报状态。
 * @return 返回警报状态
 */
bool get_alarm_auto(void);

/**
 * 设置手动警报状态。
 * @return 返回警报状态
 */
bool get_alarm_manual(void);

#endif