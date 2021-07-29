/*******************************************************************************
                   由PWM产生模拟量输出模块在PIC单片机中的实现
DA PWM基本原理-10位模拟12位（没有定义非中断时）:
  (1)计数器以一定频率计数
  (2)以满量程作为计数器的终点,到时自动复位重新开始
  (3)以当前浓度值作为匹配值,当计数器计数到此位置时,发生电平改变
  (3)外部以适当的RC滤波后,产生与浓度对应的浓度值
此模块不需要开放中断,可调用接口函数完成更新

//DA PWM PIC16原理:
使用16位TCNT1持续与OCR1x的内容比较，一旦发现它们相等，
比较器立即产生一个匹配信号;本程序采用模式14(快速pwm),ICR1中存放top值;
一个周期中,定时计数器TCNT1从0计数到匹配值时(OCR1A),将输出OC1x清零,
当计数到TOP值时(ICR1),将输出OC1x置位,并重新计数;
本程序不采用中断方式,而采用0.25S更新一次匹配寄存器数据的方式;

注意：与量程相关时，量程的最大值不能超过2000
*******************************************************************************/

#include "DA.h"
#include "IOCtrl.h"

/*******************************************************************************
                            附加配置说明
*******************************************************************************/
//#define SUPPORT_DA_ADJ_DIS_INT  默认启用中断时支持12位精度DA,禁用中断时为10位



/*******************************************************************************
                            相关函数实现
*******************************************************************************/

//---------------该模块DA为12位即4096----------------------------------
#ifndef SUPPORT_DA_ADJ_DIS_INT
  #define _PIC_CIRCLE 4096
  //硬件上只是10位PWM，要模拟出12位DA,12位DA周期4096对应10位DA1024，就是4个周期。
  unsigned short _Vol;//DA值,范围是4096
  unsigned short _Cur;//当前DA值
#endif
  
//-----------------------------初始化函数---------------------------------
void DA_Init(void)
{
   CfgDA();
   CCP2CON  = PICB_CCP2M2 | PICB_CCP2M3;//PWM模式，高电平有效。占空比高两位
   CCPTMRS0 = 0xf3; //选择定时器2为CCP2的PWM定时器
   CCPR2L = 0xf0;//PWM初始占空比值（此处给出一个随机值，调试程序时再改）
   PIR1 &= ~(PICB_TMR2IF); //清定时器2中断
   #ifdef SUPPORT_DA_ADJ_DIS_INT
     PIE1 &= ~PICB_TMR2IE ; //定时器2中断禁止
     T2CON = PICB_T2OUTPS0 | PICB_T2OUTPS1 | PICB_TMR2ON;//最高频率1/4
   #else
     //PIE1 |= PICB_TMR2IE ; //定时器2中断允许
     //初始化内部变量
     _Cur = 0;
     _Vol = 0;
     //InDA();//禁止PWM输出
     T2CON = PICB_TMR2ON;//最高频率，即32M/4 = 8M
   #endif
   PR2 = 0xff;//PWM周期设定
   CfgDA();//PWM对应引脚设置为输出
   
   #ifdef SUPPORT_LOW_POWER //两线制时,输出0保持直到上电完成以防止启动电流过大
     for(unsigned long i = 655350; i > 0; i--){
       WDT_Week();//喂狗一次
     }  
   #endif
   
   #ifndef SUPPORT_DA_ADJ_DIS_INT
     PIE1 |= PICB_TMR2IE ; //最后定时器2中断允许
   #endif
}

//----------------------------输出DA值-----------------------------------
void DA_SetDA(unsigned short DA)
{
  #ifdef SUPPORT_DA_ADJ_DIS_INT
    _Vol = DA;
    //装入占空比，周期固定在初始化已设好
    CCPR2L = (unsigned char)(DA >> 2);//先放高8位
    //再或上该位的值
    CCP2CON &= ~(PICB_DC2B1 | PICB_DC2B0);
    CCP2CON |= (((unsigned char) (DA & 0x0003)) << 4);
  #else
    unsigned long Data = (unsigned long)4096 << 12;
    Data /= DA_FULL;
    Data *= DA;
    _Vol = (unsigned short)(Data >> 12);
  #endif
}

//-----------------------定时器2中断函数------------------------------------
#ifndef SUPPORT_DA_ADJ_DIS_INT
void DA_PWM_TimerOvInt(void)
{
  //当前周期记完成了,准备下个周期
  if(_Cur >= (4096 - 1024)) _Cur = 0;
  else _Cur += 1024; 
  
  //准备装载值
  unsigned short LoadVol;
  if(_Cur <= _Vol){
    LoadVol = _Vol - _Cur;
    if(LoadVol >= 1023) LoadVol = 1023;
  }
  else LoadVol = 0;

  //装入占空比，周期固定在初始化已设好
  CCPR2L = (unsigned char)(LoadVol >> 2);//先放高8位
  //再或上该位的值
  CCP2CON &= ~(PICB_DC2B1 | PICB_DC2B0);
  CCP2CON |= (((unsigned char) (LoadVol & 0x0003)) << 4);
  PIR1 &= ~PICB_TMR2IF;     //最后清中断
}
#endif //#ifdef SUPPORT_DA_ADJ_DIS_INT                                                                                  




