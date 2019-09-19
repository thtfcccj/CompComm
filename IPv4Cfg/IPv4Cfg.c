/*******************************************************************************

					            IPv4��ַ(���˿�)���������ñ���

********************************************************************************/
#include "IPv4Cfg.h" 
#include "Eeprom.h" 
#include "InfoBase.h" 
#include <string.h>

struct _IPv4Cfg IPv4Cfg;


//����IPv4�ṹ:
static const struct _IpInfo _DefaultInfo = {
  {0, 0, 0, 212},  //4λIP��ַ
  10001,           //�˿ں�
};

/****************************************************************************
                              ��غ���ʵ��
****************************************************************************/
//--------------------------------��ʼ������------------------------------
void IPv4Cfg_Init(signed char IsInited)
{
  memset(&IPv4Cfg, 0, sizeof(struct _IPv4Cfg));
  
  //�ڲ�������ʼ��  
  if(!IsInited){//װ��Ĭ��
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

//--------------------------------����Id��ӦIP-----------------------------
void IPv4Cfg_SetIp(unsigned char Id, const unsigned char *pIpv4)
{
  memcpy(IPv4Cfg.Id[Id].Ip, pIpv4, 4);
  Eeprom_Wr(IPv4Cfg_GetInfoBase(Id), &IPv4Cfg.Id, sizeof(struct _IpInfo));
}


//--------------------------------����Id��ӦIP��Ӧλ-----------------------
void IPv4Cfg_SetIpPos(unsigned char Id, unsigned char Pos,
                        unsigned char PosAdr)
{
  IPv4Cfg.Id[Id].Ip[Pos] = PosAdr;
  Eeprom_Wr(IPv4Cfg_GetInfoBase(Id), &IPv4Cfg.Id, sizeof(struct _IpInfo));
}


//--------------------------------����Id��Ӧ�˿�---------------------------
void IPv4Cfg_SetPort(unsigned char Id, unsigned short Port)
{
  IPv4Cfg.Id[Id].Port = Port;
  Eeprom_Wr(IPv4Cfg_GetInfoBase(Id), &IPv4Cfg.Id, sizeof(struct _IpInfo));
}


