/*******************************************************************************

                     Mqtt-连接用户信息模块
此模块应用无关
*******************************************************************************/
#ifndef _MQTT_CON_USER_H
#define _MQTT_CON_USER_H


/*******************************************************************************
                             默认协议支持时
*******************************************************************************/
//MqttConUserInfo中存储的信息定义为:
//Cfg      = 0x0000     0x0000表示直接MQTT直接传输
//UserName = MQTT认证时用户名 空是无用户认证
//UserPass = MQTT认证时密码 空是无需密码
//Info =     MQTTClient，空时缺省

/*******************************************************************************
                          阿里云MQTT协议支持时
*******************************************************************************/
//设备三元组,如:
//productKey = a14Xib5kdYd
//deviceName = light1983432
//deviceSecret = oLyaKqVxtRvjH284LdhqVgVUx1UPy6zq

//建立MQTT连接时参数
//clientId = SN1928339
//timestamp = 1539421321846
//signmethod = hmacsha1
//生成password的content
//content=clientIdSN1928339deviceNamelight1983432productKeya14Xib5kdYdtimestamp1539421321846

//生成的MQTT信息:
//mqttUsername = light1983432&a14Xib5kdYd
//mqttPassword = b2488041f64f425016b467ee1c94959ebd592ad1
//mqttClientId = SN1928339|securemode=3,signmethod=hmacsha1,timestamp=1539421321846|

//MqttConUserInfo中存储的信息定义为:
//Cfg      = 0x2000 | (securemode << 4) | signmethod  (0x2000表示阿里协议)
//UserName = mqttUsername(含deviceName&productKey)
//UserPass = mqttPassword(生成的键值)
//Info =     timestamp&deviceSecret

//注：主题有两个(MQTT服务器端用影子设备固化设备信息)：
//   Topic/shadow/update/productKey/deviceName  用于app与dev发送消息到此Topic，
//                     物联网平台收到消息后，会将消息中的状态更新到影子设备中
//   Topic/shadow/get/productKey/deviceName, 设备影子(服务器)更新状态到此Topic，
//                     设备订阅此Topic的消息后，就会收到此Topic发过来的消息。
//   阿里云仅支持到Qos1且持久化Session时才仅保留3天

/*******************************************************************************
                            相关定义
*******************************************************************************/

//相关长度，用户使用时,需<其-1
#define MQTT_CON_USER_NAME_LEN  48   //用户名长度
#define MQTT_CON_USER_PASS_LEN  48  //用户密码长度, 阿里用MD5码固定40个
#define MQTT_CON_USER_INFO_LEN  96   //相关信息，透传时为客户端,否则为盐值

/*******************************************************************************
                            相关结构
*******************************************************************************/
#include "MqttConUser.h"

struct _MqttConUserInfo{
  unsigned short Cfg;                              //相关配置，见定义
  char UserName[MQTT_CON_USER_NAME_LEN];            //储存的用户名
  char UserPass[MQTT_CON_USER_PASS_LEN];            //储存的密码
  char Info[MQTT_CON_USER_INFO_LEN];                //相关信息
};

//相关配置定义为:
#define MQTT_CON_USER_CFG_PROTOCOL_MASK    0xF000   //协议类型
#define MQTT_CON_USER_CFG_OTHR_MASK    0x0FFF   //各协议类型对子的其它信息

//注:为节省RAM空间,保存在Eeprom中的为真实信息，MqttConUser存储的信息将替换为:
//UserName变为mqttUsername
//UserPass变为mqttPassword
//Info变为mqttClientId

//用户相关信息
struct _MqttConUser{
  struct _MqttConUserInfo Info;
  unsigned char AryId;  //存放此多例阵列的ID号
};

/*******************************************************************************
                          相关函数
*******************************************************************************/

//-----------------------------初始化函数---------------------------------------
void MqttConUser_Init(struct _MqttConUser *pMqttConUser,
                      unsigned char AryId,  //存放此多例阵列的ID号
                      signed char Inited);

//-------------------------------得到配置位-------------------------------------
#define MqttConUser_GetCfg(mqttConUser) ((mqttConUser)->Info.Cfg)

//-------------------------------设置配置位-------------------------------------
void MqttConUser_SetCfg(struct _MqttConUser *pMqttConUser,
                        unsigned short Cfg);

//-----------------------------得到协议类型-------------------------------------
#define  MqttConUser_GetProtocol(mqttConUser) \
    (MqttConUser_GetCfg(mqttConUser) & MQTT_CON_USER_CFG_PROTOCOL_MASK)

//----------------------------得到相关Info信息----------------------------------
//此为GUI接口，TYPE定义为: 
#define MQTT_CON_USER_NAME    0   //用户名
#define MQTT_CON_USER_PASS    1   //用户密码
#define MQTT_CON_USER_INFO    2   //相关信息
void MqttConUser_GetInfo(const struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf);

//----------------------------设备相关Info信息----------------------------------
//此为GUI接口，TYPE定义同MqttConUser_GetInfo: 
void MqttConUser_SetInfo(struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf);

/*******************************************************************************
                          相关回调函数
*******************************************************************************/

//-----------------------根据通讯协议到Mqtt连接信息处理-------------------------
//Info里是读出的密码，加密后放回原处
void MqttConUser_cbToMqttConInfo(struct _MqttConUser *pMqttConUser);



#endif //_MQTT_CON_USER_H

