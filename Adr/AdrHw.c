/**************************************************************

                通讯地址之由硬件拔码开关获得实现

***************************************************************/
#include "AdrHw.h"

struct _AdrHw AdrHw;

//-------------------上电地址检测---------------------
void AdrHw_Init(void)
{
  AdrHw_cbIoCfg();
  AdrHw.Adr = 0;
  AdrHw.Timer = ADR_HW_CHECK_COUNT;
  while(AdrHw.Timer) AdrHw_Task();
}
  
//-----------------------地址更新任务/512mS-----------------------
//3次采样都相同,地址更新
void AdrHw_Task(void)
{
  unsigned char CurAdr = AdrHw_cbGetAdr();//采样地址
  if(CurAdr != AdrHw.Adr){
    AdrHw.Adr = CurAdr;
    AdrHw.Timer = ADR_HW_CHECK_COUNT;
    return;
  }
  else if(!AdrHw.Timer) return;
  
  AdrHw.Timer--;
  if(!AdrHw.Timer){ 
    if(CurAdr)//0地址无效
      AdrHw_cbNotify(CurAdr);//地址更新通报
  }
}
