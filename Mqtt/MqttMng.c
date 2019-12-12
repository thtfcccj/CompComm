/*******************************************************************************

                     Mqtt������ʵ��

*******************************************************************************/
#include "MqttMng.h"
#include "transport.h"

#ifdef SUPPORT_PIC //memcpy��������ڴ�й¶������
  #include "stringEx.h"
  #define memcpyP(a,b,l) memcpyL((char*)a,(const char*)b,l)
#else //
  #include <string.h>
  #define memcpyP(a,b,l) memcpy(a,b,l)
#endif

static const MQTTPacket_connectData _connectDataDefault =  
  MQTTPacket_connectData_initializer;

#define _DUP      0x08    //�ظ���־

//---------------------------------��״̬��Ϣ��---------------------------------
//7bit:   �ȴ�������Ӧ���־
//3-0bit: ������Ӧ����ȷ���Զ�ת�뵽����һģʽ
static const unsigned char _MstTypeInfo[16] = {
  0x00 | CONNECT,       //reserved   //����
  0x00 | CONNACK,       //CONNECT    //��������
	0x80 | SUBSCRIBE,     //CONNACK,   //������������Ӧ��
  0x00 | PUBLISH,      //PUBLISH,    //����������Ϣ���գ��ȴ�����������������Ϣ
  0x80 | PUBLISH,      //PUBACK,     //QoS1�������ȴ�������Ӧ���գ�Ӧ���������
  0x80 | PUBREL,      //PUBREC,     //QoS2�������ȴ���������¼Ӧ���գ������Ѽ�¼Ӧ��
  0x80 | PUBCOMP,     //PUBREL,     //QoS2�������յ���¼Ӧ��,��Ӧ�ͷţ��� �ȴ��������ͷ�Ӧ��
	0x80 | PUBLISH,     //PUBCOMP,    //QoS2�������ȴ���������ɣ��� ��������ź�
  0x00 | SUBACK,     //SUBSCRIBE,   //������Ϣ
  0x80 | SUBSCRIBE,  //SUBACK,      //�������Ķ�����ϢӦ��,������һ����(�޶���ʱ�˳�)
  0x00 | UNSUBACK,   //UNSUBSCRIBE, //ȡ��������Ϣ(����δʵ��)
  0x80 | DISCONNECT, //UNSUBACK,    //��������ȡ��������ϢӦ��(����δʵ��)
	0x00 | PINGRESP,   //PINGREQ,     //�������ķ��ͣ�֪ͨ�������һ���
  0x80 | PUBLISH,   //PINGRESP,     //�������ķ�������Ӧ����֪�ҷ���������
  0x00 | 0,         //DISCONNECT    //��֪���ѶϿ�������������
  0x00 | 0,         //reserved      //���� 
};

/*******************************************************************************
                          ����״̬����
*******************************************************************************/

//--------------------------�������л����ݺ���----------------------------------
static void _SendSerialize(struct _MqttMng *pMqtt,
                           enum msgTypes NextMsgTypes)//ת�����һ״̬
{
  if(pMqtt->SerializeLen <= 0) //�쳣
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

//--------------------------�õ���ǰ��ID----------------------------------
static unsigned short _GetPacketId(struct _MqttMng *pMqtt,
                                     unsigned char Dup)//�β�Ϊ�ط���־��0��_DUP
{
  //��ID����
  unsigned short PacketId;
  if(Dup) PacketId = pMqtt->CuPacketId; //ʹ��ԭPacketId�ط�
  else{//�°�
    PacketId = pMqtt->PacketIdIndex++; 
    pMqtt->CuPacketId = PacketId; //����˰�ID
  }
  return PacketId;
}

//--------------------------����״̬���ʹ�����--------------------------------
static void _ConnectSend(struct _MqttMng *pMqtt)
{
  //�����л�����
	MQTTPacket_connectData *pData = &pMqtt->Buf.Connect;
  memcpyP(pData, &_connectDataDefault, sizeof(_connectDataDefault));//Pic��sizeof(MQTTPacket_connectData���ڴ����))
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
  _SendSerialize(pMqtt, CONNACK); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}

//--------------------------�������ķ��ͺ���----------------------------------
//������״̬
static void _HeartBeatSend(struct _MqttMng *pMqtt)
{
    pMqtt->SerializeLen = MQTTSerialize_pingreq(pMqtt->SerializeBuf,
                                                MQTT_MNG_SERIALIZE_BUF_LEN);
    _SendSerialize(pMqtt, PINGRESP); 
}

//--------------------------����״̬������----------------------------------
//������״̬
static void _SubscribeSend(struct _MqttMng *pMqtt,
                           unsigned char Dup)//�β�Ϊ�ط���־��0��_DUP
{
  const struct _MqttUserSubscribe *pSubscribe = 
    pMqtt->pUser->pGetSubscribe(pMqtt->pUserHandle, pMqtt->SubState);  
  if(pSubscribe == NULL){//���������
    pMqtt->SubState = 0;
    pMqtt->eMsgTypes = PUBLISH; //ת������ģʽ�ȴ�
    return;
  }
  //�����л�����
	pMqtt->SerializeLen = MQTTSerialize_subscribe(pMqtt->SerializeBuf, 
                                                 MQTT_MNG_SERIALIZE_BUF_LEN,
                                                 0, //dup��־�̶�Ϊ0
                                                 _GetPacketId(pMqtt, Dup), 
                                                 pSubscribe->Len, 
                                                 pSubscribe->pTopicAry,
                                                 pSubscribe->pQoS);
  _SendSerialize(pMqtt, SUBACK); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}


//--------------------------����״̬���ʹ�����--------------------------------
//����ǰ��Ҫ���������д��pMqtt->WrPublishBuf��
static void _PublishSend(struct _MqttMng *pMqtt,
                         unsigned char Dup)//�β�Ϊ�ط���־��0��_DUP
{
  //�ѽ��ղ��÷�����ģʽ
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
  //�����һ״̬
  enum msgTypes NextMsgTypes;
  if(pMqtt->WrPublishBuf.QoS == 2) //QoS2Ϊ��������¼���ˣ��ȴ���¼ȷ��
    NextMsgTypes = PUBREC;
  else if(pMqtt->WrPublishBuf.QoS == 1) //QoS1,�������յ���,�ȴ�Ӧ��ȷ��
    NextMsgTypes = PUBACK;
  else //����ΪQoS0�����겻ȷ��,���ִ�״̬
    NextMsgTypes = PUBLISH;    
  _SendSerialize(pMqtt, NextMsgTypes); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}

//------------------------�������״̬Ӧ���ͺ���------------------------------
//�����Ƿ����
static void _PublishAckSend(struct _MqttMng *pMqtt,
                            enum msgTypes ActMsgTypes,//Ӧ����Ϣ����
                            enum msgTypes NextMsgTypes,//ת������
                            unsigned char Dup)//�ط���־��0��_DUP
{
  pMqtt->SerializeLen = MQTTSerialize_ack(pMqtt->SerializeBuf,
                                           MQTT_MNG_SERIALIZE_BUF_LEN,
                                           ActMsgTypes,
                                           Dup,
                                           pMqtt->CuPacketId);
  pMqtt->eMsgTypes = NextMsgTypes;
  _SendSerialize(pMqtt, NextMsgTypes); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}

//--------------------------���յ������������ݴ�����--------------------------
static void _PublishRcvPro(struct _MqttMng *pMqtt)
{
  struct _MqttUserPublish *pRdPublishBuf;
  //�յ������ȷ����л�������
  int RcvQoS;
  if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVED){
    pMqtt->Flag &= ~MQTT_MNG_TYPE_PUBLISH_RCVED;//������
    unsigned short CuPacketId;
    if(MQTTDeserialize_publish(&pMqtt->Buf.RdPublish.Dup,
                               &RcvQoS,
                               &pMqtt->Buf.RdPublish.Retained, 
                               &CuPacketId, 
                               &pMqtt->Buf.RdPublish.TopicName,
                               &pMqtt->Buf.RdPublish.pPayload, 
                               &pMqtt->Buf.RdPublish.PayloadLen,
                               pMqtt->SerializeBuf, 
                               pMqtt->SerializeLen) != 1){//�쳣
      pMqtt->RetryIndex++;
      return;
    }
    pMqtt->Buf.RdPublish.QoS = RcvQoS;
    pRdPublishBuf = &pMqtt->Buf.RdPublish;
    pMqtt->CuPacketId = CuPacketId;//�������
    
  }
  else{//Ϊ���ڵ���
    RcvQoS = 0;//��������
    pRdPublishBuf = NULL;
    pMqtt->WaitTimer = pMqtt->pUser->GetTime(pMqtt->pUserHandle,
                                             MQTT_USER_TIME_PERTROL_PEARIOD);
  }
  
  //�����û�����
  pMqtt->WrPublishBuf.pPayload = pMqtt->Buf.WrPublishPayloadBuf;//ָ���������ݻ���
  pMqtt->WrPublishBuf.PayloadLen = MQTT_MNG_USER_PAYLOAD_LEN;//�����С
  pMqtt->pUser->PublishPro(pMqtt->pUserHandle,
                           pRdPublishBuf,
                           &pMqtt->WrPublishBuf);
  
  //�յ���������ʱ��鲢�Ȼ�ӦӦ��(�����Ƿ����)
  if(pRdPublishBuf != NULL){
    if(RcvQoS == 1)//ȷ���յ���
      _PublishAckSend(pMqtt, PUBACK, PUBLISH,  0);//��Ϊ������,����ͽ����ˡ�
    else if(RcvQoS == 2)//ȷ���յ����ݵĿͻ���,�ظ��Ѽ�¼
      _PublishAckSend(pMqtt, PUBREC, PUBREL,  0);//��Ϊ�����ߣ�ת��ȳ��ͷ��ź�
  }
  
  if(pMqtt->WrPublishBuf.PayloadLen){//�����ݻ���ʱ
    if(RcvQoS == 0) _PublishSend(pMqtt, 0); //ֱ�ӷ���
    else  pMqtt->Flag |= MQTT_MNG_TYPE_PUBLISH_RDY; //�Ժ���
  }
}

//-------------------------���ͻ���ճ�ʱ������-------------------------------
static void _SendOrRcvOvPro(struct _MqttMng *pMqtt)
{
  //��������ʵ��
  switch(pMqtt->eMsgTypes){
    case 0: //����ʱ��ת������״̬
    case CONNECT: //����״̬ʱ
    case CONNACK: //û��Ӧ�ǽ��ŷ���
       _ConnectSend(pMqtt);break;    
    case SUBSCRIBE: //����״̬
      _SubscribeSend(pMqtt, 0); break;
    case SUBACK: //����״̬��ʱ����
      _SubscribeSend(pMqtt, _DUP); break;      
    case PUBLISH://�յ��������ݻ������������ڵ���
      _PublishRcvPro(pMqtt); break;
    case PUBACK:  //QoS1�������ȴ�������Ӧ���գ�Ӧ���������
    case PUBREC:  //QoS2�������ȴ���������¼Ӧ���գ������Ѽ�¼Ӧ��      
      if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //QoS1: ������ʱӦ�𼴽����������ڴ�״̬
        //QoS2: ������ʱ��ת���ͷŵȳ�״̬,�����ڴ�״̬      
        pMqtt->RetryIndex++;
      }
      else{//������ʱ,��ʾ���շ�����Ӧ��ʱ
        _PublishSend(pMqtt, _DUP); //�ٴη���
      }
      break;
    case PUBREL:  //QoS2�������յ���¼Ӧ��,��Ӧ�ͷţ��� �ȴ��������ͷ�Ӧ��
      if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //��ΪQoS2�����ߣ��ȴ��������ͷ�Ӧ���źų�ʱ,�ٴη����ѱ����ź�
        _PublishAckSend(pMqtt, PUBREC, PUBREL, _DUP);//��Ϊ�����ߣ�ת��ȴ��ͷ��ź�
      }
      else{//������ʱ,�յ���¼Ӧ��,��Ӧ�ͷ�
        _PublishAckSend(pMqtt, PUBREL, PUBCOMP, _DUP);//ת��ȴ����״̬
      }
      break;
    case PUBCOMP:  //QoS2�������ȴ���������ɣ��� ��������ź�
      if(pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //��ΪQoS2�����ߣ���������ź�
        _PublishAckSend(pMqtt, PUBCOMP, PUBLISH, 0);//ת����շ���״̬
      }
      else{//������ʱ,�ȴ�����������źų�ʱ
        pMqtt->RetryIndex++;
      }      
      break;
    case PINGRESP: //�������ĳ�ʱ
      _HeartBeatSend(pMqtt);
      break;
    default:// UNSUBSCRIBE, UNSUBACK,PINGREQ, DISCONNECT  ��֧�ֵ�״̬,PINGREQ���ڴ�
      break;
  }
  
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER
    pMqtt->Flag |= MQTT_MNG_EN_RCV;
  #endif
}


/*******************************************************************************
                          ���պ���ʵ��
*******************************************************************************/

//-----------------����ģʽ���Ӧ��״̬�յ�������У�麯��-----------------------
//���ظ��쳣����������
signed char _PublishAckCheck(struct _MqttMng *pMqtt,
                             unsigned char CurIsRcver,
                             unsigned char *pData,     //������
                             unsigned short RcvLen)    //�յ������ݳ���
{
  if(CurIsRcver != (pMqtt->Flag & MQTT_MNG_TYPE_PUBLISH_RCVER))
    return -1;//����״̬�쳣
  
  if(MQTTDeserialize_ack(&pMqtt->Buf.PublishAck.PacketType,
                         &pMqtt->Buf.PublishAck.Dup, 
                         &pMqtt->Buf.PublishAck.PacketId, 
                         pData, RcvLen) != 1)
    return -2;//�������
  if(pMqtt->Buf.PublishAck.PacketType != pMqtt->eMsgTypes) //����Ҫ���յİ�
    return -3;
  //����
  if(pMqtt->CuPacketId != pMqtt->Buf.PublishAck.PacketId) //������
    return -3;  
  
  return 0;
}

//-------------------------------�ܽ��մ�����---------------------------------
void MqttMng_RcvPro(struct _MqttMng *pMqtt,
                    unsigned char *pData,    //������
                    unsigned short RcvLen,   //�յ������ݳ���
                    unsigned short BufSize) //��������С
{
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER
  if(!(pMqtt->Flag & MQTT_MNG_EN_RCV)) return;//���������
    pMqtt->Flag &= ~MQTT_MNG_EN_RCV;
    memcpyL(pMqtt->SerializeBuf,pData, RcvLen); //���嵽Ԥ�л����Ժ���
    pMqtt->SerializeLen = RcvLen;
    pMqtt->Flag |= MQTT_MNG_BUF_RCV;
    
  }//end MqttMng_RcvPro()
  //========================�����Ժ�����========================
  //���ڿ�������MQTT_MNG_BUF_RCV��λʱ����
  void MqttMng_RcvLater(struct _MqttMng *pMqtt){
    unsigned char *pData = pMqtt->SerializeBuf;
    unsigned short RcvLen = pMqtt->SerializeLen;
    //����ԭMqttMng_RcvPro()�ڲ�ִ��.....
  #endif
  //===========================���մ�������=======================
  //����ģʽ�յ�����������Ϣ��������
  if(pMqtt->eMsgTypes == PUBLISH){
    if(RcvLen > pMqtt->SerializeLen) RcvLen = pMqtt->SerializeLen;
    memcpyP(pMqtt->SerializeBuf,pData, RcvLen); //���嵽Ԥ�л����Ժ���
    //��������                        
    pMqtt->Flag |= MQTT_MNG_TYPE_CONTINUE | MQTT_MNG_TYPE_PUBLISH_RCVED;
    return;
  }
  //���ڵȴ����ģʽʱ����������
  if(!(_MstTypeInfo[pMqtt->eMsgTypes] & 0x80)) return;
  
  //���ȼ�鱨�������Ƿ���ȷ
  if(MQTTPacket_read(pData, RcvLen, transport_getdata) != pMqtt->eMsgTypes){ 
    pMqtt->RetryIndex++;  //��������ȴ�
    return;
  }
  
  //�������״̬���ڴ�������
  //PUBACK��QoS1: ����������ȴ��������յ�����
  //PUBREC: QoS2����: ������ȴ���������¼Ӧ�����
  //PUBREL: QoS2����: �ȴ��������ͷ�Ӧ�����
  //PUBCOMP: QoS2����: �ȴ��ȴ���������ɺ���
  if((pMqtt->eMsgTypes >= PUBACK) && (pMqtt->eMsgTypes <= PUBCOMP)){
    unsigned char CurIsRcver;
    if(pMqtt->eMsgTypes == PUBREL) CurIsRcver = MQTT_MNG_TYPE_PUBLISH_RCVER;
    else CurIsRcver = 0;
    signed char Resume = _PublishAckCheck(pMqtt, CurIsRcver, pData, RcvLen);
    if(Resume){
      pMqtt->RetryIndex++;  //��������ȴ�
      return;
    }
    goto _RcvNorEnd; //��ȷ����
  }

  //����Ӧ����
  if(pMqtt->eMsgTypes == SUBACK){
    if(pMqtt->pUser->pGetSubscribeQosAry != NULL){//Ҫ��дQoSʱ
      int Count;
      int *pQoSAry = pMqtt->pUser->pGetSubscribeQosAry(pMqtt->pUserHandle,
                                                       pMqtt->SubState, &Count);
      //�跴�л�����󣬼�鷵�ص���Ч�غ��У�QoS������д����Ƿ���ͬ
      if(MQTTDeserialize_suback(&pMqtt->Buf.SubscribeAck.PacketId,
                                Count,
                                &Count, 
                                pQoSAry, 
                                pData, 
                                RcvLen) != 1){
                                  //Ӧ�����ʶ���ݺ���              
      }
      //���PackedId��
    }
    pMqtt->SubState++; //��һ����
    goto _RcvNorEnd; //��ȷ����
  }
  //����δʵ��(���������Ļظ�),��Ϊ��ȷ
  
_RcvNorEnd: //��ȷ����ʱ
  //ת��һ״̬
  pMqtt->eMsgTypes = (enum msgTypes)(_MstTypeInfo[pMqtt->eMsgTypes] & 0x0f);
  pMqtt->RetryIndex = 0;//��λ
  pMqtt->Flag |= MQTT_MNG_TYPE_CONTINUE;
  return;
}

/*******************************************************************************
                             ��غ���
*******************************************************************************/

#include <string.h>

//----------------------------��ʼ������----------------------------------------
void MqttMng_Init(struct _MqttMng *pMqtt,
                  const struct _MqttUser *pUser,    //������û���Ϣ
                  void *pUserHandle)                 //�û���Ϣ��Ҫ�ľ��          
{
  memset(pMqtt, 0, sizeof(struct _MqttMng));
  pMqtt->pUser = pUser;
  pMqtt->pUserHandle = pUserHandle;
  pMqtt->HeartBeatIndex = 1000;
}

//-------------------------����������----------------------------------------
void MqttMng_FastTask(struct _MqttMng *pMqtt)
{
  //�ȴ�����
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER
  if(pMqtt->Flag & MQTT_MNG_BUF_RCV){
    pMqtt->Flag &= ~MQTT_MNG_BUF_RCV;
    MqttMng_RcvLater(pMqtt);
  }
  #endif
  
  if(pMqtt->Flag & MQTT_MNG_TYPE_CONTINUE){
    pMqtt->Flag &= ~MQTT_MNG_TYPE_CONTINUE;
    _SendOrRcvOvPro(pMqtt); //��������
  }
}

//------------------------����������������------------------------------------
//10msΪ��λ
static void _HeartBeatTask(struct _MqttMng *pMqtt)
{
  if(pMqtt->eMsgTypes != PUBLISH) return; 
  
  if(pMqtt->HeartBeatIndex) pMqtt->HeartBeatIndex--;
  else{//�������ķ�ֹ����������()
    pMqtt->HeartBeatIndex = pMqtt->pUser->GetTime(pMqtt->pUserHandle, 
                                                  MQTT_USER_TIME_HEART_BEAT);
    if(pMqtt->HeartBeatIndex < 1000) pMqtt->HeartBeatIndex = 1000;//����10s
    
    _HeartBeatSend(pMqtt); 
    #ifdef SUPPORT_MQTT_MNG_RCV_LATER
      pMqtt->Flag |= MQTT_MNG_EN_RCV;
    #endif
  }  
}


//-------------------------10ms������----------------------------------------
void MqttMng_Task(struct _MqttMng *pMqtt)
{
  if(pMqtt->Flag & MQTT_MNG_WORK_PAUSE) return; //��ͣ��
 
   _HeartBeatTask(pMqtt);//�������Ĵ���

  if(pMqtt->WaitTimer){
    pMqtt->WaitTimer--;
    return; //�ȴ�ʱ��δ��
  }

  //�ȴ���Ӧģʽ��ʱԤ����
  if((_MstTypeInfo[pMqtt->eMsgTypes] & 0x80)){
    pMqtt->RetryIndex++;
    if(pMqtt->RetryIndex >= //�������Դ��������Ͽ���,������
       pMqtt->pUser->GetTime(pMqtt->pUserHandle,MQTT_USER_TIME_RETRY_COUNT)) {
      MqttMng_cbErrToServerNotify(pMqtt);
      //���³�ʼ����ģ�鲢��õȴ�ʱ��
      const struct _MqttUser *pUser = pMqtt->pUser;
      MqttMng_Init(pMqtt, pUser, pMqtt->pUserHandle); 
      pMqtt->WaitTimer = pUser->GetTime(pMqtt->pUserHandle, MQTT_USER_TIME_RE_CONNECT);
      if(pMqtt->WaitTimer == 0) //�ֶ�����ʱ��ͣ
        pMqtt->Flag = MQTT_MNG_WORK_PAUSE;
      return;
    }
  }
   
  //��ʱ���ճ�ʱ����
  #ifdef SUPPORT_MQTT_MNG_RCV_LATER //ȥ������־
    pMqtt->Flag &= ~(MQTT_MNG_BUF_RCV | MQTT_MNG_EN_RCV);
  #endif

  _SendOrRcvOvPro(pMqtt); 
}


