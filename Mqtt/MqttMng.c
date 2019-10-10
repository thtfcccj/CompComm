/*******************************************************************************

                     Mqtt管理器实现

*******************************************************************************/
#include "MqttMng.h"
#include "transport.h"

#include <string.h>

struct _MqttMng MqttMng;  //直接单例化

/*******************************************************************************
                          序列化相关
序列化指将相关信息转换为MQTT在网络上传输入的数据流
*******************************************************************************/

static const MQTTPacket_connectData _connectDataDefault =  MQTTPacket_connectData_initializer;


//--------------------------序列化连接数据函数----------------------------------
static void _SerializeConnect(void)
{
	MQTTPacket_connectData *pData = &MqttMng.Buf.Connect;
  memcpy(pData, &_connectDataDefault, sizeof(MQTTPacket_connectData));
	pData->clientID.cstring = MqttMng.pUser->pClientId;
	pData->keepAliveInterval = MqttMng.pUser->KeepAlive;
	pData->cleansession = 1;
	pData->username.cstring = MqttMng.pUser->pUserName;
	pData->password.cstring = MqttMng.pUser->pUserPass;
	MqttMng.SerializeLen = MQTTSerialize_connect(MqttMng.SerializeBuf, 
                                               MQTT_MNG_SERIALIZE_BUF_LEN,
                                               pData);
}

//--------------------------序列化订阅数据函数----------------------------------
//返回负值表示订阅数据完成
static signed char _SerializeSubscribe()
{
  unsigned char Pos = MqttMng.SubState;  
  if(Pos >= MqttMng.pUser->SubscribeAryCount) return -1;
  struct _MqttUserSubscribe *pSubscribe = MqttMng.pUser->pSubscribeAry + Pos;
	MqttMng.SerializeLen = MQTTSerialize_subscribe(MqttMng.SerializeBuf, 
                                                 MQTT_MNG_SERIALIZE_BUF_LEN,
                                                 0, //dup标志固定为0
                                                 pSubscribe->identifier, 
                                                 pSubscribe->Len, 
                                                 pSubscribe->pTopicAry,
                                                 pSubscribe->pQoS);
  return Pos;
}
	
/*******************************************************************************
                          相关函数实现
*******************************************************************************/

//各状态信息，定义为:
//7bit:   是否为等待服务器应答模式，只有此模式下才接收服务器发来的消息
//3-0bit: 正确后，转到下一模式
static const unsigned char _MstTypeInfo[16] = {
  0x00 | CONNECT,       //reserved   //保留
  0x00 | CONNACK,       //CONNECT    //发起连接
	0x80 | SUBSCRIBE,     //CONNACK,   //服务器的连接应答
  0x00 | PUBLISH,      //PUBLISH,    //发布消息，或等待服务器发布来的消息
  0x80 | PUBLISH,      //PUBACK,     //发布Qos1消息后，服务器的应答确认
  0x80 | PUBREL,      //PUBREC,     //发布Qos2消息后第1步：服务器应答了，需要发已收到应答
  0x00 | PUBCOMP,     //PUBREL,     //发布Qos2消息后第2步：发已收到应答给服务器
	0x80 | PUBLISH,     //PUBCOMP,    //发布Qos2消息后第3步：发布完成的服务器回应
  0x00 | SUBACK,     //SUBSCRIBE,   //订阅消息
  0x80 | SUBSCRIBE,  //SUBACK,      //服务器的订阅消息应答
  0x00 | UNSUBACK,   //UNSUBSCRIBE, //取消订阅消息
  0x80 | DISCONNECT, //UNSUBACK,    //服务器的取消订阅消息应答
	0x00 | PINGRESP,   //PINGREQ,     //心跳报文发送，通知服务器我还在
  0x80 | PUBLISH,   //PINGRESP,     //心跳报文服务器响应，告知我服务器还在
  0x00 | 0,         //DISCONNECT    //告知我已断开服务器的连接
  0x00 | 0,         //reserved      //保留
};

//----------------------------初始化函数----------------------------------------
void MqttMng_Init(void)
{
  memset(&MqttMng, 0, sizeof(struct _MqttMng));
}

//-------------------------接收处理函数----------------------------------------
void MqttMng_RcvPro(unsigned char *pData,    //数据区
                    unsigned short RcvLen,   //收到的数据长度
                    unsigned short BufSize) //缓冲区大小
{
  //发布模式收到发布来的消息单独处理
  if(MqttMng.eMsgTypes == PUBLISH){
    
    return;
  }
  //不在等待应答模式时不处理数据
  if(!(_MstTypeInfo[MqttMng.eMsgTypes] & 0x80)) return;
  
  //数据正确了,到下一状态
  if(MQTTPacket_read(pData, RcvLen, transport_getdata) == MqttMng.eMsgTypes){
    MqttMng.eMsgTypes = (enum msgTypes)(_MstTypeInfo[MqttMng.eMsgTypes] & 0x0f);
    MqttMng.SubState = 0;
    if(MqttMng.eMsgTypes == SUBACK){
      MqttMng.SubState++;//下一组发订阅的消息
      MqttMng.Flag |= MQTT_MNG_TYPE_CONTINUE;//继续处理
    }
    MqttMng.WaitTimer = 0;    
  }
  else{//数据错误
    MqttMng.RetryIndex++;  //加速重试
  }
}

//-------------------------超时处理函数函数-------------------------------------
//返回是否继续
static signed char _TimerOvPro(void)
{
  //不在等待应答模式不计时并继续
  if(!(_MstTypeInfo[MqttMng.eMsgTypes] & 0x80)) return 1;
  
  if(MqttMng.WaitTimer){
    MqttMng.WaitTimer--;
    return 0;//未超时
  }
  
  //超时处理
  MqttMng.RetryIndex++;
  if(MqttMng.RetryIndex == 10) {
    MqttMng_ErrToServerNotify();
    MqttMng.WaitTimer = 255;    //最长等待时间以等待重新建立通讯
    MqttMng.RetryIndex = 0;
  }
  MqttMng.eMsgTypes = CONNECT; //重新连接
  return 1;
}

//-------------------------10ms任务函数----------------------------------------
void MqttMng_Task(void)
{
  //强制继续处理不处理超时
  if(!(MqttMng.Flag & MQTT_MNG_TYPE_CONTINUE)){
    if(_TimerOvPro() == 0) return; //等待中
  }
  else MqttMng.Flag &= ~MQTT_MNG_TYPE_CONTINUE;
  
  //发送数据实现
  switch(MqttMng.eMsgTypes){
    case 0: //开机时，转到连接状态
    case CONNECT: //中间重新连接状态时
      _SerializeConnect(); 
      if(MqttMng.SerializeLen <= 0) //异常
        MqttMng.Err = MqttMng.SerializeLen; 
      else{
        transport_sendPacketBuffer(0, MqttMng.SerializeBuf, MqttMng.SerializeLen);
        MqttMng.eMsgTypes = CONNACK; //转到等待连接应答模式
        MqttMng.WaitTimer = 255;    //最长等待时间
      }
      break;    
    case SUBSCRIBE: //到订阅状态
      if(_SerializeSubscribe() < 0){//订阅完成了
        MqttMng.eMsgTypes = PUBLISH; //转到发布模式等待
        break;
      }
      if(MqttMng.SerializeLen <= 0) //异常
        MqttMng.Err = MqttMng.SerializeLen; 
      else{
        transport_sendPacketBuffer(0, MqttMng.SerializeBuf, MqttMng.SerializeLen);
        MqttMng.eMsgTypes = SUBACK; //转到等待订阅应答模式
        MqttMng.WaitTimer = 255;    //最长等待时间
      }
      break;
  }
}

