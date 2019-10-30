/*******************************************************************************

                     Mqtt-连接用户信息实现
*******************************************************************************/


#include "MqttConUser.h"
#include "InfoBase.h"
#include "Eeprom.h"
#include <string.h>
#include "stringEx.h"

/*******************************************************************************
                          相关函数实现
*******************************************************************************/

//-----------------------------重载Info----------------------------------
static void _ReloadUInfo(struct _MqttConUser *pMqttConUser)
{
  //重新读取以更新MQTT通讯信息
  Eeprom_Rd(MqttConUser_GetInfoBase(pMqttConUser->AryId),
            &pMqttConUser->Info, 
            sizeof(struct _MqttConUserInfo));
  //转到MQTT通讯信息
  if(pMqttConUser->Info.Cfg) MqttConUser_cbToMqttConInfo(pMqttConUser);
}

//-----------------------------初始化函数---------------------------------------
void MqttConUser_Init(struct _MqttConUser *pMqttConUser,
                      unsigned char AryId,  //存放此多例阵列的ID号
                      signed char Inited)
{
  memset(pMqttConUser, 0, sizeof(struct _MqttConUser));
  pMqttConUser->AryId = AryId;
  if(!Inited){
    Eeprom_Wr(MqttConUser_GetInfoBase(AryId),
              &pMqttConUser->Info,
              sizeof(struct _MqttConUserInfo));
  }
  else _ReloadUInfo(pMqttConUser);

}

//-------------------------------设置配置位-------------------------------------
void MqttConUser_SetCfg(struct _MqttConUser *pMqttConUser,
                        unsigned short Cfg)
{
  pMqttConUser->Info.Cfg = Cfg;
  Eeprom_Wr(MqttConUser_GetInfoBase(pMqttConUser->AryId) +
            struct_offset(struct _MqttConUserInfo, Cfg),  &Cfg, 2); 
  
  _ReloadUInfo(pMqttConUser);
}

//----------------------------Info存储信息查找表--------------------------------
static const unsigned short _InfoBase[] = {
  0,                      //struct_offset(struct _MqttConUserInfo, UserName),//用户名
  MQTT_CON_USER_NAME_LEN, //struct_offset(struct _MqttConUserInfo, UserPass),//用户密码 
  MQTT_CON_USER_PASS_LEN, //struct_offset(struct _MqttConUserInfo, Info),    //相关信息   
};

static const unsigned char _InfoLen[] = {
  MQTT_CON_USER_NAME_LEN,     //用户名
  MQTT_CON_USER_PASS_LEN,     //用户密码 
  MQTT_CON_USER_INFO_LEN,    //相关信息   
};

//----------------------------得到相关Info信息----------------------------------
void MqttConUser_GetInfo(const struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf)
{
  //直接从EEPROM中读取防止读取到中间值
  Eeprom_Rd(MqttConUser_GetInfoBase(pMqttConUser->AryId) +  _InfoBase[Type], 
            pBuf, _InfoLen[Type]);
}

//----------------------------设置相关Info信息----------------------------------
//此为GUI接口，TYPE定义同MqttConUser_GetInfo: 
void MqttConUser_SetInfo(struct _MqttConUser *pMqttConUser,
                         unsigned char Type, char *pBuf)
{
  //错误检查
  if(Type > 3) return;
  unsigned char Len = strlen(pBuf) + 1; //含结束字符
  if(Len > _InfoLen[Type]) Len = _InfoLen[Type];//超限
  //保存
  Eeprom_Wr(MqttConUser_GetInfoBase(pMqttConUser->AryId) + 
            _InfoBase[Type], pBuf, Len);

  _ReloadUInfo(pMqttConUser);
}








