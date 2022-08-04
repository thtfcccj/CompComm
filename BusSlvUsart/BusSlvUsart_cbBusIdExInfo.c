/*****************************************************************************

   BusSlvUsart回调函数-在使用BusId与定义有USART_DEV_CFG_USER时的回调实现

******************************************************************************/  

#include "BusSlvUsart.h"
#include "BusId.h"
#include "Usart.h"      //通讯
//通讯配置结构需扩展 #define USART_DEV_CFG_USER UsartDevCfgUser0_t U;
#include "UsartDevCfg.h" 

//----------------------------由总线ID获取设备----------------------------
struct _UsartDev *BusSlvUsart_cbGetDev(unsigned char BusId)
{
  return Usart_GetDev(BusId_GetSubId(BusId));
}

//----------------------------由总线ID获取对应从机地址--------------------
unsigned char BusSlvUsart_cbGetAdr(unsigned char BusId)
{
  return UsartDevCfg[BusId_GetSubId(BusId)].U.S.Adr;
}

//----------------------------------由总线ID控制RTC--------------------------
void BusSlvUsart_cbRTS(unsigned char BusId, unsigned char IsSend)
{
  BusId_CtrlUsartRTS(BusId, IsSend);
}
  
//----------------------------由总线ID得帧间隔--------------------------
unsigned char BusSlvUsart_cbGetSpaceT(unsigned char BusId)
{
  return UsartDevCfg[BusId_GetSubId(BusId)].U.S.SpaceT;
}