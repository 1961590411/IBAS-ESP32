//Dev      :  Doogle007
//Last Edit:  2026/3/13

#include "data\flash.h"

FlashDataStruct FlashData;

void initFlash(void){
  Serial.println("Flash初始化");
  EEPROM.begin(EEPROM_SIZE);
  loadFlash();
  if(strcmp(FlashData.signature, CONFIG_SIGNATURE) != 0){
    Serial.println("检测到数据损坏，将载入默认配置...");
    resetFlash();
  }
  Serial.println("初始化完成");
}

void saveFlash(void){
  EEPROM.put(0, FlashData);
  //保存
  EEPROM.commit();
}

void loadFlash(void){
  EEPROM.get(0, FlashData);
}

void resetFlash(void){
  // 设置默认变量值
  strncpy(FlashData.signature, CONFIG_SIGNATURE, 8);
  strncpy(FlashData.mqttName, "devide-id", 32);
  strncpy(FlashData.wifiSSID, "wifi-ssid", 32);
  strncpy(FlashData.wifiPass, "wifi-password", 32);
  FlashData.hasGroup = false;
  strncpy(FlashData.mqttName, "null", 32);
  strncpy(FlashData.logicalAddress, "local-address", 64);
  //记得保存
  saveFlash();
  Serial.println("Flash重置完成");
}