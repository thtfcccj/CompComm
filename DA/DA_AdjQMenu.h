/******************************************************************************

         DA_Adj模块-QMenu参数接口

*******************************************************************************/
#ifndef _DA_ADJ_QMENU_H
#define _DA_ADJ_QMENU_H
#ifdef SUPPORT_EX_PREINCLUDE//不支持Preinlude時
  #include "Preinclude.h"
#endif

#include "QMenuPara.h"

/******************************************************************************
                         QMenu参数接口
*******************************************************************************/

//-----------------------------零点标定--------------------------------
extern const struct _QMenuFun DA_AdjQMenu_Zero;

//-----------------------------满量程标定--------------------------------
extern const struct _QMenuFun DA_AdjQMenu_Full;

/******************************************************************************
                         回调函数接口
*******************************************************************************/

//零点标定提示
#ifndef DA_ADJ_QMENU_NOTE_ZERO
  #define DA_ADJ_QMENU_NOTE_ZERO {LED_SIGN_Z, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT} //4.20.Z
#endif
  
//满点标定提示
#ifndef DA_ADJ_QMENU_NOTE_FULL
  #define DA_ADJ_QMENU_NOTE_FULL   {LED_SIGN_F, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT} //4.20.F
#endif
  
#endif



