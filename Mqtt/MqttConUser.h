/*******************************************************************************

                     Mqtt-�����û���Ϣģ��
��ģ��Ӧ���޹�
*******************************************************************************/
#ifndef _MQTT_CON_USER_H
#define _MQTT_CON_USER_H


/*******************************************************************************
                             Ĭ��Э��֧��ʱ
*******************************************************************************/
//MqttConUserInfo�д洢����Ϣ����Ϊ:
//Cfg      = 0x0000     0x0000��ʾֱ��MQTTֱ�Ӵ���
//UserName = MQTT��֤ʱ�û��� �������û���֤
//UserPass = MQTT��֤ʱ���� ������������
//Info =     MQTTClient����ʱȱʡ

/*******************************************************************************
                          ������MQTTЭ��֧��ʱ
*******************************************************************************/
//�豸��Ԫ��,��:
//productKey = a14Xib5kdYd
//deviceName = light1983432
//deviceSecret = oLyaKqVxtRvjH284LdhqVgVUx1UPy6zq

//����MQTT����ʱ����
//clientId = SN1928339
//timestamp = 1539421321846
//signmethod = hmacsha1
//����password��content
//content=clientIdSN1928339deviceNamelight1983432productKeya14Xib5kdYdtimestamp1539421321846

//���ɵ�MQTT��Ϣ:
//mqttUsername = light1983432&a14Xib5kdYd
//mqttPassword = b2488041f64f425016b467ee1c94959ebd592ad1
//mqttClientId = SN1928339|securemode=3,signmethod=hmacsha1,timestamp=1539421321846|

//MqttConUserInfo�д洢����Ϣ����Ϊ:
//Cfg      = 0x2000 | (securemode << 4) | signmethod  (0x2000��ʾ����Э��)
//UserName = mqttUsername(��deviceName&productKey)
//UserPass = mqttPassword(���ɵļ�ֵ)
//Info =     timestamp&deviceSecret

//ע������������(MQTT����������Ӱ���豸�̻��豸��Ϣ)��
//   Topic/shadow/update/productKey/deviceName  ����app��dev������Ϣ����Topic��
//                     ������ƽ̨�յ���Ϣ�󣬻Ὣ��Ϣ�е�״̬���µ�Ӱ���豸��
//   Topic/shadow/get/productKey/deviceName, �豸Ӱ��(������)����״̬����Topic��
//                     �豸���Ĵ�Topic����Ϣ�󣬾ͻ��յ���Topic����������Ϣ��
//   �����ƽ�֧�ֵ�Qos1�ҳ־û�Sessionʱ�Ž�����3��

/*******************************************************************************
                            ��ض���
*******************************************************************************/

//��س��ȣ��û�ʹ��ʱ,��<��-1
#define MQTT_CON_USER_NAME_LEN  48   //�û�������
#define MQTT_CON_USER_PASS_LEN  48  //�û����볤��, ������MD5��̶�40��
#define MQTT_CON_USER_INFO_LEN  96   //�����Ϣ��͸��ʱΪ�ͻ���,����Ϊ��ֵ

/*******************************************************************************
                            ��ؽṹ
*******************************************************************************/
#include "MqttConUser.h"

struct _MqttConUserInfo{
  unsigned short Cfg;                              //������ã�������
  char UserName[MQTT_CON_USER_NAME_LEN];            //������û���
  char UserPass[MQTT_CON_USER_PASS_LEN];            //���������
  char Info[MQTT_CON_USER_INFO_LEN];                //�����Ϣ
};

//������ö���Ϊ:
#define MQTT_CON_USER_CFG_PROTOCOL_MASK    0xF000   //Э������
#define MQTT_CON_USER_CFG_OTHR_MASK    0x0FFF   //��Э�����Ͷ��ӵ�������Ϣ

//ע:Ϊ��ʡRAM�ռ�,������Eeprom�е�Ϊ��ʵ��Ϣ��MqttConUser�洢����Ϣ���滻Ϊ:
//UserName��ΪmqttUsername
//UserPass��ΪmqttPassword
//Info��ΪmqttClientId

//�û������Ϣ
struct _MqttConUser{
  struct _MqttConUserInfo Info;
  unsigned char AryId;  //��Ŵ˶������е�ID��
};

/*******************************************************************************
                          ��غ���
*******************************************************************************/

//-----------------------------��ʼ������---------------------------------------
void MqttConUser_Init(struct _MqttConUser *pMqttConUser,
                      unsigned char AryId,  //��Ŵ˶������е�ID��
                      signed char Inited);

//-------------------------------�õ�����λ-------------------------------------
#define MqttConUser_GetCfg(mqttConUser) ((mqttConUser)->Info.Cfg)

//-------------------------------��������λ-------------------------------------
void MqttConUser_SetCfg(struct _MqttConUser *pMqttConUser,
                        unsigned short Cfg);

//-----------------------------�õ�Э������-------------------------------------
#define  MqttConUser_GetProtocol(mqttConUser) \
    (MqttConUser_GetCfg(mqttConUser) & MQTT_CON_USER_CFG_PROTOCOL_MASK)

//----------------------------�õ����Info��Ϣ----------------------------------
//��ΪGUI�ӿڣ�TYPE����Ϊ: 
#define MQTT_CON_USER_NAME    0   //�û���
#define MQTT_CON_USER_PASS    1   //�û�����
#define MQTT_CON_USER_INFO    2   //�����Ϣ
void MqttConUser_GetInfo(const struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf);

//----------------------------�豸���Info��Ϣ----------------------------------
//��ΪGUI�ӿڣ�TYPE����ͬMqttConUser_GetInfo: 
void MqttConUser_SetInfo(struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf);

/*******************************************************************************
                          ��ػص�����
*******************************************************************************/

//-----------------------����ͨѶЭ�鵽Mqtt������Ϣ����-------------------------
//Info���Ƕ��������룬���ܺ�Ż�ԭ��
void MqttConUser_cbToMqttConInfo(struct _MqttConUser *pMqttConUser);



#endif //_MQTT_CON_USER_H

