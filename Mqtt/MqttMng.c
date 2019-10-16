/*******************************************************************************

                     Mqtt������ʵ��

*******************************************************************************/
#include "MqttMng.h"
#include "transport.h"

#include <string.h>

struct _MqttMng MqttMng;  //ֱ�ӵ�����

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
  0x80 | SUBSCRIBE,  //SUBACK,      //�������Ķ�����ϢӦ��
  0x00 | UNSUBACK,   //UNSUBSCRIBE, //ȡ��������Ϣ(����δʵ��)
  0x80 | DISCONNECT, //UNSUBACK,    //��������ȡ��������ϢӦ��(����δʵ��)
	0x00 | PINGRESP,   //PINGREQ,     //�������ķ��ͣ�֪ͨ�������һ���(����δʵ��)
  0x80 | PUBLISH,   //PINGRESP,     //�������ķ�������Ӧ����֪�ҷ���������(����δʵ��)
  0x00 | 0,         //DISCONNECT    //��֪���ѶϿ�������������(����δʵ��)
  0x00 | 0,         //reserved      //���� 
};

/*******************************************************************************
                          ����״̬����
*******************************************************************************/

//--------------------------�������л����ݺ���----------------------------------
static void _SendSerialize(enum msgTypes NextMsgTypes)//ת�����һ״̬
{
  if(MqttMng.SerializeLen <= 0) //�쳣
    MqttMng.Err = MqttMng.SerializeLen; 
  else{
    transport_sendPacketBuffer(0, MqttMng.SerializeBuf, MqttMng.SerializeLen);
    MqttMng.eMsgTypes = NextMsgTypes; 
    MqttMng.WaitTimer = 255;    //��ȴ�ʱ��
  }
}

//--------------------------�õ���ǰ��ID----------------------------------
static unsigned short _GetPacketId(unsigned char Dup)//�β�Ϊ�ط���־��0��_DUP
{
  //��ID����
  unsigned short PacketId;
  if(Dup) PacketId = MqttMng.CuPacketId; //ʹ��ԭPacketId�ط�
  else{//�°�
    PacketId = MqttMng.PacketIdIndex++; 
    MqttMng.CuPacketId = PacketId; //����˰�ID
  }
  return PacketId;
}

//--------------------------����״̬���ʹ�����--------------------------------
static void _ConnectSend(void)
{
  //�����л�����
	MQTTPacket_connectData *pData = &MqttMng.Buf.Connect;
  memcpy(pData, &_connectDataDefault, sizeof(MQTTPacket_connectData));
	pData->clientID.cstring = MqttMng.pUser->pGetString(MQTT_USER_TYPE0_CLIENT_ID);
	pData->keepAliveInterval = MqttMng.pUser->KeepAlive;
	pData->cleansession = 1;
	pData->username.cstring = MqttMng.pUser->pGetString(MQTT_USER_TYPE0_USER_NAME);
	pData->password.cstring = MqttMng.pUser->pGetString(MQTT_USER_TYPE0_USER_PASS);
	MqttMng.SerializeLen = MQTTSerialize_connect(MqttMng.SerializeBuf, 
                                               MQTT_MNG_SERIALIZE_BUF_LEN,
                                               pData);
  _SendSerialize(CONNACK); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}

//--------------------------����״̬������----------------------------------
//������״̬
static void _SubscribeSend(unsigned char Dup)//�β�Ϊ�ط���־��0��_DUP
{
  unsigned char Pos = MqttMng.SubState;  
  const struct _MqttUserSubscribe *pSubscribe = MqttMng.pUser->pGetSubscribe(Pos);  
  if(pSubscribe == NULL){//���������
    MqttMng.eMsgTypes = PUBLISH; //ת������ģʽ�ȴ�
    return;
  }

  //�����л�����
	MqttMng.SerializeLen = MQTTSerialize_subscribe(MqttMng.SerializeBuf, 
                                                 MQTT_MNG_SERIALIZE_BUF_LEN,
                                                 0, //dup��־�̶�Ϊ0
                                                 _GetPacketId(Dup), 
                                                 pSubscribe->Len, 
                                                 pSubscribe->pTopicAry,
                                                 pSubscribe->pQoS);
  _SendSerialize(SUBACK); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}


//--------------------------����״̬���ʹ�����--------------------------------
//����ǰ��Ҫ���������д��MqttMng.WrPublishBuf��
static void _PublishSend(unsigned char Dup)//�β�Ϊ�ط���־��0��_DUP
{
  //�ѽ��ղ��÷�����ģʽ
  MqttMng.Flag &= ~(MQTT_MNG_TYPE_PUBLISH_RDY | MqttMng.Flag & MQTT_MNG_TYPE_PUBLISH_RCVER); 
  
  MqttMng.SerializeLen = MQTTSerialize_publish(MqttMng.SerializeBuf, 
                                               MqttMng.SerializeLen,
                                               Dup,
                                               MqttMng.WrPublishBuf.QoS,
                                               MqttMng.WrPublishBuf.Retained,
                                               _GetPacketId(Dup),
                                               MqttMng.WrPublishBuf.TopicName,
                                               MqttMng.WrPublishBuf.pPayload, 
                                               MqttMng.WrPublishBuf.PayloadLen);
  //�����һ״̬
  enum msgTypes NextMsgTypes;
  if(MqttMng.WrPublishBuf.QoS == 0) //QoS1�����겻ȷ��,���ִ�״̬
    NextMsgTypes = PUBLISH;
  else if(MqttMng.WrPublishBuf.QoS == 1) //QoS1,�������յ���,�ȴ�Ӧ��ȷ��
    NextMsgTypes = PUBACK;
  else //QoS2Ϊ��������¼���ˣ��ȴ���¼ȷ��
    NextMsgTypes = PUBREC;    
  _SendSerialize(NextMsgTypes); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}

//------------------------�������״̬Ӧ���ͺ���------------------------------
//�����Ƿ����
static void _PublishAckSend(enum msgTypes ActMsgTypes,//Ӧ����Ϣ����
                            enum msgTypes NextMsgTypes,//ת������
                            unsigned char Dup)//�ط���־��0��_DUP
{
  MqttMng.SerializeLen = MQTTSerialize_ack(MqttMng.SerializeBuf,
                                           MQTT_MNG_SERIALIZE_BUF_LEN,
                                           ActMsgTypes,
                                           Dup,
                                           MqttMng.CuPacketId);
  MqttMng.eMsgTypes = NextMsgTypes;
  _SendSerialize(NextMsgTypes); //���Ͳ�ת���ȴ�����Ӧ��ģʽ
}

//--------------------------���յ������������ݴ�����--------------------------
static void _PublishRcvPro(void)
{
  struct _MqttUserPublish *pRdPublishBuf;
  //�յ������ȷ����л�������
  if(MqttMng.Flag & MQTT_MNG_TYPE_PUBLISH_RCVED){
    MqttMng.Flag &= ~MQTT_MNG_TYPE_PUBLISH_RCVED;//������
    unsigned short CuPacketId;
    if(MQTTDeserialize_publish(&MqttMng.Buf.RdPublish.Dup,
                               &MqttMng.Buf.RdPublish.QoS,
                               &MqttMng.Buf.RdPublish.Retained, 
                               &CuPacketId, 
                               &MqttMng.Buf.RdPublish.TopicName,
                               &MqttMng.Buf.RdPublish.pPayload, 
                               &MqttMng.Buf.RdPublish.PayloadLen,
                               MqttMng.SerializeBuf, 
                               MqttMng.SerializeLen) != 1){//�쳣
      MqttMng.RetryIndex++;
      return;
    }
    pRdPublishBuf = &MqttMng.Buf.RdPublish;
    MqttMng.CuPacketId = CuPacketId;//�������
  }
  else pRdPublishBuf = NULL;//Ϊ���ڵ���
  //�����û�����
  MqttMng.WrPublishBuf.pPayload = MqttMng.Buf.WrPublishPayloadBuf;//ָ���������ݻ���
  MqttMng.WrPublishBuf.PayloadLen = MQTT_MNG_USER_PAYLOAD_LEN;//�����С
  MqttMng.pUser->PublishPro(&MqttMng.WrPublishBuf, pRdPublishBuf);
  
  //�յ���������ʱ��鲢��ӦӦ��
  unsigned char RcvQoS;
  if(pRdPublishBuf != NULL){
    RcvQoS = pRdPublishBuf->QoS;
    if(RcvQoS == 1){//ȷ���յ���
      _PublishAckSend(PUBACK, PUBLISH,  0);//��Ϊ������,����ͽ����ˡ�
    }
    else if(RcvQoS == 2){//ȷ���յ����ݵĿͻ���,�ظ��Ѽ�¼
      _PublishAckSend(PUBREC, PUBREL,  0);//��Ϊ�����ߣ�ת��ȳ��ͷ��ź�
    }
  }
  else RcvQoS = 0;
  
  if(MqttMng.WrPublishBuf.PayloadLen){//�����ݻ���ʱ
    if(RcvQoS == 0) _PublishSend(0); //ֱ�ӷ���
    else  MqttMng.Flag |= MQTT_MNG_TYPE_PUBLISH_RDY; //�Ժ���
  }
}

//-------------------------���ͻ���ճ�ʱ������-------------------------------
static void _SendOrRcvOvPro(void)
{
  //��������ʵ��
  switch(MqttMng.eMsgTypes){
    case 0: //����ʱ��ת������״̬
    case CONNECT: //�м���������״̬ʱ
       _ConnectSend();break;    
    case SUBSCRIBE: //����״̬
      _SubscribeSend(0); break;
    case SUBACK: //����״̬��ʱ����
      _SubscribeSend(_DUP); break;      
    case PUBLISH://�յ��������ݻ������������ڵ���
      _PublishRcvPro(); break;
    case PUBACK:  //QoS1�������ȴ�������Ӧ���գ�Ӧ���������
    case PUBREC:  //QoS2�������ȴ���������¼Ӧ���գ������Ѽ�¼Ӧ��      
      if(MqttMng.Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //QoS1: ������ʱӦ�𼴽����������ڴ�״̬
        //QoS2: ������ʱ��ת���ͷŵȳ�״̬,�����ڴ�״̬      
        MqttMng.RetryIndex++;
      }
      else{//������ʱ,��ʾ���շ�����Ӧ��ʱ
        _PublishSend(_DUP); //�ٴη���
      }
      break;
    case PUBREL:  //QoS2�������յ���¼Ӧ��,��Ӧ�ͷţ��� �ȴ��������ͷ�Ӧ��
      if(MqttMng.Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //��ΪQoS2�����ߣ��ȴ��������ͷ�Ӧ���źų�ʱ,�ٴη����ѱ����ź�
        _PublishAckSend(PUBREC, PUBREL, _DUP);//��Ϊ�����ߣ�ת��ȴ��ͷ��ź�
      }
      else{//������ʱ,�յ���¼Ӧ��,��Ӧ�ͷ�
        _PublishAckSend(PUBREL, PUBCOMP, _DUP);//ת��ȴ����״̬
      }
      break;
    case PUBCOMP:  //QoS2�������ȴ���������ɣ��� ��������ź�
      if(MqttMng.Flag & MQTT_MNG_TYPE_PUBLISH_RCVER){
        //��ΪQoS2�����ߣ���������ź�
        _PublishAckSend(PUBCOMP, PUBLISH, 0);//ת����շ���״̬
      }
      else{//������ʱ,�ȴ�����������źų�ʱ
        MqttMng.RetryIndex++;
      }      
      break;
  }
}


/*******************************************************************************
                          ���պ���ʵ��
*******************************************************************************/

//-----------------����ģʽ���Ӧ��״̬�յ�������У�麯��-----------------------
//���ظ��쳣����������
signed char _PublishAckCheck(unsigned char CurIsRcver,
                             unsigned char *pData,     //������
                             unsigned short RcvLen)    //�յ������ݳ���
{
  if(CurIsRcver != (MqttMng.Flag & MQTT_MNG_TYPE_PUBLISH_RCVER))
    return -1;//����״̬�쳣
  
  if(MQTTDeserialize_ack(&MqttMng.Buf.PublishAck.PacketType,
                         &MqttMng.Buf.PublishAck.Dup, 
                         &MqttMng.Buf.PublishAck.PacketId, 
                         pData, RcvLen) != 1)
    return -2;//�������
  if(MqttMng.Buf.PublishAck.PacketType != MqttMng.eMsgTypes) //����Ҫ���յİ�
    return -3;
  //����
  if(MqttMng.CuPacketId != MqttMng.Buf.PublishAck.PacketId) //������
    return -3;  
  
  return 0;
}

//-------------------------------�ܽ��մ�����---------------------------------
void MqttMng_RcvPro(unsigned char *pData,    //������
                    unsigned short RcvLen,   //�յ������ݳ���
                    unsigned short BufSize) //��������С
{
  //����ģʽ�յ�����������Ϣ��������
  if(MqttMng.eMsgTypes == PUBLISH){
    if(RcvLen > MqttMng.SerializeLen) RcvLen = MqttMng.SerializeLen;
    memcpy(MqttMng.SerializeBuf,pData, RcvLen); //���嵽Ԥ�л����Ժ���
    //��������                        
    MqttMng.Flag |= MQTT_MNG_TYPE_CONTINUE | MQTT_MNG_TYPE_PUBLISH_RCVED;
    return;
  }
  //���ڵȴ����ģʽʱ����������
  if(!(_MstTypeInfo[MqttMng.eMsgTypes] & 0x80)) return;
  
  //���ȼ�鱨�������Ƿ���ȷ
  if(MQTTPacket_read(pData, RcvLen, transport_getdata) != MqttMng.eMsgTypes){ 
    MqttMng.RetryIndex++;  //��������ȴ�
    return;
  }
  
  //�������״̬���ڴ�������
  //PUBACK��QoS1: ����������ȴ��������յ�����
  //PUBREC: QoS2����: ������ȴ���������¼Ӧ�����
  //PUBREL: QoS2����: �ȴ��������ͷ�Ӧ�����
  //PUBCOMP: QoS2����: �ȴ��ȴ���������ɺ���
  if((MqttMng.eMsgTypes >= SUBACK) && (MqttMng.eMsgTypes <= PUBCOMP)){
    unsigned char CurIsRcver;
    if(MqttMng.eMsgTypes == PUBREL) CurIsRcver = MQTT_MNG_TYPE_PUBLISH_RCVER;
    else CurIsRcver = 0;
    signed char Resume = _PublishAckCheck(CurIsRcver, pData, RcvLen);
    if(Resume){
      MqttMng.RetryIndex++;  //��������ȴ�
      return;
    }
    goto _RcvNorEnd; //��ȷ����
  }

  //����Ӧ����
  if(MqttMng.eMsgTypes == SUBACK){
    if(MqttMng.pUser->pGetSubscribeQosAry != NULL){//Ҫ��дQoSʱ
      int Count;
      int *pQoSAry = MqttMng.pUser->pGetSubscribeQosAry(MqttMng.SubState, &Count);
      //�跴�л�����󣬼�鷵�ص���Ч�غ��У�QoS������д����Ƿ���ͬ
      if(MQTTDeserialize_suback(&MqttMng.Buf.SubscribeAck.PacketId,
                                Count,
                                &Count, 
                                pQoSAry, 
                                pData, 
                                RcvLen) != 1){
                                  //Ӧ�����ʶ���ݺ���              
      }
      //���PackedId��
    }
    goto _RcvNorEnd; //��ȷ����
  }
  //����δʵ��
  
  
_RcvNorEnd: //��ȷ����ʱ
  //ת��һ״̬
  MqttMng.eMsgTypes = (enum msgTypes)(_MstTypeInfo[MqttMng.eMsgTypes] & 0x0f);
  return;
}

/*******************************************************************************
                             ��غ���
*******************************************************************************/
//----------------------------��ʼ������----------------------------------------
void MqttMng_Init(const struct _MqttUser *pUser)    //����Ľӿ���Ϣ
{
  memset(&MqttMng, 0, sizeof(struct _MqttMng));
  MqttMng.pUser = pUser;
}

//-------------------------����������----------------------------------------
void MqttMng_FastTask(void)
{
  if(MqttMng.Flag & MQTT_MNG_TYPE_CONTINUE){
    MqttMng.Flag &= ~MQTT_MNG_TYPE_CONTINUE;
    _SendOrRcvOvPro(); //��������
    
  }
}

//-------------------------10ms������----------------------------------------
void MqttMng_Task(void)
{
  //���ڵȴ�Ӧ��ģʽ����ʱ������
  if(!(_MstTypeInfo[MqttMng.eMsgTypes] & 0x80)) return;
  
  if(MqttMng.WaitTimer){
    MqttMng.WaitTimer--;
    return; //δ��ʱ
  }
  
  //��ʱԤ����
  MqttMng.RetryIndex++;
  if(MqttMng.RetryIndex == 10) {
    MqttMng_ErrToServerNotify();
    MqttMng.WaitTimer = 255;    //��ȴ�ʱ���Եȴ����½���ͨѶ
    MqttMng.RetryIndex = 0;
    MqttMng.eMsgTypes = CONNECT; //��������
  }
  
  _SendOrRcvOvPro(); //��ʱ���ճ�ʱ����
}


