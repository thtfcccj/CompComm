/**************************************************************

    ͨѶ��ַ֮��8λӲ�����뿪�ػ��ģ��ص�����-��STM32F1x�е�ʵ��

***************************************************************/
#include "AdrHw.h"
#include "IOCtrl.h"

//��̬�ṹ����
struct _PinStatic{
  unsigned char Port;           //���ڶ˿ڣ���ӦABCD
  AdrHw_BitMask_t PinMask;		   //λ��������
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

//--------------------------�õ���ַ����-------------------------
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
