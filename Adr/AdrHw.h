/**************************************************************

                 通讯地址之硬件拔码开关获得模块

***************************************************************/
#ifndef _ADR_HW_H
#define _ADR_HW_H

/**************************************************************
                        相关配置
***************************************************************/

//支持硬件拔码，其它模块可能需要时在全局里定义,否则不需要包含此模块
//#define   SUPPORT_ADR_HW

//地址采样次数
#ifndef ADR_HW_CHECK_COUNT
  #define   ADR_HW_CHECK_COUNT      3
#endif

//拔码位数
#ifndef ADR_HW_BIT_COUNT
  #define  ADR_HW_BIT_COUNT      8
#endif

//位掩码位数，与IO位置有关
#ifndef AdrHw_BitMask_t
  #define  AdrHw_BitMask_t   unsigned char 
#endif

/**************************************************************
                        相关结构
***************************************************************/
struct _AdrHw{
  unsigned char Timer;//地址采样计数
  unsigned char Adr;//地址
};

extern struct _AdrHw AdrHw;

/**************************************************************
                        相关函数
***************************************************************/
//-------------------上电地址检测---------------------
void AdrHw_Init(void);

//-----------------------地址更新任务/512mS-----------------------
//3次采样都相同,地址更新
void AdrHw_Task(void);

//-----------------------得到设定的地址----------------------
#define  AdrHw_GetAdr() (AdrHw.Adr)

/********************************************************************
                        回调函数
*********************************************************************/

//------------------------------IO初始化---------------------------
#include "IoCtrl.h"
#define AdrHw_cbIoCfg()   CfgAdrHwIo();

//--------------------------得到地址函数-------------------------
unsigned char AdrHw_cbGetAdr(void);

//------------------------地址更新通报---------------------------
//3次采样都相同,地址更新
//void AdrHw_cbNotify(unsigned char Adr);
#ifdef SUPPORT_SW_ADR //支持软件地址时
  #include "Adr.h" 
  #define AdrHw_cbNotify(adr) do{Adr_SetHwAdr(adr);}while(0)
#else//直接实现:
  void AdrHw_cbNotify(unsigned char Adr);
#endif


#endif
