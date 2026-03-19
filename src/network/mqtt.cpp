//Dev      :  Doogle007
//Last Edit:  2026/3/19

#include "network/mqtt.h"

void mqttCallback(char*,byte*,unsigned int);
void helloPub(void);
void dataPub(void);
void controlSub(void);

static char mqtt_topic_data_pub_buf[128] = {0};
static char mqtt_topic_hello_pub_buf[128] = {0};
static char mqtt_topic_control_sub_buf[128] = {0};

/** MQTT相关配置信息 */
const  char    *mqtt_broker_addr           = "120.26.133.159";                     //服务器地址
const  uint16_t mqtt_broker_port           = 1883;                                 //服务端口号

const  char    *mqtt_username              = nullptr;                              //账号
static char     mqtt_client_id[13]         = {0};                                  //客户端ID
const  char    *mqtt_password              = "IBAS";                               //密码

const  char    *mqtt_topic_pre_hello_pub   = "IBAS/system/device/hello/";          //发布主题Hello
const  char    *mqtt_topic_hello_pub       = mqtt_topic_hello_pub_buf;             //发布主题Data

const  char    *mqtt_topic_pre_data_pub    = "IBAS/system/device/group/";          //发布主题Data前缀
const  char    *mqtt_topic_data_pub        = mqtt_topic_data_pub_buf;              //发布主题Data

const  char    *mqtt_topic_pre_control_sub = "IBAS/system/client/control/device/"; //订阅主题Control前缀
const  char    *mqtt_topic_control_sub     = mqtt_topic_control_sub_buf;           //订阅主题Control

WiFiClient tcpClient;
PubSubClient mqttClient;
//声明一个客户端对象，用于与服务器进行连接

unsigned long previousConnectMillis       = 0;        // 毫秒时间记录
const long    intervalConnectMillis       = 5000;     // 时间间隔
unsigned long previousHelloPublishMillis  = 0;        // 毫秒时间记录
const long    intervalHelloPublishMillis  = 3600000;  // 时间间隔
unsigned long previousDataPublishMillis   = 0;        // 毫秒时间记录
const long    intervalDataPublishMillis   = 60000;    // 时间间隔

void initMQTT(void){
  mqtt_username = FlashData.mqttName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(mqtt_client_id, 13,"%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  snprintf(mqtt_topic_hello_pub_buf, sizeof(mqtt_topic_hello_pub_buf), "%s%s", mqtt_topic_pre_hello_pub, mqtt_client_id);
  snprintf(mqtt_topic_data_pub_buf, sizeof(mqtt_topic_data_pub_buf), "%s%s", mqtt_topic_pre_data_pub, FlashData.group);
  snprintf(mqtt_topic_control_sub_buf, sizeof(mqtt_topic_control_sub_buf), "%s%s", mqtt_topic_pre_control_sub, mqtt_client_id);

  mqttClient.setClient(tcpClient);
  mqttClient.setServer(mqtt_broker_addr, mqtt_broker_port);
  mqttClient.setCallback(mqttCallback);
}

void mqttCallback(char *topic, byte *payload, unsigned int length){
  boolean needSaveFlash   = false;
  boolean needDataRefresh = false;
  boolean needDisconnect  = false;
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
    set_alarm_manual(jsonBuffer["params"]["AlarmManual"].as<bool>());
    needDataRefresh = true;
  }
  if(!jsonBuffer["params"]["AlarmAuto"].isNull()){
    set_alarm_auto(jsonBuffer["params"]["AlarmAuto"].as<bool>());
    needDataRefresh = true;
  }
  if(!jsonBuffer["params"]["Group"].isNull()) {
    const char* newGroup = jsonBuffer["params"]["Group"].as<const char*>();
    if (newGroup && strncmp(FlashData.group, newGroup, sizeof(FlashData.group)) != 0) {
      strncpy(FlashData.group, newGroup, sizeof(FlashData.group) - 1);
      FlashData.group[sizeof(FlashData.group) - 1] = '\0';
      FlashData.hasGroup = (FlashData.group[0] != '\0');
      needSaveFlash = true;
      snprintf(mqtt_topic_data_pub_buf, sizeof(mqtt_topic_data_pub_buf), "%s%s", mqtt_topic_pre_data_pub, FlashData.group);
    }
  }
  if(!jsonBuffer["params"]["Name"].isNull()) {
    const char* newName = jsonBuffer["params"]["Name"].as<const char*>();
    if (newName && strncmp(FlashData.mqttName, newName, sizeof(FlashData.mqttName)) != 0) {
      strncpy(FlashData.mqttName, newName, sizeof(FlashData.mqttName) - 1);
      FlashData.mqttName[sizeof(FlashData.mqttName) - 1] = '\0';
      mqtt_username = FlashData.mqttName;
      needSaveFlash = true;
      needDisconnect = true;
    }
  }
  if(!jsonBuffer["params"]["LogicalAddress"].isNull()) {
    const char* newLogicalAddress = jsonBuffer["params"]["LogicalAddress"].as<const char*>();
    if (newLogicalAddress && strncmp(FlashData.logicalAddress, newLogicalAddress, sizeof(FlashData.logicalAddress)) != 0) {
      strncpy(FlashData.logicalAddress, newLogicalAddress, sizeof(FlashData.logicalAddress) - 1);
      FlashData.logicalAddress[sizeof(FlashData.logicalAddress) - 1] = '\0';
      needSaveFlash = true;
    }
  }
  if(needSaveFlash){
    saveFlash();
    helloRefresh();
  }
  if(needDataRefresh)
    dataRefresh();
  if(needDisconnect){
    previousConnectMillis = 0;
    dataRefresh();
    helloRefresh();
    mqttClient.disconnect();
    return;
  }
}

void loopMQTT(void){
  /** 查询连接状态 */
  if(!isConnect())
    return;

  unsigned long currentMillis = millis(); // 读取当前时间

  // 连接MQTT服务器
  if(!mqttClient.connected()){ // 如果未连接
    if(currentMillis - previousConnectMillis > intervalConnectMillis){
      previousConnectMillis = currentMillis;
      if(!mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password)){ // 连接成功后可以订阅主题
        Serial.print("MQTT连接失败, 错误编号: ");
        Serial.println(mqttClient.state());
      }
      else{
        Serial.println("MQTT连接成功!");
        helloPub();
        dataPub();
        controlSub();
      }
    }
    return;
  }

  // 定期发送消息
  if(currentMillis - previousHelloPublishMillis >= intervalHelloPublishMillis){
    previousHelloPublishMillis = currentMillis;
    Serial.print("即将推送主题: ");
    Serial.println(mqtt_topic_hello_pub);
    helloPub();
  }
  if(currentMillis - previousDataPublishMillis >= intervalDataPublishMillis){ // 如果和前次时间大于等于时间间隔{
    previousDataPublishMillis = currentMillis;
    Serial.print("即将推送主题: ");
    Serial.println(mqtt_topic_data_pub);
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
  if(!FlashData.hasGroup){
    Serial.println("设备未分组，本次发布跳过！");
    return;
  }
  JsonDocument jsonBuffer;
  JsonObject root = jsonBuffer["params"].to<JsonObject>();
  root["Name"] = FlashData.mqttName;
  root["AlarmManual"] = get_alarm_manual();
  root["AlarmAuto"]   = get_alarm_auto();
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