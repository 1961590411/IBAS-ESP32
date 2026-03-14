//Dev      :  Doogle007
//Last Edit:  2026/3/11

#ifndef _WIFI_H
#define _WIFI_H

#include "Arduino.h"
#include "data/flash.h"
#include <esp_wifi.h>
#include <WiFi.h>

/**
 * @brief  Wi-Fi 模块初始化
 * @details 
 * 1. 设置 Wi-Fi 工作模式为 STA (Station) 模式。
 */
void initWiFi(void);

/**
 * @brief  Wi-Fi 状态监控循环
 * @details 
 * 1. 检查当前的连接状态。如果发现连接断开（WiFi.status() != WL_CONNECTED），
 * 2. 则根据业务需求触发自动重连机制。
 * @note   建议在 main loop() 中周期性调用，以维持长期的网络稳定性。
 */
void loopWiFi(void);

/**
 * @brief  主动断开当前 Wi-Fi 连接
 * @details 
 * 停止当前的无线连接。
 */
void disconnectWiFi(void);

/**
 * @brief  执行 Wi-Fi 连接动作
 * @details 
 * 从全局变量 FlashData 中读取 wifiSSID 和 wifiPass，
 * 并调用 WiFi.begin() 尝试发起握手请求。
 */
void connectWiFi(void);

/**
 * @brief  检查当前 Wi-Fi 是否已成功建立连接
 * @return true: 已获取 IP 地址并处于连接状态 | false: 处于断开、搜索中或认证失败状态。
 */
bool isConnect(void);

#endif