/*******************************************************************************

                     Mqtt协议-用户接口定义
此定义配合MqttMng以与应用层交互
*******************************************************************************/
#ifndef MQTT_USER_H
#define MQTT_USER_H


/*******************************************************************************
                            相关结构
*******************************************************************************/

#include "MqttPacket.h"

//管理器需要的用户订阅信息的主题过滤器列表(注意不要太多以免长度超限，可多个)
struct _MqttUserSubscribe{
  MQTTString *pTopicAry;            //订阅的主题过滤器阵列
  int *pQoS;                        //对应阵列的服务级别,0,1,2
  unsigned char Len;               //主题阵列长度
};

//管理器需要的发布消息格式定义,收发相同
struct _MqttUserPublish{
  unsigned char Dup;         //重发标志,接收有效，1表示首次未回应重发的消息
  unsigned char Retained;    //保留标志,表示服务器端需一直保留此信息  
  int QoS;                    //对应阵列的服务级别,0,1,2
  MQTTString TopicName;       //主题名称 
  unsigned char *pPayload;  //收发用户数据内容(发送时需指向自已的缓冲区) 
  int PayloadLen;            //收发数据内容长度,0无数据要发送
};

//用户相关信息
struct _MqttUser{
  //得到字符串类对像,形参见定义,返回见形参对应定义
  char *(*pGetString)(void *pUserHandle, unsigned char TypeId);  
  
  //得到订阅,形参为订阅编号，返回NULL表示完成  
  struct _MqttUserSubscribe *(*pGetSubscribe)(void *pUserHandle,
                                              unsigned char No);
  //订阅返回时，将真正服务器上的Qos回写至原订阅中,不需要时此函数时可定义为NULL
  int *(*pGetSubscribeQosAry)(void *pUserHandle,
                              unsigned char No, int *pQosAryLen);  
  
  //收到发布数据后的处理
  //形参pRdPublish表示收到的发布数据,为NULL表示周期发布信息调用
  //写入时用PayloadLen为写缓冲大小，pPayload为需填充的数据。
  //PayloadLen表示已填填充数据长度，0时没有填充不发送
  //注: pWrPublish中的Dup位设置无效,写缓冲后,pRdPublish不可再使用!!
  void(*PublishPro)(void *pUserHandle,
                    const struct _MqttUserPublish *pRdPublish,
                     struct _MqttUserPublish *pWrPublish);  

  unsigned short KeepAlive;  //保活时间，s为单位
};

//得到字符串类对像按组组织，每个分组返回的字符不可共用一个缓冲区, 具定义为:
//组0: 
#define MQTT_USER_TYPE0_CLIENT_ID      0   //客户端ID号,返回NULL表示无
#define MQTT_USER_TYPE0_USER_NAME      1   //用户登录的名称,返回NULL表示无
#define MQTT_USER_TYPE0_USER_PASS      2   //用户登录的密码,返回NULL表示无

/*******************************************************************************
                          相关函数
*******************************************************************************/


#endif //MQTT_USER_H
