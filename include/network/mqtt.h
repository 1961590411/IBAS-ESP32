//Dev      :  Doogle007
//Last Edit:  2026/3/12

#ifndef _MQTT_H
#define _MQTT_H

#include "Arduino.h"
#include "alarm/alarm.h"
#include "data/flash.h"
#include "network/wifi.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/**
 * @brief  MQTT 通讯模块初始化
 * @details 
 * 1. 配置 MQTT 代理服务器（Broker）地址与端口。
 * 2. 注册消息回调函数 (setCallback)，处理来自服务器的下行指令。
 * 3. 准备初始的连接参数（如 ClientID, Username）。
 */
void initMQTT(void);

/**
 * @brief  MQTT 任务轮询与心跳维护
 * @details 
 * 1. 检查网络状态，若断开则触发自动重连机制。
 * 2. 处理订阅主题的传入消息流。
 * 3. 根据业务逻辑发布传感器数据。
 */
void loopMQTT(void);

/**
 * @brief  触发数据立即同步（刷新消息冷却计时器）
 * @details 
 * 1. 该函数通过重置内部的时间戳或计数器，解除发送频率限制。
 * 2. 强制系统在 loopMQTT 的下一次迭代中封装并发布最新的传感器 JSON 数据。
 */
void dataRefresh(void);

/**
 * @brief  触发挥手立即同步（刷新消息冷却计时器）
 * @details 
 * 1. 该函数通过重置内部的时间戳或计数器，解除发送频率限制。
 * 2. 强制系统在 loopMQTT 的下一次迭代中封装并发布最新的传感器 JSON 数据。
 */
void helloRefresh(void);

#endif