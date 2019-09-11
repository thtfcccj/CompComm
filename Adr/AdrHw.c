/**************************************************************

                ͨѶ��ַ֮��Ӳ�����뿪�ػ��ʵ��

***************************************************************/
#include "AdrHw.h"

struct _AdrHw AdrHw;

//-------------------�ϵ��ַ���---------------------
void AdrHw_Init(void)
{
  AdrHw_cbIoCfg();
  AdrHw.Adr = 0;
  AdrHw.Timer = ADR_HW_CHECK_COUNT;
  while(AdrHw.Timer) AdrHw_Task();
}
  
//-----------------------��ַ��������/512mS-----------------------
//3�β�������ͬ,��ַ����
void AdrHw_Task(void)
{
  unsigned char CurAdr = AdrHw_cbGetAdr();//������ַ
  if(CurAdr != AdrHw.Adr){
    AdrHw.Adr = CurAdr;
    AdrHw.Timer = ADR_HW_CHECK_COUNT;
    return;
  }
  else if(!AdrHw.Timer) return;
  
  AdrHw.Timer--;
  if(!AdrHw.Timer){ 
    if(CurAdr)//0��ַ��Ч
      AdrHw_cbNotify(CurAdr);//��ַ����ͨ��
  }
}
