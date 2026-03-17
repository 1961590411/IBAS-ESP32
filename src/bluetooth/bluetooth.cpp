//Dev      :  Doogle007
//Last Edit:  2026/3/13

#include "bluetooth/bluetooth.h"

bool BTMode = false;
//蓝牙调试模式，默认为关闭状态
const uint8_t PinLED = 2;
//默认的pin口值，调用来自setup(uint_8)的参数
const uint8_t DataSize = 128;
//缓存字符串长度
BluetoothSerial SerialBT;
//BluetoothSerial的实例化

/**
 * 比较两节字符串的内容是否相同。
 * @param str1 : 常量字符串1
 * @param str2 : 常量字符串2
 * @return 字符串内容是否相等
 */
bool equalString(const char*, const char*);

/**
 * 从蓝牙串口截取字符串，保存在缓存字符串中。
 * @param buffer  : 缓存字符串
 * @param size    : 截取字符串最大尺寸
 * @param length  : 截取字符串长度
 */
bool readNextString(char*, uint8_t, uint8_t*);

void setupBluetooth() {
  pinMode(PinLED, OUTPUT);
  //设置LED引脚为输出模式，用于反映蓝牙状态:熄灭时未连接,闪烁时等待连接,点亮时已连接
}

void loopBluetooth() {
  if(!SerialBT.available()) {
    delay(20);
    return;
    //检测蓝牙串口非活跃状态, 等待短暂时间后返回
  }

  uint8_t length;
  uint8_t numCommand = 0;
  char word[DataSize];
  //初始化变量: 字段长度, 命令编号, 缓存字符串
  
  readNextString(word, 16, &length);
  if(equalString(word, "help"))
    numCommand = 1;
  else if(equalString(word, "wifi"))
    numCommand = 2;
  else if(equalString(word, "address"))
    numCommand = 3;
  else if(equalString(word, "off"))
    numCommand = 4;
  else if(equalString(word, "reset"))
    numCommand = 5;
  //截取第一个字段，解析该字段命令

  switch(numCommand) {
    case 1:
      //Help命令
      SerialBT.println("Command list:\n\t1.help\n\t2.wifi\n\t3.address\n\t4.off\n\t5.reset\n");
      break;
      //向蓝牙串口发送帮助信息
      //命令格式: help
    case 2:
      //WiFi命令
      if(!readNextString(word, 32, &length)) {
        SerialBT.print("SSID: ");
        SerialBT.println(FlashData.wifiSSID);
        SerialBT.print("Password: ");
        SerialBT.println(FlashData.wifiPass);
        SerialBT.println();
        //读取存储在EEPROM中的WiFi信息
        //命令格式: wifi
      }
      else {
        char ssid[length + 1];
        ssid[length] = '\0';
        for(uint8_t i = 0; i < length; i++)
          ssid[i] = word[i];
        //截取WiFi的SSID

        readNextString(word, 32, &length);
        char password[length+1];
        password[length] = '\0';
        for(uint8_t i = 0; i < length; i++)
          password[i] = word[i];
        //截取WiFi的密码

        strcpy(FlashData.wifiSSID, ssid);
        strcpy(FlashData.wifiPass, password);
        saveFlash();
        //将WiFi的SSID和密码写入闪存

        SerialBT.println("New WiFi saved.");
        //写入ssid和password后，重置连接
        //命令格式: wifi [ssid] [password]
      }
      break;
    case 3:
      //地址命令
      if(!readNextString(word, 64, &length)) {
        SerialBT.print("Logical Address: ");
        SerialBT.println(FlashData.logicalAddress);
        SerialBT.println();
        break;
        //读取存储在EEPROM中的地址信息
        //命令格式: address
      }
      else {
        char address[length+1];
        address[length] = '\0';
        for(uint8_t i = 0; i < length; i++)
          address[i] = word[i];
        //截取地址信息

        strcpy(FlashData.logicalAddress, address);
        saveFlash();
        //将地址写入闪存

        SerialBT.println("New logiacal address saved.");
        //写入address后，重置连接
        //命令格式: address [logical address]
      }
      break;
    case 4:
      //OFF命令
      setBTMode(false);
      break;
      //关闭蓝牙模式
      //命令格式: off
    case 5:
      resetFlash();
      break;
    default:
      SerialBT.println("Unknown command. Put \"help\" for help.\n");
      //未知命令
  }

  while(SerialBT.read() != -1);
  delay(20);
  //清空蓝牙串口换成, 等待短暂时间
}

bool getBTMode() {
  return BTMode;
}

void setBTMode(bool mode) {
  if(!BTMode && mode)
  {
    disconnectWiFi();
    Serial.printf("Free heap before BT: %u bytes\n", ESP.getFreeHeap());
    //跳变由False到True，启动蓝牙
    rgb_mode(1,true);
    SerialBT.begin("MyESP32");
    digitalWrite(PinLED,1);
    Serial.println("BT mode is on.");
    //点亮RGB和LED，启动蓝牙广播，并通知串口
  }
  else if(BTMode && !mode)
  {
    //跳变由True到False，关闭蓝牙
    rgb_mode(1,false);
    SerialBT.end();
    digitalWrite(PinLED,0);
    Serial.println("BT mode is off.");
    //熄灭RGB和LED，关闭蓝牙连接，并通知串口
    initWiFi();
  }
  BTMode = mode;
}

bool equalString(const char *str1, const char *str2) {
  for(uint8_t i = 0; str1[i] != '\0' || str2[i] != '\0'; i++)
    if(str1[i] != str2[i])
      return false;
  //若比对字符串中同一下标的两个字符不相等, 说明字符串不相等
  return true;
}

bool readNextString(char* buffer, const uint8_t size,uint8_t* length) {
  *length = 0;
  char c = SerialBT.read();
  while(c != 0xFF && c != ' ') {
    buffer[*length] = c;
    *length += 1;
    if(*length >= size)
      break;
    c = SerialBT.read();
  }//反复读取字符并存入缓冲字符串, 同时length, 直到读取到\0或空格, 或者length值等于size值

  if(*length != DataSize)
    buffer[*length] = '\0';
  return *length != 0;
  //字符串末尾补\0后, 返回值取决于length值是否为0, 0值表示截取空字符串
}