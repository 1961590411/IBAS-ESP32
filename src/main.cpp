/**
 * @mainpage 智能楼宇警报系统
 * @author Doogle007
 * @version alpha5 2024/12/11
 */
#include "bluetooth/bluetooth.h"
#include "button/button.h"
#include "data/flash.h"
#include "network/wifi.h"
#include "network/mqtt.h"
#include "sensor/sensor.h"
#include "alarm/alarm.h"
#include "rgb/myrgb.h"

void setup()
{
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  /** 串口初始化，设定串口波特率 */
  Serial.begin(115200);

  /** 各功能的初始化 */
  setupBluetooth();
  setup_button();
  initFlash();
  initWiFi();
  initMQTT();
  setup_sensor();
  setup_alarm();
  setup_rgb();

  /** 就绪问候语 */
  Serial.println("The system is ready.");

  if(strcmp(FlashData.wifiSSID,"wifi-ssid") == 0){
    Serial.println("初次启动，请打开蓝牙完成配置");
    setBTMode(true);
  }
}

void loop()
{
  /** 检测是否开启蓝牙调试模式 */
  if(getBTMode())
  {
    /** 蓝牙调试模式循环，自动打开蓝牙调试功能 */
    loopBluetooth();
  }
  else
  {
    /** 网络连接，上传数据到云平台或等待回调 */
    loopWiFi();
    loopMQTT();

    /** 按钮检测，监视蓝牙，手动警报，复位等三个开关 */
    loop_button();

    /** 传感器检测，判断是否开启自动警报 */
    loop_sensor();

    /** 警报检测，警报激活时启动蜂鸣器 */
    loop_alarm();
  }

  delay(100);
}