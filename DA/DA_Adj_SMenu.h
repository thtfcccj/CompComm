/******************************************************************************

         DA_Adj模块-SMenu参数接口

*******************************************************************************/
#ifndef _DA_ADJ_SMENU_H
#define _DA_ADJ_SMENU_H
#ifdef SUPPORT_EX_PREINCLUDE//不支持Preinlude時
  #include "Preinclude.h"
#endif

#include "SMenuUser.h"

/******************************************************************************
                         SMenu参数接口
*******************************************************************************/

//-----------------------------4-20mA零点标定--------------------------------
extern const struct _SMenuFun SMenu_DA_Adj_Zero;

//-----------------------------4-20mA满量程标定--------------------------------
extern const struct _SMenuFun SMenu_DA_Adj_Full;

/******************************************************************************
                         回调函数接口
*******************************************************************************/

//零点标定提示
#ifndef DA_ADJ_SMENU_NOTE_ZERO
  #define DA_ADJ_SMENU_NOTE_ZERO {LED_SIGN_4, LED_SIGN__,LED_SIGN__,LED_SIGN_I} //I--4
#endif
  
//满点标定提示
#ifndef DA_ADJ_SMENU_NOTE_FULL
  #define DA_ADJ_SMENU_NOTE_FULL {LED_SIGN_0, LED_SIGN_2,LED_SIGN__,LED_SIGN_I} //I-20
#endif
  
#endif



