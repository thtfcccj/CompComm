/*******************************************************************************

                     Mqtt-�����û���Ϣʵ��
*******************************************************************************/


#include "MqttConUser.h"
#include "InfoBase.h"
#include "Eeprom.h"
#include <string.h>
#include "stringEx.h"

/*******************************************************************************
                          ��غ���ʵ��
*******************************************************************************/

//-----------------------------����Info----------------------------------
static void _ReloadUInfo(struct _MqttConUser *pMqttConUser)
{
  //���¶�ȡ�Ը���MQTTͨѶ��Ϣ
  Eeprom_Rd(MqttConUser_GetInfoBase(pMqttConUser->AryId),
            &pMqttConUser->Info, 
            sizeof(struct _MqttConUserInfo));
  //ת��MQTTͨѶ��Ϣ
  MqttConUser_cbToMqttConInfo(pMqttConUser);
}

//-----------------------------��ʼ������---------------------------------------
void MqttConUser_Init(struct _MqttConUser *pMqttConUser,
                      unsigned char AryId,  //��Ŵ˶������е�ID��
                      signed char Inited)
{
  memset(pMqttConUser, 0, sizeof(struct _MqttConUser));
  pMqttConUser->AryId = AryId;
  if(!Inited){
    MqttConUser_cbInitInfo(&pMqttConUser->Info);
    Eeprom_Wr(MqttConUser_GetInfoBase(AryId),
              &pMqttConUser->Info,
              sizeof(struct _MqttConUserInfo));
  }
  _ReloadUInfo(pMqttConUser);
}

//-------------------------------��������λ-------------------------------------
void MqttConUser_SetCfg(struct _MqttConUser *pMqttConUser,
                        unsigned short Cfg)
{
  pMqttConUser->Info.Cfg = Cfg;
  Eeprom_Wr(MqttConUser_GetInfoBase(pMqttConUser->AryId) +
            struct_offset(struct _MqttConUserInfo, Cfg),  &Cfg, 2); 
  
  _ReloadUInfo(pMqttConUser);
}

//----------------------------Info�洢��Ϣ���ұ�--------------------------------
//struct_offset(struct _MqttConUserInfo, UserPass)���ֱ����������
static const unsigned short _InfoBase[] = {
   2,                      //�û���,ǰ��Ϊcfg
  (2 + MQTT_CON_USER_NAME_LEN), //�û����� 
  (2 + MQTT_CON_USER_NAME_LEN + MQTT_CON_USER_PASS_LEN),//�����Ϣ   
};

static const unsigned char _InfoLen[] = {
  MQTT_CON_USER_NAME_LEN,     //�û���
  MQTT_CON_USER_PASS_LEN,     //�û����� 
  MQTT_CON_USER_INFO_LEN,    //�����Ϣ   
};

//----------------------------�õ����Info��Ϣ----------------------------------
void MqttConUser_GetInfo(const struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf)
{
  //ֱ�Ӵ�EEPROM�ж�ȡ��ֹ��ȡ���м�ֵ
  unsigned char Len = _InfoLen[Type];
  Eeprom_Rd(MqttConUser_GetInfoBase(pMqttConUser->AryId) +  _InfoBase[Type], 
            pBuf, Len);
  *(pBuf + Len - 1) = '\0';//ǿ�����ӽ����ַ�
}

//----------------------------�������Info��Ϣ----------------------------------
//��ΪGUI�ӿڣ�TYPE����ͬMqttConUser_GetInfo: 
void MqttConUser_SetInfo(struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf)
{
  //������
  if(Type > 3) return;
  unsigned char Len = strlen(pBuf) + 1; //�������ַ�
  if(Len > _InfoLen[Type]) Len = _InfoLen[Type];//����
  //����
  Eeprom_Wr(MqttConUser_GetInfoBase(pMqttConUser->AryId) + 
            _InfoBase[Type], pBuf, Len);

  _ReloadUInfo(pMqttConUser);
}








