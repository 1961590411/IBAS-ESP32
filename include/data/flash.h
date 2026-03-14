//Dev      :  Doogle007
//Last Edit:  2026/3/13

#ifndef _FLASH_H
#define _FLASH_H

#include "Arduino.h"
#include <EEPROM.h>

#define EEPROM_SIZE 256         //数值必须是4的倍数
#define CONFIG_SIGNATURE "IBAS" //数据签名，用于校验数据是否丢失

/**
 * @struct FlashDataStruct
 * @brief  系统持久化配置结构体
 * @details 
 * 用于在 ESP32 的 Flash 中存储关键运行参数。
 * 通过 signature 字段校验数据合法性，确保系统升级或首次启动时能正确初始化。
 */
struct FlashDataStruct
{
    /** @brief 数据合法性签名。固定为 "IBAS\0"，用于识别 Flash 数据是否已由本程序初始化。 */
    char signature[8];

    /** @brief MQTT 连接身份用户名。用于 EMQX 等服务器的安全认证（Authentication）。 */
    char mqttName[32];

    /** @brief 目标无线网络名称 (SSID)。设备尝试连接的 Wi-Fi 热点名。 */
    char wifiSSID[32];

    /** @brief 无线网络访问密码。对应的 Wi-Fi 预共享密钥。 */
    char wifiPass[32];

    /** @brief 设备的逻辑识别地址。
     * 例如：设备在建筑中的具体位置描述（如 "BuildingA-Floor3-Room302"）或云端唯一映射地址。 
     */
    char logicalAddress[64];
};

/** @brief 全局配置实例。程序运行时通过此变量访问或修改当前系统的运行参数。 */
extern FlashDataStruct FlashData;

/**
 * @brief  Flash 存储功能的初始化
 * @details 
 * 初始化底层的 EEPROM/Flash 模拟库（分配内存空间）。
 * 自动调用 loadFlash() 尝试加载历史配置。
 * 校验签名：若 Flash 中无有效签名，则执行重置逻辑并写入初始默认值。
 */
void initFlash(void);

/**
 * @brief  将内存中的配置持久化到 Flash
 * @details 
 * 将全局变量 FlashData 中的当前内容完整写入 EEPROM 缓冲区。
 * 执行 commit() 操作，将数据从内存缓冲区真正刷入闪存（Flash）。
 * @note   注意：ESP32 的 Flash 擦写寿命有限，请避免在 loop() 中高频调用此函数。
 */
void saveFlash(void);

/**
 * @brief  从 Flash 中恢复配置数据到内存
 * @details 
 * 从 EEPROM 地址偏移 0 处读取 FlashDataStruct 长度的数据块，并填充至 FlashData 全局变量中。
 * 通常在系统启动阶段由 initFlash() 内部调用。
 */
void loadFlash(void);

/**
 * @brief  恢复出厂设置
 * @details 清空现有配置并写入默认值，通常在 initFlash 校验失败或用户手动触发时调用。
 */
void resetFlash(void);

#endif