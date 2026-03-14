//Dev      :  Doogle007
//Last Edit:  2026/3/10

#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "Arduino.h"
#include "rgb/myrgb.h"
#include "data/flash.h"
#include "network/wifi.h"
#include <BluetoothSerial.h>

/**
 * 蓝牙调试模式的初始化。
 */
void setupBluetooth(void);

/**
 * 蓝牙调试模式的循环。
 * @note 应当在蓝牙调试模式启动时执行。
 */
void loopBluetooth(void);

/**
 * 获取是否打开蓝牙调试模式。
 * @return 是否打开蓝牙调试模式
 */
bool getBTMode(void);

/**
 * 设置蓝牙调试模式状态。
 * @param mode : 是否打开蓝牙调试模式
 */
void setBTMode(bool mode);

#endif