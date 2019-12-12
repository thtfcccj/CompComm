/*******************************************************************************

                     Mqtt管理器实现

*******************************************************************************/
#include "MqttMng.h"
#include "transport.h"

#ifdef SUPPORT_PIC //memcpy会出错导致内存泄露！！！
  #include "stringEx.h"
  #define memcpyP(a,b,l) memcpyL((char*)a,(const char*)b,l)
#else //
  #include <string.h>
  #define memcpyP(a,b,l) memcpy(a,b,l)
#endif

static const MQTTPacket_connectData _connectDataDefault =  
  MQTTPacket_connectData_initializer;

#define _DUP      0x08    //重复标志

//---------------------------------各状态信息表---------------------------------
//7bit:   等待服务器应答标志
//3-0bit: 服务器应答正确后，自动转入到的下一模式
static const unsigned char _MstTypeInfo[16] = {
  0x00 | CONNECT,       //reserved   //保留
  0x00 | CONNACK,       //CONNECT    //发起连接
	0x80 | SUBSCRIBE,     //CONNACK,   //服务器的连接应答
  0x00 | PUBLISH,      //PUBLISH,    //发：发布消息，收：等待服务器发布来的消息
  0x80 | PUBLISH,      //PUBACK,     //QoS1：发：等待服务器应答，收：应答服务器：
  0x80 | PUBREL,      //PUBREC,     //QoS2：发：等待服务器记录应答，收：发布已记录应答
  0x80 | PUBCOMP,     //PUBREL,     //QoS2：发：收到记录应后,回应释放，收 等待服务器释放应答
	0x80 | PUBLISH,     //PUBCOMP,    //QoS2：发：等待服务器完成，收 发送完成信号
  0x00 | SUBACK,     //SUBSCRIBE,   //订阅消息
  0x80 | SUBSCRIBE,  //SUBACK,      //服务器的订阅消息应答,继续下一订阅(无订阅时退出)
  0x00 | UNSUBACK,   //UNSUBSCRIBE, //取消订阅消息(★暂未实现)
  0x80 | DISCONNECT, //UNSUBACK,    //服务器的取消订阅消息应答(★暂未实现)
	0x00 | PINGRESP,   //PINGREQ,     //心跳报文发送，通知服务器我还在
  0x80 | PUBLISH,   //PINGRESP,     //心跳报文服务器响应，告知我服务器还在
  0x00 | 0,         //DISCONNECT    //告知我已断开服务器的连接
  0x00 | 0,         //reserved      //保留 
};

/*******************************************************************************
                          发送状态处理
*******************************************************************************/

//--------------------------发送序列化数据函数----------------------------------
static void _SendSerialize(struct _MqttMng *pMqtt,
                           enum msgTypes NextMsgTypes)//转入的下一状态
{
  if(pMqtt->SerializeLen <= 0) //异常
    pMqtt->Err = pMqtt->SerializeLen; 
  else{
    transport_sendPacketBuffer(pMqtt->SocketId, 
                               pMqtt->SerializeBuf, 
                               pMqtt->SerializeLen);
    pMqtt->eMsgTypes = NextMsgTypes; 
    pMqtt->WaitTimer = pMqtt->pUser->GetTime(pMqtt->pUserHandle,
                                             MQTT_USER_TIME_SERVER_RESP);
  }
}

//--------------------------得到当前包ID----------------------------------
static unsigned short _GetPacketId(struct _MqttMng *pMqtt,
                                     unsigned char Dup)//形参为重发标志，0或_DUP
{
  //包ID处理
  unsigned short PacketId;
  if(Dup) PacketId = pMqtt->CuPacketId; //使用原PacketId重发
  else{//新包
    PacketId = pMqtt->PacketIdIndex++; 
    pMqtt->CuPacketId = PacketId; //缓冲此包ID
  }
  return PacketId;
}

//--------------------------连接状态发送处理函数--------------------------------
static void _ConnectSend(struct _MqttMng *pMqtt)
{
  //先序列化数据
	MQTTPacket_connectData *pData = &pMqtt->Buf.Connect;
  memcpyP(pData, &_connectDataDefault, sizeof(_connectDataDefault));//Pic中sizeof(MQTTPacket_connectData会内存溢出))
	pData->clientID.cstring = pMqtt->pUser->pGetString(pMqtt->pUserHandle,
                                                     MQTT_USER_TYPE0_CLIENT_ID);
	pData->keepAliveInterval = pMqtt->pUser->GetTime(pMqtt->pUserHandle, 
                                                   MQTT_USER_TIME_KEEP_ALIVE);
	pData->cleansession = 1;
	pData->username.cstring = pMqtt->pUser->pGetString(pMqtt->pUserHandle,
                                                     MQTT_USER_TYPE0_USER_NAME);
	pData->password.cstring = pMqtt->pUser->pGetString(pMqtt->pUserHandle,
                                                     MQTT_USER_TYPE0_USER_PASS);
	pMqtt->SerializeLen = MQTTSerialize_connect(pMqtt->SerializeBuf, 
                                               MQTT_MNG_SERIALIZE_BUF_LEN,
                                               pData);
  _SendSerialize(pMqtt, CONNACK); //发送并转到等待连接应答模式
}

//--------------------------心跳报文发送函数----------------------------------
//单向发送状态
static void _HeartBeatSend(struct _MqttMng *pMqtt)
{
    pMqtt->SerializeLen = MQTTSerialize_pingreq(pMqtt->SerializeBuf,
                                                MQTT_MNG_SERIALIZE_BUF_LEN);
    _SendSerialize(pMqtt, PINGRESP); 
}

//--------------------------订阅状态处理函数----------------------------------
//单向发送状态
static void _SubscribeSend(struct _MqttMng *pMqtt,
                           unsigned char Dup)//形参为重发标志，0或_DUP
{
  const struct _MqttUserSubscribe *pSubscribe = 
    pMqtt->pUser->pGetSubscribe(pMqtt->pUserHandle, pMqtt->SubState);  
  if(pSubscribe == NULL){//订阅完成了
    pMqtt->SubState = 0;
    pMqtt->eMsgTypes = PUBLISH; //转到发布模式等待
    return;
  }
  //先序列化数据
	pMqtt->SerializeLen = MQTTSerialize_subscribe(pMqtt->SerializeBuf, 
                                                 MQTT_MNG_SERIALIZE_BUF_LEN,
                                                 0, //dup标志固定为0
                                                 _GetPacketId(pMqtt, Dup), 
                                                 pSubscribe->Len, 
                                                 pSubscribe->pTopicAry,
                                                 pSubscribe->pQoS);
  _SendSerialize(pMqtt, SUBACK); //发送并转到等待订阅应答模式
}


//--------------------------发布状态发送处理函数--------------------------------
//需提前把要编码的数据写入pMqtt->WrPublishBuf中
static void _PublishSend(struct _MqttMng *pMqtt,
                         unsigned char Dup)//形参为重发标志，0或_DUP
{
  //已接收并置发送者模式
  pMqtt->Flag &= ~(MQTT_MNG_TYPE_PUBLISH_RDY | pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER); 
  
  pMqtt->SerializeLen = MQTTSerialize_publish(pMqtt->SerializeBuf, 
                                               MQTT_MNG_SERIALIZE_BUF_LEN,
                                               Dup,
                                               pMqtt->WrPublishBuf.QoS,
                                               pMqtt->WrPublishBuf.Retained,
                                               _GetPacketId(pMqtt, Dup),
                                               pMqtt->WrPublishBuf.TopicName,
                                               pMqtt->WrPublishBuf.pPayload, 
                                               pMqtt->WrPublishBuf.PayloadLen);
  //获得下一状态
  enum msgTypes NextMsgTypes;
  if(pMqtt->WrPublishBuf.QoS == 2) //QoS2为服务器记录上了，等待记录确认
    NextMsgTypes = PUBREC;
  else if(pMqtt->WrPublishBuf.QoS == 1) //QoS1,服务器收到后,等待应答确认
    NextMsgTypes = PUBACK;
  else //其它为QoS0发送完不确认,保持此状态
    NextMsgTypes = PUBLISH;    
  _SendSerialize(pMqtt, NextMsgTypes); //发送并转到等待订阅应答模式
}

//------------------------发布相关状态应答发送函数------------------------------
//返回是否错误
static void _PublishAckSend(struct _MqttMng *pMqtt,
                            enum msgTypes ActMsgTypes,//应答消息类型
                            enum msgTypes NextMsgTypes,//转入类型
                            unsigned char Dup)//重发标志，0或_DUP
{
  pMqtt->SerializeLen = MQTTSerialize_ack(pMqtt->SerializeBuf,
                                           MQTT_MNG_SERIALIZE_BUF_LEN,
                                           ActMsgTypes,
                                           Dup,
                                           pMqtt->CuPacketId);
  pMqtt->eMsgTypes = NextMsgTypes;
  _SendSerialize(pMqtt, NextMsgTypes); //发送并转到等待订阅应答模式
}

//--------------------------接收到发布来的数据处理函数--------------------------
static void _PublishRcvPro(struct _MqttMng *pMqtt)
{
  struct _MqttUserPublish *pRdPublishBuf;
  //收到数据先反序列化并处理
  int RcvQoS;
  if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVED){
    pMqtt->Flag &= ~MQTT_MNG_TYPE_PUBLISH_RCVED;//处理了
    unsigned short CuPacketId;
    if(MQTTDeserialize_publish(&pMqtt->Buf.RdPublish.Dup,
                               &RcvQoS,
                               &pMqtt->Buf.RdPublish.Retained, 
                               &CuPacketId, 
                               &pMqtt->Buf.RdPublish.TopicName,
                               &pMqtt->Buf.RdPublish.pPayload, 
                               &pMqtt->Buf.RdPublish.PayloadLen,
                               pMqtt->SerializeBuf, 
                               pMqtt->SerializeLen) != 1){//异常
      pMqtt->RetryIndex++;
      return;
    }
    pMqtt->Buf.RdPublish.QoS = RcvQoS;
    pRdPublishBuf = &pMqtt->Buf.RdPublish;
    pMqtt->CuPacketId = CuPacketId;//留存待用
    
  }
  else{//为周期调用
    RcvQoS = 0;//后期条件
    pRdPublishBuf = NULL;
    pMqtt->WaitTimer = pMqtt->pUser->GetTime(pMqtt->pUserHandle,
                                             MQTT_USER_TIME_PERTROL_PEARIOD);
  }
  
  //交由用户处理
  pMqtt->WrPublishBuf.pPayload = pMqtt->Buf.WrPublishPayloadBuf;//指向填入数据缓冲
  pMqtt->WrPublishBuf.PayloadLen = MQTT_MNG_USER_PAYLOAD_LEN;//缓冲大小
  pMqtt->pUser->PublishPro(pMqtt->pUserHandle,
                           pRdPublishBuf,
                           &pMqtt->WrPublishBuf);
  
  //收到发布数据时检查并先回应应答(不管是否回数)
  if(pRdPublishBuf != NULL){
    if(RcvQoS == 1)//确认收到了
      _PublishAckSend(pMqtt, PUBACK, PUBLISH,  0);//作为接收者,回完就结束了。
    else if(RcvQoS == 2)//确认收到数据的客户端,回复已记录
      _PublishAckSend(pMqtt, PUBREC, PUBREL,  0);//作为接收者，转入等持释放信号
  }
  
  if(pMqtt->WrPublishBuf.PayloadLen){//有数据缓冲时
    if(RcvQoS == 0) _PublishSend(pMqtt, 0); //直接发布
    else  pMqtt->Flag |= MQTT_MNG_TYPE_PUBLISH_RDY; //稍后发送
  }
}

//-------------------------发送或接收超时处理函数-------------------------------
static void _SendOrRcvOvPro(struct _MqttMng *pMqtt)
{
  //发送数据实现
  switch(pMqtt->eMsgTypes){
    case 0: //开机时，转到连接状态
    case CONNECT: //连接状态时
    case CONNACK: //没响应是接着发送
       _ConnectSend(pMqtt);break;    
    case SUBSCRIBE: //订阅状态
      _SubscribeSend(pMqtt, 0); break;
    case SUBACK: //订阅状态超时处理
      _SubscribeSend(pMqtt, _DUP); break;      
    case PUBLISH://收到发布数据或主动发送周期到了
      _PublishRcvPro(pMqtt); break;
    case PUBACK:  //QoS1：发：等待服务器应答，收：应答服务器：
    case PUBREC:  //QoS2：发：等待服务器记录应答，收：发布已记录应答      
      if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //QoS1: 接收者时应答即接束，不存在此状态
        //QoS2: 接收者时已转入释放等持状态,不存在此状态      
        pMqtt->RetryIndex++;
      }
      else{//发送者时,表示接收服务器应答超时
        _PublishSend(pMqtt, _DUP); //再次发布
      }
      break;
    case PUBREL:  //QoS2：发：收到记录应后,回应释放，收 等待服务器释放应答
      if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //作为QoS2接收者，等待服务器释放应答信号超时,再次发送已保存信号
        _PublishAckSend(pMqtt, PUBREC, PUBREL, _DUP);//作为接收者，转入等待释放信号
      }
      else{//发送者时,收到记录应后,回应释放
        _PublishAckSend(pMqtt, PUBREL, PUBCOMP, _DUP);//转入等待完成状态
      }
      break;
    case PUBCOMP:  //QoS2：发：等待服务器完成，收 发送完成信号
      if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //作为QoS2接收者，发送完成信号
        _PublishAckSend(pMqtt, PUBCOMP, PUBLISH, 0);//转入接收发布状态
      }
      else{//发送者时,等待服务器完成信号超时
        pMqtt->RetryIndex++;
      }      
      break;
    case PINGRESP: //心跳报文超时
      _HeartBeatSend(pMqtt);
      break;
    default:// UNSUBSCRIBE, UNSUBACK,PINGREQ, DISCONNECT  不支持的状态,PINGREQ不在此
      break;
  }
  
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER
    pMqtt->Flag |= MQTT_MNG_EN_RCV;
  #endif
}


/*******************************************************************************
                          接收函数实现
*******************************************************************************/

//-----------------发布模式相关应答状态收到的数据校验函数-----------------------
//返回负异常，否则正常
signed char _PublishAckCheck(struct _MqttMng *pMqtt,
                             unsigned char CurIsRcver,
                             unsigned char *pData,     //数据区
                             unsigned short RcvLen)    //收到的数据长度
{
  if(CurIsRcver != (pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER))
    return -1;//工作状态异常
  
  if(MQTTDeserialize_ack(&pMqtt->Buf.PublishAck.PacketType,
                         &pMqtt->Buf.PublishAck.Dup, 
                         &pMqtt->Buf.PublishAck.PacketId, 
                         pData, RcvLen) != 1)
    return -2;//解码错误
  if(pMqtt->Buf.PublishAck.PacketType != pMqtt->eMsgTypes) //不是要接收的包
    return -3;
  //检查包
  if(pMqtt->CuPacketId != pMqtt->Buf.PublishAck.PacketId) //包错误
    return -3;  
  
  return 0;
}

//-------------------------------总接收处理函数---------------------------------
void MqttMng_RcvPro(struct _MqttMng *pMqtt,
                    unsigned char *pData,    //数据区
                    unsigned short RcvLen,   //收到的数据长度
                    unsigned short BufSize) //缓冲区大小
{
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER
  if(!(pMqtt->Flag & MQTT_MNG_EN_RCV)) return;//不允许接收
    pMqtt->Flag &= ~MQTT_MNG_EN_RCV;
    memcpyL(pMqtt->SerializeBuf,pData, RcvLen); //缓冲到预列化中稍后处理
    pMqtt->SerializeLen = RcvLen;
    pMqtt->Flag |= MQTT_MNG_BUF_RCV;
    
  }//end MqttMng_RcvPro()
  //========================接收稍后处理函数========================
  //放在快速任务MQTT_MNG_BUF_RCV置位时调用
  void MqttMng_RcvLater(struct _MqttMng *pMqtt){
    unsigned char *pData = pMqtt->SerializeBuf;
    unsigned short RcvLen = pMqtt->SerializeLen;
    //继续原MqttMng_RcvPro()内部执行.....
  #endif
  //===========================接收处理程序段=======================
  //发布模式收到发布来的消息单独处理
  if(pMqtt->eMsgTypes == PUBLISH){
    if(RcvLen > pMqtt->SerializeLen) RcvLen = pMqtt->SerializeLen;
    memcpyP(pMqtt->SerializeBuf,pData, RcvLen); //缓冲到预列化中稍后处理
    //继续处理                        
    pMqtt->Flag |= MQTT_MNG_TYPE_CONTINUE | MQTT_MNG_TYPE_PUBLISH_RCVED;
    return;
  }
  //不在等待相关模式时不处理数据
  if(!(_MstTypeInfo[pMqtt->eMsgTypes] & 0x80)) return;
  
  //首先检查报文类型是否正确
  if(MQTTPacket_read(pData, RcvLen, transport_getdata) != pMqtt->eMsgTypes){ 
    pMqtt->RetryIndex++;  //错误继续等待
    return;
  }
  
  //发布相关状态后期处理，含：
  //PUBACK：QoS1: 发：发布后等待服务器收到后处理
  //PUBREC: QoS2：发: 发布后等待服务器记录应答后处理
  //PUBREL: QoS2：收: 等待服务器释放应答后处理
  //PUBCOMP: QoS2：发: 等待等待服务器完成后处理
  if((pMqtt->eMsgTypes >= PUBACK) && (pMqtt->eMsgTypes <= PUBCOMP)){
    unsigned char CurIsRcver;
    if(pMqtt->eMsgTypes == PUBREL) CurIsRcver = MQTT_MNG_TYPE_PUBLISH_RCVER;
    else CurIsRcver = 0;
    signed char Resume = _PublishAckCheck(pMqtt, CurIsRcver, pData, RcvLen);
    if(Resume){
      pMqtt->RetryIndex++;  //错误继续等待
      return;
    }
    goto _RcvNorEnd; //正确接收
  }

  //订阅应答处理
  if(pMqtt->eMsgTypes == SUBACK){
    if(pMqtt->pUser->pGetSubscribeQosAry != NULL){//要回写QoS时
      int Count;
      int *pQoSAry = pMqtt->pUser->pGetSubscribeQosAry(pMqtt->pUserHandle,
                                                       pMqtt->SubState, &Count);
      //需反列化处理后，检查返回的有效载荷中，QoS序列与写入的是否相同
      if(MQTTDeserialize_suback(&pMqtt->Buf.SubscribeAck.PacketId,
                                Count,
                                &Count, 
                                pQoSAry, 
                                pData, 
                                RcvLen) != 1){
                                  //应答包错识，暂忽略              
      }
      //检查PackedId略
    }
    pMqtt->SubState++; //下一订阅
    goto _RcvNorEnd; //正确接收
  }
  //其它未实现(含心跳报文回复),认为正确
  
_RcvNorEnd: //正确接收时
  //转下一状态
  pMqtt->eMsgTypes = (enum msgTypes)(_MstTypeInfo[pMqtt->eMsgTypes] & 0x0f);
  pMqtt->RetryIndex = 0;//复位
  pMqtt->Flag |= MQTT_MNG_TYPE_CONTINUE;
  return;
}

/*******************************************************************************
                             相关函数
*******************************************************************************/

#include <string.h>

//----------------------------初始化函数----------------------------------------
void MqttMng_Init(struct _MqttMng *pMqtt,
                  const struct _MqttUser *pUser,    //带入的用户信息
                  void *pUserHandle)                 //用户信息需要的句柄          
{
  memset(pMqtt, 0, sizeof(struct _MqttMng));
  pMqtt->pUser = pUser;
  pMqtt->pUserHandle = pUserHandle;
  pMqtt->HeartBeatIndex = 1000;
}

//-------------------------快速任务函数----------------------------------------
void MqttMng_FastTask(struct _MqttMng *pMqtt)
{
  //先处理缓冲
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER
  if(pMqtt->Flag & MQTT_MNG_BUF_RCV){
    pMqtt->Flag &= ~MQTT_MNG_BUF_RCV;
    MqttMng_RcvLater(pMqtt);
  }
  #endif
  
  if(pMqtt->Flag & MQTT_MNG_TYPE_CONTINUE){
    pMqtt->Flag &= ~MQTT_MNG_TYPE_CONTINUE;
    _SendOrRcvOvPro(pMqtt); //立即处理
  }
}

//------------------------心跳报文子任务函数------------------------------------
//10ms为单位
static void _HeartBeatTask(struct _MqttMng *pMqtt)
{
  if(pMqtt->eMsgTypes != PUBLISH) return; 
  
  if(pMqtt->HeartBeatIndex) pMqtt->HeartBeatIndex--;
  else{//心跳报文防止服务器掉线()
    pMqtt->HeartBeatIndex = pMqtt->pUser->GetTime(pMqtt->pUserHandle, 
                                                  MQTT_USER_TIME_HEART_BEAT);
    if(pMqtt->HeartBeatIndex < 1000) pMqtt->HeartBeatIndex = 1000;//最少10s
    
    _HeartBeatSend(pMqtt); 
    #ifdef SUPPORT_MQTT_MNG_RCV_LATER
      pMqtt->Flag |= MQTT_MNG_EN_RCV;
    #endif
  }  
}


//-------------------------10ms任务函数----------------------------------------
void MqttMng_Task(struct _MqttMng *pMqtt)
{
  if(pMqtt->Flag & MQTT_MNG_WORK_PAUSE) return; //暂停了
 
   _HeartBeatTask(pMqtt);//心跳报文处理

  if(pMqtt->WaitTimer){
    pMqtt->WaitTimer--;
    return; //等待时间未到
  }

  //等待回应模式超时预处理
  if((_MstTypeInfo[pMqtt->eMsgTypes] & 0x80)){
    pMqtt->RetryIndex++;
    if(pMqtt->RetryIndex >= //大于重试次数，网断开了,需重连
       pMqtt->pUser->GetTime(pMqtt->pUserHandle,MQTT_USER_TIME_RETRY_COUNT)) {
      MqttMng_cbErrToServerNotify(pMqtt);
      //重新初始化此模块并获得等待时间
      const struct _MqttUser *pUser = pMqtt->pUser;
      MqttMng_Init(pMqtt, pUser, pMqtt->pUserHandle); 
      pMqtt->WaitTimer = pUser->GetTime(pMqtt->pUserHandle, MQTT_USER_TIME_RE_CONNECT);
      if(pMqtt->WaitTimer == 0) //手动控制时暂停
        pMqtt->Flag = MQTT_MNG_WORK_PAUSE;
      return;
    }
  }
   
  //定时接收超时处理
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER //去锁定标志
    pMqtt->Flag &= ~(MQTT_MNG_BUF_RCV | MQTT_MNG_EN_RCV);
  #endif

  _SendOrRcvOvPro(pMqtt); 
}


