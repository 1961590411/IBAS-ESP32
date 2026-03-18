//Dev      :  Doogle007
//Last Edit:  2026/3/19

#include "network/mqtt.h"

void mqttCallback(char*,byte*,unsigned int);

static char mqtt_topic_data_pub_buf[128] = {0};
static char mqtt_topic_control_sub_buf[128] = {0};

/** MQTT相关配置信息 */
const  char    *mqtt_broker_addr           = "120.26.133.159";                     //服务器地址
const  uint16_t mqtt_broker_port           = 1883;                                 //服务端口号

const  char    *mqtt_username              = nullptr;                              //账号
static char     mqtt_client_id[13]         = {0};                                  //客户端ID
const  char    *mqtt_password              = "IBAS";                               //密码

const  char    *mqtt_topic_hello_pub       = "IBAS/system/device/hello";           //发布主题Hello
const  char    *mqtt_topic_pre_data_pub    = "IBAS/system/device/group/";          //发布主题Data前缀
const  char    *mqtt_topic_data_pub        = mqtt_topic_data_pub_buf;              //发布主题Data

const  char    *mqtt_topic_pre_control_sub = "IBAS/system/client/control/device/"; //订阅主题Control前缀
const  char    *mqtt_topic_control_sub     = mqtt_topic_control_sub_buf;           //订阅主题Control

WiFiClient tcpClient;
PubSubClient mqttClient;
//声明一个客户端对象，用于与服务器进行连接

void initMQTT(void){
  mqtt_username = FlashData.mqttName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(mqtt_client_id, 13,"%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  snprintf(mqtt_topic_data_pub_buf, sizeof(mqtt_topic_data_pub_buf), "%s%s", mqtt_topic_pre_data_pub, FlashData.group);
  snprintf(mqtt_topic_control_sub_buf, sizeof(mqtt_topic_control_sub_buf), "%s%s", mqtt_topic_pre_control_sub, mqtt_client_id);

  mqttClient.setClient(tcpClient);
  mqttClient.setServer(mqtt_broker_addr, mqtt_broker_port);
  mqttClient.setCallback(mqttCallback);
}

void mqttCallback(char *topic, byte *payload, unsigned int length){
  boolean needSaveFlash = false;
  boolean needDataRefresh = false;
  JsonDocument jsonBuffer;
  DeserializationError err = deserializeJson(jsonBuffer, payload, length);
  if(err){
    Serial.print("JSON解析失败: ");
    Serial.println(err.c_str());
    return;
  }
  String input;
  serializeJson(jsonBuffer, input);
  Serial.print("收到订阅的json: ");
  Serial.println(input);
  if(!jsonBuffer["params"]["AlarmManual"].isNull()){
    set_alarm_auto(jsonBuffer["params"]["AlarmManual"]);
    needDataRefresh = true;
  }
  if(!jsonBuffer["params"]["AlarmAuto"].isNull()){
    set_alarm_auto(jsonBuffer["params"]["AlarmAuto"]);
    needDataRefresh = true;
  }
  if(!jsonBuffer["params"]["Group"].isNull()) {
    strncpy(FlashData.group, jsonBuffer["params"]["Group"], 31);
    FlashData.group[31] = '\0';
    FlashData.hasGroup = true;
    needSaveFlash = true;
    snprintf(mqtt_topic_data_pub_buf, sizeof(mqtt_topic_data_pub_buf), "%s%s", mqtt_topic_pre_data_pub, FlashData.group);
  }
  if(!jsonBuffer["params"]["Name"].isNull()) {
    strncpy(FlashData.mqttName, jsonBuffer["params"]["Name"], 31);
    FlashData.mqttName[31] = '\0';
    needSaveFlash = true;
  }
  if(!jsonBuffer["params"]["LogicalAddress"].isNull()) {
    strncpy(FlashData.logicalAddress, jsonBuffer["params"]["LogicalAddress"], 63);
    FlashData.logicalAddress[63] = '\0';
    needSaveFlash = true;
  }
  if(needSaveFlash){
    saveFlash();
    helloRefresh();
  }
  if(needDataRefresh)
    dataRefresh();
}

unsigned long previousConnectMillis = 0; // 毫秒时间记录
const long intervalConnectMillis = 5000; // 时间间隔
unsigned long previousHelloPublishMillis = 0; // 毫秒时间记录
const long intervalHelloPublishMillis = 3600000; // 时间间隔
unsigned long previousDataPublishMillis = 0; // 毫秒时间记录
const long intervalDataPublishMillis = 60000; // 时间间隔

void loopMQTT(void){
  /** 查询连接状态 */
  if(!isConnect())
    return;

  unsigned long currentMillis = millis(); // 读取当前时间

  // 连接MQTT服务器
  if(!mqttClient.connected()) // 如果未连接
    if(currentMillis - previousConnectMillis > intervalConnectMillis){
      previousConnectMillis = currentMillis;
      if(!mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password)){
        Serial.print("MQTT连接失败, 错误编号: ");
        Serial.println(mqttClient.state()); // 连接成功后可以订阅主题
      }
      else{
        Serial.print("MQTT连接成功!");
        controlSub();
      }
    }

  if(!mqttClient.connected())
    return;
  // 定期发送消息
  if(currentMillis - previousHelloPublishMillis >= intervalHelloPublishMillis){
    previousHelloPublishMillis = currentMillis;
    helloPub();
  }
  if(currentMillis - previousDataPublishMillis >= intervalDataPublishMillis){ // 如果和前次时间大于等于时间间隔{
    previousDataPublishMillis = currentMillis;
    dataPub();
  }
  mqttClient.loop();
}

void dataRefresh(void){
  previousDataPublishMillis = 0;
}

void helloRefresh(void){
  previousHelloPublishMillis = 0;
}

void helloPub(void){
  JsonDocument jsonBuffer;
  JsonObject root = jsonBuffer["params"].to<JsonObject>();
  root["Name"] = FlashData.mqttName;
  if(FlashData.hasGroup)
    root["Group"] = FlashData.group;
  root["LogicalAddress"] = FlashData.logicalAddress;
  String output;
  serializeJson(jsonBuffer, output);
  Serial.print("即将推送的json: ");
  Serial.println(output);
  mqttClient.publish(mqtt_topic_hello_pub, output.c_str(), true);
}

void dataPub(void){
  JsonDocument jsonBuffer;
  JsonObject root = jsonBuffer["params"].to<JsonObject>();
  root["Name"] = FlashData.mqttName;
  root["AlarmManual"] = get_alarm_manual() ? "true" : "false";
  root["AlarmAuto"] = get_alarm_auto() ? "true" : "false";
  String output;
  serializeJson(jsonBuffer, output);
  Serial.print("即将推送的json: ");
  Serial.println(output);
  mqttClient.publish(mqtt_topic_data_pub, output.c_str());
}

void controlSub(void){
  if(mqttClient.subscribe(mqtt_topic_control_sub)){
    Serial.print("成功订阅主题: ");
    Serial.println(mqtt_topic_control_sub);
    return;
  }
  Serial.println("主题订阅失败, 请检查相关配置!");
}