/*******************************************************************************

                     Mqtt协议管理器-作为客户端时
* 仅支持MQTT数据包的串行处理，不支持并行处理
* 管理器仅负责MQTT的工作状态，相关数据及数据协议不在此处理。
* 管理器仅依赖于transport模块实现数据收发，与底层数据无关。
* 此管理器使用paho MQTT协议栈进行数据的序列化或与反序列化。

*******************************************************************************/
#ifndef MQTT_MNG_H
#define MQTT_MNG_H

/*******************************************************************************
                            相关定义
*******************************************************************************/

#ifndef MQTT_MNG_SERIALIZE_BUF_LEN  //序列化字符长度，与应用相关
  #define MQTT_MNG_SERIALIZE_BUF_LEN    300    
#endif

#ifndef MQTT_MNG_USER_PAYLOAD_LEN  //用户荷载长度，与应用相关
  #define MQTT_MNG_USER_PAYLOAD_LEN     128   
#endif

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

//订阅模式应答包缓冲
struct _MqttMngSubscribeAck{
  unsigned short PacketId;
};

//发布模式应答包缓冲
struct _MqttMngPublishAck{
  unsigned char  PacketType; 
  unsigned char  Dup;
  unsigned short PacketId;
};

//序列化/反序列化需要的数据缓冲
union _MqttMngDataBuf{
  MQTTPacket_connectData Connect;          //连接需要的数据
  struct _MqttMngSubscribeAck SubscribeAck; //订阅模式应答包缓冲
  struct _MqttUserPublish RdPublish;        //发布时读数据临时缓冲
  struct _MqttMngPublishAck PublishAck;  //发布模式应答包缓冲
  //发布时读数据的用户荷载缓冲
  unsigned char WrPublishPayloadBuf[MQTT_MNG_USER_PAYLOAD_LEN];   
};

struct _MqttMng{
  //状态相关：
  enum msgTypes  eMsgTypes;             //消息类型
  unsigned char SubState;              //消息类型对应子状态
  unsigned char WaitTimer;            //各状态超时等待定时器，10ms为单位
  unsigned char RetryIndex;           //相关状态重试次数
  //用户交互相关：
  const struct _MqttUser *pUser;    //带入的用户信息
  void *pUserHandle;                 //用户信息需要的句柄
  //缓冲相关
  union _MqttMngDataBuf Buf;            //中转缓冲区
  unsigned char SerializeBuf[MQTT_MNG_SERIALIZE_BUF_LEN]; //序列化数据缓冲区
  struct _MqttUserPublish WrPublishBuf; //发布时写数据缓冲
  
  signed short SerializeLen;            //序列化后的有效数据个数,负值见MQTT定义
  unsigned short PacketIdIndex;         //包计数器
  unsigned short CuPacketId;           //当前正在操作包的ID号
  signed char Err;      //错误标志
  unsigned char Flag;  //相关标志，见定义
};

//相关标志定义为:
#define MQTT_MNG_TYPE_CONTINUE         0x80   //当前状态正常继续
#define MQTT_MNG_TYPE_PUBLISH_RCVED    0x40   //收到发布消息标志
#define MQTT_MNG_TYPE_PUBLISH_RDY      0x20   //发布消息已准备好标志
#define MQTT_MNG_TYPE_PUBLISH_RCVER    0x10   //发布消息时现在为接收者，否则为发送者
#define MQTT_MNG_ARY_SOCK_ID_MASK       0x0F   //SockId

/*******************************************************************************
                          相关函数
*******************************************************************************/

//----------------------------初始化函数----------------------------------------
void MqttMng_Init(struct _MqttMng *pMqtt,
                  const struct _MqttUser *pUser,   //带入的用户信息
                  void *pUserHandle);               //用户信息需要的句柄


//-------------------------更新SockId----------------------------------------
void MqttMng_UdatetSockId(struct _MqttMng *pMqtt,
                          unsigned char SockId);
                    
//-------------------------接收处理函数----------------------------------------
void MqttMng_RcvPro(struct _MqttMng *pMqtt,
                    unsigned char *pData,  //数据区
                    unsigned short RcvLen,   //收到的数据长度
                    unsigned short BufSize); //缓冲区大小

//-------------------------快速任务函数----------------------------------------
void MqttMng_FastTask(struct _MqttMng *pMqtt);

//-------------------------10ms任务函数----------------------------------------
void MqttMng_Task(struct _MqttMng *pMqtt);

/*******************************************************************************
                              回调函数
*******************************************************************************/

//---------------------------------通讯无法与MQTT服务器通讯---------------------
//void MqttMng_ErrToServerNotify(void);
#define MqttMng_ErrToServerNotify() do{}while(0)


#endif //MQTT_MNG_H
