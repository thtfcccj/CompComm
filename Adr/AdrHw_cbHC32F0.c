/**************************************************************

    通讯地址之由8位硬件拔码开关获得模块回调函数-在HC32F0中的实现

***************************************************************/
#include "AdrHw.h"
#include "IOCtrl.h"

//静态结构定义
struct _PinStatic{
  unsigned char Port;           //所在端口，对应ABCD
  AdrHw_BitMask_t PinMask;		   //位置屏敝码
};

static const  struct _PinStatic _Static[ADR_HW_BIT_COUNT]={
  ADR_CFG_BIT0,
  #ifdef ADR_CFG_BIT1
    ADR_CFG_BIT1,
  #endif
  #ifdef ADR_CFG_BIT2
    ADR_CFG_BIT2,
  #endif
  #ifdef ADR_CFG_BIT3
    ADR_CFG_BIT3,
  #endif
  #ifdef ADR_CFG_BIT4
    ADR_CFG_BIT4,
  #endif
  #ifdef ADR_CFG_BIT5
    ADR_CFG_BIT5,
  #endif
  #ifdef ADR_CFG_BIT6
    ADR_CFG_BIT6,
  #endif
  #ifdef ADR_CFG_BIT7
    ADR_CFG_BIT7,
  #endif
};

//--------------------------得到地址函数-------------------------
//0为PA,1为PB,2为PC,3为PD 4为PE,5为PF
unsigned char AdrHw_cbGetAdr(void)
{
  unsigned char Adr = 0;
  unsigned char Shift = 0x01;
  for(unsigned char Bit = 0; Bit < ADR_HW_BIT_COUNT; Bit++){
    unsigned char Pin = 0;
    unsigned short PinMask = _Static[Bit].PinMask;
    switch(_Static[Bit].Port){
      case 0: if(M0P_GPIO->PAIN & PinMask) Pin = Shift; break;
      case 1: if(M0P_GPIO->PBIN & PinMask) Pin = Shift; break;
      case 2: if(M0P_GPIO->PCIN & PinMask) Pin = Shift; break;
      case 3: if(M0P_GPIO->PDIN & PinMask) Pin = Shift; break;
      case 4: if(M0P_GPIO->PEIN & PinMask) Pin = Shift; break;
      case 5: if(M0P_GPIO->PFIN & PinMask) Pin = Shift; break;      
      default: 
        break;
    }
    Adr |= Pin;
    Shift <<= 1;
  }
  Adr = ~Adr;
  #if ADR_HW_BIT_COUNT < 8
    Adr &= ((1 << ADR_HW_BIT_COUNT) - 1); //不需要的位复位
  #endif 
  return Adr;
}
