/*******************************************************************************

                     MqttЭ��-�û��ӿڶ���
�˶������MqttMng����Ӧ�ò㽻��
*******************************************************************************/
#ifndef MQTT_USER_H
#define MQTT_USER_H


/*******************************************************************************
                            ��ؽṹ
*******************************************************************************/

#include "MqttPacket.h"

//��������Ҫ���û�������Ϣ������������б�(ע�ⲻҪ̫�����ⳤ�ȳ��ޣ��ɶ��)
struct _MqttUserSubscribe{
  MQTTString *pTopicAry;            //���ĵ��������������
  int *pQoS;                        //��Ӧ���еķ��񼶱�,0,1,2
  unsigned char Len;               //�������г���
};

//��������Ҫ�ķ�����Ϣ��ʽ����,�շ���ͬ
struct _MqttUserPublish{
  unsigned char Dup;         //�ط���־,������Ч��1��ʾ�״�δ��Ӧ�ط�����Ϣ
  unsigned char Retained;    //������־,��ʾ����������һֱ��������Ϣ  
  int QoS;                    //��Ӧ���еķ��񼶱�,0,1,2
  MQTTString TopicName;       //�������� 
  unsigned char *pPayload;  //�շ��û���������(����ʱ��ָ�����ѵĻ�����) 
  int PayloadLen;            //�շ��������ݳ���,0������Ҫ����
};

//�û������Ϣ
struct _MqttUser{
  //�õ��ַ��������,�βμ�����,���ؼ��βζ�Ӧ����
  char *(*pGetString)(void *pUserHandle, unsigned char TypeId);  
  
  //�õ�����,�β�Ϊ���ı�ţ�����NULL��ʾ���  
  struct _MqttUserSubscribe *(*pGetSubscribe)(void *pUserHandle,
                                              unsigned char No);
  //���ķ���ʱ���������������ϵ�Qos��д��ԭ������,����Ҫʱ�˺���ʱ�ɶ���ΪNULL
  int *(*pGetSubscribeQosAry)(void *pUserHandle,
                              unsigned char No, int *pQosAryLen);  
  
  //�յ��������ݺ�Ĵ���
  //�β�pRdPublish��ʾ�յ��ķ�������,ΪNULL��ʾ���ڷ�����Ϣ����
  //д��ʱ��PayloadLenΪд�����С��pPayloadΪ���������ݡ�
  //PayloadLen��ʾ����������ݳ��ȣ�0ʱû����䲻����
  //ע: pWrPublish�е�Dupλ������Ч,д�����,pRdPublish������ʹ��!!
  void(*PublishPro)(void *pUserHandle,
                    const struct _MqttUserPublish *pRdPublish,
                     struct _MqttUserPublish *pWrPublish);  

  unsigned short KeepAlive;  //����ʱ�䣬sΪ��λ
};

//�õ��ַ������������֯��ÿ�����鷵�ص��ַ����ɹ���һ��������, �߶���Ϊ:
//��0: 
#define MQTT_USER_TYPE0_CLIENT_ID      0   //�ͻ���ID��,����NULL��ʾ��
#define MQTT_USER_TYPE0_USER_NAME      1   //�û���¼������,����NULL��ʾ��
#define MQTT_USER_TYPE0_USER_PASS      2   //�û���¼������,����NULL��ʾ��

/*******************************************************************************
                          ��غ���
*******************************************************************************/


#endif //MQTT_USER_H
