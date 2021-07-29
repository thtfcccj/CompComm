/**************************************************************

    ͨѶ��ַ֮��8λӲ�����뿪�ػ��ģ��ص�����-��HC32F0�е�ʵ��

***************************************************************/
#include "AdrHw.h"
#include "IOCtrl.h"

//��̬�ṹ����
struct _PinStatic{
  unsigned char Port;           //���ڶ˿ڣ���ӦABCD
  AdrHw_BitMask_t PinMask;		   //λ��������
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

//--------------------------�õ���ַ����-------------------------
//0ΪPA,1ΪPB,2ΪPC,3ΪPD 4ΪPE,5ΪPF
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
    Adr &= ((1 << ADR_HW_BIT_COUNT) - 1); //����Ҫ��λ��λ
  #endif 
  return Adr;
}
