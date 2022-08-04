/*****************************************************************************

   BusSlvUsart�ص�����-��ʹ��BusId�붨����USART_DEV_CFG_USERʱ�Ļص�ʵ��

******************************************************************************/  

#include "BusSlvUsart.h"
#include "BusId.h"
#include "Usart.h"      //ͨѶ
//ͨѶ���ýṹ����չ #define USART_DEV_CFG_USER UsartDevCfgUser0_t U;
#include "UsartDevCfg.h" 

//----------------------------������ID��ȡ�豸----------------------------
struct _UsartDev *BusSlvUsart_cbGetDev(unsigned char BusId)
{
  return Usart_GetDev(BusId_GetSubId(BusId));
}

//----------------------------������ID��ȡ��Ӧ�ӻ���ַ--------------------
unsigned char BusSlvUsart_cbGetAdr(unsigned char BusId)
{
  return UsartDevCfg[BusId_GetSubId(BusId)].U.S.Adr;
}

//----------------------------------������ID����RTC--------------------------
void BusSlvUsart_cbRTS(unsigned char BusId, unsigned char IsSend)
{
  BusId_CtrlUsartRTS(BusId, IsSend);
}
  
//----------------------------������ID��֡���--------------------------
unsigned char BusSlvUsart_cbGetSpaceT(unsigned char BusId)
{
  return UsartDevCfg[BusId_GetSubId(BusId)].U.S.SpaceT;
}