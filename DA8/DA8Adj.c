/*******************************************************************************

       低精度DA输出模块接口,在ASM32单片机PCA中（由PWM产生）实现

*******************************************************************************/
#include "DA8Adj.h"
#include "DA8.h"

#include "Eeprom.h"
#include "InfoBase.h"

struct _DA8_Adj DA8_Adj;  //直接实例化

//-----------------------------初始化函数---------------------------------
void DA8Adj_Init(signed char IsInited)
{
  if(!IsInited){
    DA8_Adj.Info.DA = DA8_ADJ_DEFAULT_DA;
    Eeprom_Wr(DA8Adj_GetInfoBase(), &DA8_Adj.Info.DA, 1);
  }
  else{
    Eeprom_Rd(DA8Adj_GetInfoBase(), &DA8_Adj.Info.DA, 1);
  }
  DA8_Init();    
  DA8_SetDA(DA8_Adj.Info.DA);
}

//---------------------------保存输出位置-----------------------------
void DA8Adj_SavePos(void)
{
  DA8_Adj.Info.DA = DA8_GetDA();  
  Eeprom_Wr(DA8Adj_GetInfoBase(), &DA8_Adj.Info.DA, 1);
}

//---------------------------恢复输出位置-----------------------------
void DA8Adj_RestorePos(void)
{
  DA8_SetDA(DA8_Adj.Info.DA);  
}





