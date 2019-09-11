/**************************************************************

    通讯地址之由8位硬件拔码开关获得模块回调函数-在STM32F1x中的实现

***************************************************************/
#include "AdrHw.h"
#include "IOCtrl.h"

//静态结构定义
struct _PinStatic{
  unsigned char Port;           //所在端口，对应ABCD
  AdrHw_BitMask_t PinMask;		   //位置屏敝码
};

static const  __flash struct _PinStatic _Static[8]={
  ADR_CFG_BIT0,
  ADR_CFG_BIT1,
  ADR_CFG_BIT2,
  ADR_CFG_BIT3,
  ADR_CFG_BIT4,
  ADR_CFG_BIT5,
  ADR_CFG_BIT6,
  ADR_CFG_BIT7
};

//--------------------------得到地址函数-------------------------
unsigned char AdrHw_cbGetAdr(void)
{
  unsigned char Adr = 0;
  unsigned char Shift = 0x01;
  for(unsigned char Bit = 0; Bit < ADR_HW_BIT_COUNT; Bit++){
    unsigned char Pin = 0;
    unsigned char PinMask = _Static[Bit].PinMask;
    switch(_Static[Bit].Port){
      case 0: if(GPIOA->IDR & PinMask) Pin = Shift; break;
      case 1: if(GPIOB->IDR & PinMask) Pin = Shift; break;
      case 2: if(GPIOC->IDR & PinMask) Pin = Shift; break;
      default: 
        break;
    }
    Adr |= Pin;
    Shift <<= 1;
  }
  Adr = ~Adr;
  return Adr;
}
