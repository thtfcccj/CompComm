/*******************************************************************************

                         STM32 DA的直接实现

*******************************************************************************/

#include "DA.h"
#include "IOCtrl.h"


//-----------------------------初始化函数---------------------------------
//注:调用此模块后应立即调用DA_SetDA()实现零点输出
void DA_Init(void)
{
  RCC->APB1ENR |= RCC_APB1RSTR_DACRST;
  DAC->CR |= ((1<<2)|(0x07<<3)|1<<0|1<<1);
  CfgDA();
}

//----------------------------输出DA值-----------------------------------
void DA_SetDA(unsigned short DA)
{
  DAC->DHR12L1 = DA;
  DAC->SWTRIGR |= 1<<0;
}
