//Dev      :  Doogle007
//Last Edit:  2026/3/12

#include "network/wifi.h"

void initWiFi(void){
  WiFi.mode(WIFI_STA);
  //WiFi采用STA模式
  WiFi.setSleep(false);
  //关闭STA模式下wifi休眠，提高响应速度
}

void loopWiFi(void){
  //没有连接，则尝试连接
  if(!isConnect()){
    connectWiFi();
    if(!isConnect()){
      //连接失败，进入冷却
      delay(5000);
    }
  }
}

void disconnectWiFi(void){
  WiFi.disconnect(true, true); // 断开连接，并清除 AP 缓存
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop(); 
  esp_wifi_deinit();
  delay(500);
}

void connectWiFi(void){
  Serial.print("尝试连接到无线网络: ");
  Serial.println(FlashData.wifiSSID);
  //串口发送文本
  WiFi.begin(FlashData.wifiSSID, FlashData.wifiPass);
  //启动无线终端工作模式
  Serial.print("连接中");
  for(unsigned char i = 0; !isConnect() && i < 30; i++)
  {
    delay(500);
    Serial.print(".");
    //等待一段时间，直到无线网络连接或者超时
  }
  /** 查询连接状态 */
  if(isConnect())
  {
    Serial.println("连接成功");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
    //连接成功，串口发送文本
  }
  else
  {
    Serial.println("未成功连接无线网络,请检查网络设置");
    //未连接成功，串口发送文本
  }
}

bool isConnect(void){
  return WiFi.status() == WL_CONNECTED;
}
