//Dev      :  Doogle007
//Last Edit:  2026/3/12

#include "network/mqtt.h"

static char mqtt_client_id_buf[13] = {0};
static char mqtt_topic_pub_buf[128] = {0};

/** MQTT相关配置信息 */
const char    *mqtt_broker_addr = "120.26.133.159";                                       //服务器地址
const uint16_t mqtt_broker_port = 1883;                                                   //服务端口号
const char    *mqtt_username;                                              //账号
const char    *mqtt_password    = "IBAS";                                                 //密码
const char    *mqtt_client_id   = "MyESP32";                                              //客户端ID

const char    *mqtt_topic_hello_pub   = "IBAS/system/device/hello";    
const char    *mqtt_topic_pre_data_pub = "/sys/k1xumLrUjyl/MyESP32/thing/event/property/post/"; // 发布主题前缀(示例)
const char    *mqtt_topic_pub   = mqtt_topic_pub_buf;                                     //需要发布到的主题(运行时拼接)

const char    *mqtt_topic_data_sub   = "/sys/k1xumLrUjyl/MyESP32/thing/service/property/set";  //需要订阅的主题


WiFiClient tcpClient;
PubSubClient mqttClient;
//声明一个客户端对象，用于与服务器进行连接

void mqttCallback(char *topic, byte *payload, unsigned int length){
  JsonDocument jsonBuffer;
  deserializeJson(jsonBuffer, payload);
  String input;
  serializeJson(jsonBuffer, input);
  Serial.print("收到订阅的json: ");
  Serial.println(input);
  if(!jsonBuffer["params"]["AlarmSwitchAuto"].isNull())
    set_alarm_auto(jsonBuffer["params"]["AlarmSwitchAuto"]);
  if(!jsonBuffer["params"]["AlarmSwitchManual"].isNull())
    set_alarm_manual(jsonBuffer["params"]["AlarmSwitchManual"]);
}

void initMQTT(void){
  snprintf(mqtt_topic_pub_buf, sizeof(mqtt_topic_pub_buf), "%s%s", mqtt_topic_pre_data_pub, mqtt_client_id);
  mqtt_username = FlashData.mqttName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(mqtt_client_id_buf, sizeof(mqtt_client_id_buf),
           "%02X%02X%02X%02X%02X%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  mqttClient.setClient(tcpClient);
  mqttClient.setServer(mqtt_broker_addr, mqtt_broker_port);
  mqttClient.setCallback(mqttCallback);
}

unsigned long previousConnectMillis = 0; // 毫秒时间记录
const long intervalConnectMillis = 20000; // 时间间隔
unsigned long previousHelloPublishMillis = 0; // 毫秒时间记录
const long intervalHelloPublishMillis = 3600000; // 时间间隔
unsigned long previousDataPublishMillis = 0; // 毫秒时间记录
const long intervalDataPublishMillis = 20000; // 时间间隔

void loopMQTT(void){
  /** 查询连接状态 */
  if(!isConnect())
    return;
  unsigned long currentMillis = millis(); // 读取当前时间

  // 连接MQTT服务器
  if(!mqttClient.connected()) // 如果未连接
  {
    if(currentMillis - previousConnectMillis > intervalConnectMillis){
      previousConnectMillis = currentMillis;
      if(mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password)){
        mqttClient.subscribe(mqtt_topic_data_sub); // 连接成功后可以订阅主题
      }
      else{
        unsigned char i = mqttClient.state();
        Serial.print("MQTT连接失败, 错误编号: ");
        Serial.println(i);
      }
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

void refresh(void){
  previousDataPublishMillis = 0;
}

void helloPub(void){
  JsonDocument jsonBuffer;
  JsonObject root = jsonBuffer["params"].to<JsonObject>();
  root["DeviceID"] = FlashData.mqttName;
  if(FlashData.hasGroup)
    root["Group"] = FlashData.group;
  root["LogicalAddress"] = FlashData.logicalAddress;
  String output;
  serializeJson(jsonBuffer, output);
  Serial.print("即将推送的json: ");
  Serial.println(output);
  mqttClient.publish(mqtt_topic_hello_pub, output.c_str());
}

void dataPub(void){
  JsonDocument jsonBuffer;
  JsonObject root = jsonBuffer["params"].to<JsonObject>();
  root["DeviceID"] = FlashData.mqttName;
  root["LogicalAddress"] = FlashData.logicalAddress;
  root["AlarmSwitchManual"] = get_alarm_manual() ? "true" : "false";
  root["AlarmSwitchAuto"] = get_alarm_auto() ? "true" : "false";
  String output;
  serializeJson(jsonBuffer, output);
  Serial.print("即将推送的json: ");
  Serial.println(output);
  mqttClient.publish(mqtt_topic_pub, output.c_str());
}