/*******************************************************************************

       由PWM产生模拟量输出模块在ASM32单片机通用定时器(TIM2)中的实现

*******************************************************************************/

#include "DA.h"
#include "IOCtrl.h"

//-----------------------------初始化函数---------------------------------
void DA_Init(void)
{
  CfgDA();//IO与时钟允许
  DA_TIM->PSC = 0;          //不分频以尽可能最大速率输出
  DA_TIM->ARR = 0xFFF;       //设置 PWM周期值 周期值 ARR  ->实际使用12位以增加频率 
  DA_CfgPWMOut();   //外部实现
  DA_TIM->CR1 |= TIM_CR1_CEN | TIM_CR1_ARPE;    //开启定时器并启用预装载
}

//----------------------------输出DA值-----------------------------------
void DA_SetDA(unsigned short DA)
{
  DA_TIM->DA_CCR = DA >> 4; //16位转换为实际的12位
}




