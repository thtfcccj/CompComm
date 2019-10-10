/*******************************************************************************

                     Mqtt协议管理器
* 管理器仅负责MQTT的工作状态，相关数据及数据协议不在此处理。
* 管理器仅依赖于transport模块实现数据收发，与底层数据无关。
* 此管理器使用paho MQTT协议栈进行数据的序列化或与反序列化。

*******************************************************************************/
#ifndef MQTT_MNG_H
#define MQTT_MNG_H

/*******************************************************************************
                            相关定义
*******************************************************************************/

#define MQTT_MNG_SERIALIZE_BUF_LEN    512 //序列化字符长度，与应用相关


/*******************************************************************************
                            相关结构
*******************************************************************************/
#include "MqttPacket.h"
#include "MqttUser.h"

//工作状态
enum _MqttMngState{
  MqttMngState_Idie = 0,         //空闲状态，等待连接
  MqttMngState_Connect = 1,      //链接中,等待服务器回应
  MqttMngState_Subscribe = 2,    //发送订阅状态
  MqttMngState_Publish = 2,      //发送消息状态(获取订阅后回)
};

//序列化/反序列化需要的数据缓冲
union _MqttMngDataBuf{
  MQTTPacket_connectData Connect; //连接需要的数据
};

struct _MqttMng{
  //状态相关：
  enum msgTypes  eMsgTypes;             //消息类型
  unsigned char SubState;              //消息类型对应子状态
  unsigned char WaitTimer;            //各状态超时等待定时器，10ms为单位
  unsigned char RetryIndex;           //相关状态重试次数
  //用户交互相关：
  const struct _MqttUser *pUser;    //带入的用户信息
  //缓冲相关
  union _MqttMngDataBuf Buf;            //中转缓冲区
  unsigned char SerializeBuf[MQTT_MNG_SERIALIZE_BUF_LEN]; //序列化数据缓冲区
  signed short SerializeLen;         //序列化后的有效数据个数,负值见MQTT定义
  
  signed char Err;      //错误标志
  unsigned char Flag;  //相关标志，见定义
};

//相关标志定义为:
#define MQTT_MNG_TYPE_CONTINUE  0x80   //当前状态正常继续

extern struct _MqttMng MqttMng;  //直接单例化

/*******************************************************************************
                          相关函数
*******************************************************************************/

//----------------------------初始化函数----------------------------------------
void MqttMng_Init(void);

//-------------------------接收处理函数----------------------------------------
void MqttMng_RcvPro(unsigned char *pData,  //数据区
                    unsigned short RcvLen,   //收到的数据长度
                    unsigned short BufSize); //缓冲区大小

//-------------------------10ms任务函数----------------------------------------
void MqttMng_Task(void);


/*******************************************************************************
                              回调函数
*******************************************************************************/

//---------------------------------通讯无法与MQTT服务器通讯---------------------
void MqttMng_ErrToServerNotify(void);



#endif //MQTT_MNG_H
