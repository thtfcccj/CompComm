/*******************************************************************************

                     MqttЭ�������-��Ϊ�ͻ���ʱ
* ��֧��MQTT���ݰ��Ĵ��д�����֧�ֲ��д���
* ������������MQTT�Ĺ���״̬��������ݼ�����Э�鲻�ڴ˴���
* ��������������transportģ��ʵ�������շ�����ײ������޹ء�
* �˹�����ʹ��paho MQTTЭ��ջ�������ݵ����л����뷴���л���

*******************************************************************************/
#ifndef MQTT_MNG_H
#define MQTT_MNG_H

/*******************************************************************************
                            ��ض���
*******************************************************************************/

#ifndef MQTT_MNG_SERIALIZE_BUF_LEN  //���л��ַ����ȣ���Ӧ�����
  #define MQTT_MNG_SERIALIZE_BUF_LEN    300    
#endif

#ifndef MQTT_MNG_USER_PAYLOAD_LEN  //�û����س��ȣ���Ӧ�����
  #define MQTT_MNG_USER_PAYLOAD_LEN     128   
#endif

/*******************************************************************************
                            ��ؽṹ
*******************************************************************************/
#include "MqttPacket.h"
#include "MqttUser.h"

//����״̬
enum _MqttMngState{
  MqttMngState_Idie = 0,         //����״̬���ȴ�����
  MqttMngState_Connect = 1,      //������,�ȴ���������Ӧ
  MqttMngState_Subscribe = 2,    //���Ͷ���״̬
  MqttMngState_Publish = 2,      //������Ϣ״̬(��ȡ���ĺ��)
};

//����ģʽӦ�������
struct _MqttMngSubscribeAck{
  unsigned short PacketId;
};

//����ģʽӦ�������
struct _MqttMngPublishAck{
  unsigned char  PacketType; 
  unsigned char  Dup;
  unsigned short PacketId;
};

//���л�/�����л���Ҫ�����ݻ���
union _MqttMngDataBuf{
  MQTTPacket_connectData Connect;          //������Ҫ������
  struct _MqttMngSubscribeAck SubscribeAck; //����ģʽӦ�������
  struct _MqttUserPublish RdPublish;        //����ʱ��������ʱ����
  struct _MqttMngPublishAck PublishAck;  //����ģʽӦ�������
  //����ʱ�����ݵ��û����ػ���
  unsigned char WrPublishPayloadBuf[MQTT_MNG_USER_PAYLOAD_LEN];   
};

struct _MqttMng{
  //״̬��أ�
  enum msgTypes  eMsgTypes;             //��Ϣ����
  unsigned char SubState;              //��Ϣ���Ͷ�Ӧ��״̬
  unsigned char WaitTimer;            //��״̬��ʱ�ȴ���ʱ����10msΪ��λ
  unsigned char RetryIndex;           //���״̬���Դ���
  //�û�������أ�
  const struct _MqttUser *pUser;    //������û���Ϣ
  //�������
  union _MqttMngDataBuf Buf;            //��ת������
  unsigned char SerializeBuf[MQTT_MNG_SERIALIZE_BUF_LEN]; //���л����ݻ�����
  signed short SerializeLen;            //���л������Ч���ݸ���,��ֵ��MQTT����
  unsigned short PacketIdIndex;         //��������
  unsigned short CuPacketId;           //��ǰ���ڲ�������ID��
  struct _MqttUserPublish WrPublishBuf; //����ʱд���ݻ���
  
  signed char Err;      //�����־
  unsigned char Flag;  //��ر�־��������
};

//��ر�־����Ϊ:
#define MQTT_MNG_TYPE_CONTINUE         0x80   //��ǰ״̬��������
#define MQTT_MNG_TYPE_PUBLISH_RCVED    0x40   //�յ�������Ϣ��־
#define MQTT_MNG_TYPE_PUBLISH_RDY      0x20   //������Ϣ��׼���ñ�־
#define MQTT_MNG_TYPE_PUBLISH_RCVER    0x10   //������Ϣʱ����Ϊ�����ߣ�����Ϊ������


extern struct _MqttMng MqttMng;  //ֱ�ӵ�����

/*******************************************************************************
                          ��غ���
*******************************************************************************/

//----------------------------��ʼ������----------------------------------------
void MqttMng_Init(void);

//-------------------------���մ�����----------------------------------------
void MqttMng_RcvPro(unsigned char *pData,  //������
                    unsigned short RcvLen,   //�յ������ݳ���
                    unsigned short BufSize); //��������С

//-------------------------10ms������----------------------------------------
void MqttMng_Task(void);


/*******************************************************************************
                              �ص�����
*******************************************************************************/

//---------------------------------ͨѶ�޷���MQTT������ͨѶ---------------------
void MqttMng_ErrToServerNotify(void);



#endif //MQTT_MNG_H
