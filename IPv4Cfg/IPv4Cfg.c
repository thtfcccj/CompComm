/*******************************************************************************

					            IPv4地址(含端口)定义与配置保存

********************************************************************************/
#include "IPv4Cfg.h" 
#include "Eeprom.h" 
#include "InfoBase.h" 
#include <string.h>

struct _IPv4Cfg IPv4Cfg;


//定义IPv4结构:
static const struct _IpInfo _DefaultInfo = {
  {0, 0, 0, 212},  //4位IP地址
  10001,           //端口号
};

/****************************************************************************
                              相关函数实现
****************************************************************************/
//--------------------------------初始化函数------------------------------
void IPv4Cfg_Init(signed char IsInited)
{
  memset(&IPv4Cfg, 0, sizeof(struct _IPv4Cfg));
  
  //内部变量初始化  
  if(!IsInited){//装载默认
    for(unsigned char i = 0; i < IPV4_CFG_COUNT; i++){
      memcpy(&IPv4Cfg.Id[i], &_DefaultInfo, sizeof(struct _IpInfo));
    }
    Eeprom_Wr(IPv4Cfg_GetInfoBase(0),
              &IPv4Cfg,
              sizeof(struct _IPv4Cfg));
  }
  else{
    Eeprom_Rd(IPv4Cfg_GetInfoBase(0),
                &IPv4Cfg,
                sizeof(struct _IPv4Cfg));
  }
}

//--------------------------------设置Id对应IP-----------------------------
void IPv4Cfg_SetIp(unsigned char Id, const unsigned char *pIpv4)
{
  memcpy(IPv4Cfg.Id[Id].Ip, pIpv4, 4);
  Eeprom_Wr(IPv4Cfg_GetInfoBase(Id), &IPv4Cfg.Id, sizeof(struct _IpInfo));
}


//--------------------------------设置Id对应IP对应位-----------------------
void IPv4Cfg_SetIpPos(unsigned char Id, unsigned char Pos,
                        unsigned char PosAdr)
{
  IPv4Cfg.Id[Id].Ip[Pos] = PosAdr;
  Eeprom_Wr(IPv4Cfg_GetInfoBase(Id), &IPv4Cfg.Id, sizeof(struct _IpInfo));
}


//--------------------------------设置Id对应端口---------------------------
void IPv4Cfg_SetPort(unsigned char Id, unsigned short Port)
{
  IPv4Cfg.Id[Id].Port = Port;
  Eeprom_Wr(IPv4Cfg_GetInfoBase(Id), &IPv4Cfg.Id, sizeof(struct _IpInfo));
}


