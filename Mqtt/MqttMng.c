/*******************************************************************************

                     Mqtt������ʵ��

*******************************************************************************/
#include "MqttMng.h"
#include "transport.h"

#include <string.h>

struct _MqttMng MqttMng;  //ֱ�ӵ�����

/*******************************************************************************
                          ���л����
���л�ָ�������Ϣת��ΪMQTT�������ϴ������������
*******************************************************************************/

static const MQTTPacket_connectData _connectDataDefault =  MQTTPacket_connectData_initializer;


//--------------------------���л��������ݺ���----------------------------------
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

//--------------------------���л��������ݺ���----------------------------------
//���ظ�ֵ��ʾ�����������
static signed char _SerializeSubscribe()
{
  unsigned char Pos = MqttMng.SubState;  
  if(Pos >= MqttMng.pUser->SubscribeAryCount) return -1;
  struct _MqttUserSubscribe *pSubscribe = MqttMng.pUser->pSubscribeAry + Pos;
	MqttMng.SerializeLen = MQTTSerialize_subscribe(MqttMng.SerializeBuf, 
                                                 MQTT_MNG_SERIALIZE_BUF_LEN,
                                                 0, //dup��־�̶�Ϊ0
                                                 pSubscribe->identifier, 
                                                 pSubscribe->Len, 
                                                 pSubscribe->pTopicAry,
                                                 pSubscribe->pQoS);
  return Pos;
}
	
/*******************************************************************************
                          ��غ���ʵ��
*******************************************************************************/

//��״̬��Ϣ������Ϊ:
//7bit:   �Ƿ�Ϊ�ȴ�������Ӧ��ģʽ��ֻ�д�ģʽ�²Ž��շ�������������Ϣ
//3-0bit: ��ȷ��ת����һģʽ
static const unsigned char _MstTypeInfo[16] = {
  0x00 | CONNECT,       //reserved   //����
  0x00 | CONNACK,       //CONNECT    //��������
	0x80 | SUBSCRIBE,     //CONNACK,   //������������Ӧ��
  0x00 | PUBLISH,      //PUBLISH,    //������Ϣ����ȴ�����������������Ϣ
  0x80 | PUBLISH,      //PUBACK,     //����Qos1��Ϣ�󣬷�������Ӧ��ȷ��
  0x80 | PUBREL,      //PUBREC,     //����Qos2��Ϣ���1����������Ӧ���ˣ���Ҫ�����յ�Ӧ��
  0x00 | PUBCOMP,     //PUBREL,     //����Qos2��Ϣ���2���������յ�Ӧ���������
	0x80 | PUBLISH,     //PUBCOMP,    //����Qos2��Ϣ���3����������ɵķ�������Ӧ
  0x00 | SUBACK,     //SUBSCRIBE,   //������Ϣ
  0x80 | SUBSCRIBE,  //SUBACK,      //�������Ķ�����ϢӦ��
  0x00 | UNSUBACK,   //UNSUBSCRIBE, //ȡ��������Ϣ
  0x80 | DISCONNECT, //UNSUBACK,    //��������ȡ��������ϢӦ��
	0x00 | PINGRESP,   //PINGREQ,     //�������ķ��ͣ�֪ͨ�������һ���
  0x80 | PUBLISH,   //PINGRESP,     //�������ķ�������Ӧ����֪�ҷ���������
  0x00 | 0,         //DISCONNECT    //��֪���ѶϿ�������������
  0x00 | 0,         //reserved      //����
};

//----------------------------��ʼ������----------------------------------------
void MqttMng_Init(void)
{
  memset(&MqttMng, 0, sizeof(struct _MqttMng));
}

//-------------------------���մ�����----------------------------------------
void MqttMng_RcvPro(unsigned char *pData,    //������
                    unsigned short RcvLen,   //�յ������ݳ���
                    unsigned short BufSize) //��������С
{
  //����ģʽ�յ�����������Ϣ��������
  if(MqttMng.eMsgTypes == PUBLISH){
    
    return;
  }
  //���ڵȴ�Ӧ��ģʽʱ����������
  if(!(_MstTypeInfo[MqttMng.eMsgTypes] & 0x80)) return;
  
  //������ȷ��,����һ״̬
  if(MQTTPacket_read(pData, RcvLen, transport_getdata) == MqttMng.eMsgTypes){
    MqttMng.eMsgTypes = (enum msgTypes)(_MstTypeInfo[MqttMng.eMsgTypes] & 0x0f);
    MqttMng.SubState = 0;
    if(MqttMng.eMsgTypes == SUBACK){
      MqttMng.SubState++;//��һ�鷢���ĵ���Ϣ
      MqttMng.Flag |= MQTT_MNG_TYPE_CONTINUE;//��������
    }
    MqttMng.WaitTimer = 0;    
  }
  else{//���ݴ���
    MqttMng.RetryIndex++;  //��������
  }
}

//-------------------------��ʱ����������-------------------------------------
//�����Ƿ����
static signed char _TimerOvPro(void)
{
  //���ڵȴ�Ӧ��ģʽ����ʱ������
  if(!(_MstTypeInfo[MqttMng.eMsgTypes] & 0x80)) return 1;
  
  if(MqttMng.WaitTimer){
    MqttMng.WaitTimer--;
    return 0;//δ��ʱ
  }
  
  //��ʱ����
  MqttMng.RetryIndex++;
  if(MqttMng.RetryIndex == 10) {
    MqttMng_ErrToServerNotify();
    MqttMng.WaitTimer = 255;    //��ȴ�ʱ���Եȴ����½���ͨѶ
    MqttMng.RetryIndex = 0;
  }
  MqttMng.eMsgTypes = CONNECT; //��������
  return 1;
}

//-------------------------10ms������----------------------------------------
void MqttMng_Task(void)
{
  //ǿ�Ƽ�����������ʱ
  if(!(MqttMng.Flag & MQTT_MNG_TYPE_CONTINUE)){
    if(_TimerOvPro() == 0) return; //�ȴ���
  }
  else MqttMng.Flag &= ~MQTT_MNG_TYPE_CONTINUE;
  
  //��������ʵ��
  switch(MqttMng.eMsgTypes){
    case 0: //����ʱ��ת������״̬
    case CONNECT: //�м���������״̬ʱ
      _SerializeConnect(); 
      if(MqttMng.SerializeLen <= 0) //�쳣
        MqttMng.Err = MqttMng.SerializeLen; 
      else{
        transport_sendPacketBuffer(0, MqttMng.SerializeBuf, MqttMng.SerializeLen);
        MqttMng.eMsgTypes = CONNACK; //ת���ȴ�����Ӧ��ģʽ
        MqttMng.WaitTimer = 255;    //��ȴ�ʱ��
      }
      break;    
    case SUBSCRIBE: //������״̬
      if(_SerializeSubscribe() < 0){//���������
        MqttMng.eMsgTypes = PUBLISH; //ת������ģʽ�ȴ�
        break;
      }
      if(MqttMng.SerializeLen <= 0) //�쳣
        MqttMng.Err = MqttMng.SerializeLen; 
      else{
        transport_sendPacketBuffer(0, MqttMng.SerializeBuf, MqttMng.SerializeLen);
        MqttMng.eMsgTypes = SUBACK; //ת���ȴ�����Ӧ��ģʽ
        MqttMng.WaitTimer = 255;    //��ȴ�ʱ��
      }
      break;
  }
}

