/*******************************************************************************

                     Mqtt-�����û���Ϣʵ��
*******************************************************************************/


#include "MqttConUser.h"
#include "InfoBase.h"
#include "Eeprom.h"
#include <string.h>

/*******************************************************************************
                          ��غ���ʵ��
*******************************************************************************/

//-----------------------------��ʼ������---------------------------------------
void MqttConUser_Init(struct _MqttConUser *pMqttConUser,
                      unsigned char AryId,  //��Ŵ˶������е�ID��
                      signed char Inited)
{
  memset(pMqttConUser, 0, sizeof(struct _MqttConUser));
  pMqttConUser->AryId = AryId;
  if(!Inited){
    Eeprom_Wr(MqttConUser_GetInfoBase(AryId),
              &pMqttConUser->Info,
              sizeof(struct _MqttConUserInfo));
  }
  else{
    Eeprom_Rd(MqttConUser_GetInfoBase(AryId),
              &pMqttConUser->Info,
              sizeof(struct _MqttConUserInfo));
    //��ֹ�洢�쳣��β��ǿ�Ƽ�0
    pMqttConUser->Info.UserName[MQTT_CON_USER_NAME_LEN - 1] = '\0';
    pMqttConUser->Info.UserPass[MQTT_CON_USER_PASS_LEN - 1] = '\0';
    pMqttConUser->Info.Info[MQTT_CON_USER_INFO_LEN - 1] = '\0';
  }

  //ת��MQTTͨѶ��Ϣ
  if(pMqttConUser->Info.Cfg) MqttConUser_cbToMqttConInfo(pMqttConUser);
}

//-------------------------------��������λ-------------------------------------
void MqttConUser_SetCfg(struct _MqttConUser *pMqttConUser,
                        unsigned char Cfg)
{
  pMqttConUser->Info.Cfg = Cfg;
  Eeprom_Wr(MqttConUser_GetInfoBase(pMqttConUser->AryId) +
            struct_offset(struct _MqttConUserInfo, Cfg),  &Cfg, 2); 
}

//----------------------------Info�洢��Ϣ���ұ�--------------------------------
static const unsigned short _InfoBase[] = {
  struct_offset(struct _MqttConUserInfo, UserName),//�û���
  struct_offset(struct _MqttConUserInfo, UserPass),//�û����� 
  struct_offset(struct _MqttConUserInfo, Info),    //�����Ϣ   
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
  Eeprom_Rd(MqttConUser_GetInfoBase(pMqttConUser->AryId) + 
            _InfoBase[Type], pBuf, _InfoLen[Type]);  
}

//----------------------------�������Info��Ϣ----------------------------------
//��ΪGUI�ӿڣ�TYPE����ͬMqttConUser_GetInfo: 
void MqttConUser_SetInfo(struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf)
{
  //������
  if(Type > 3) return;
  unsigned char Len = _InfoLen[Type];
  if(strlen(pBuf) >= Len) *(pBuf + Len - 1) = '\0';//ǿ�ƽض�
  //����
  Eeprom_Wr(MqttConUser_GetInfoBase(pMqttConUser->AryId) + 
            _InfoBase[Type], pBuf, Len);
  //���¶�ȡ�Ը���MQTTͨѶ��Ϣ
  Eeprom_Rd(MqttConUser_GetInfoBase(pMqttConUser->AryId),
            &pMqttConUser->Info, 
            sizeof(struct _MqttConUserInfo));
  if(pMqttConUser->Info.Cfg) MqttConUser_cbToMqttConInfo(pMqttConUser);
}








