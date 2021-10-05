/******************************************************************************

         DA_Adjģ��-SMenu�����ӿ�

*******************************************************************************/
#ifndef _DA_ADJ_SMENU_H
#define _DA_ADJ_SMENU_H
#ifdef SUPPORT_EX_PREINCLUDE//��֧��Preinlude�r
  #include "Preinclude.h"
#endif

#include "SMenuUser.h"

/******************************************************************************
                         SMenu�����ӿ�
*******************************************************************************/

//-----------------------------4-20mA���궨--------------------------------
extern const struct _SMenuFun SMenu_DA_Adj_Zero;

//-----------------------------4-20mA�����̱궨--------------------------------
extern const struct _SMenuFun SMenu_DA_Adj_Full;

/******************************************************************************
                         �ص������ӿ�
*******************************************************************************/

//���궨��ʾ
#ifndef DA_ADJ_SMENU_NOTE_ZERO
  #define DA_ADJ_SMENU_NOTE_ZERO {LED_SIGN_Z, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT} //4.20.Z
#endif
  
//����궨��ʾ
#ifndef DA_ADJ_SMENU_NOTE_FULL
  #define DA_ADJ_SMENU_NOTE_FULL   {LED_SIGN_F, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT} //4.20.F
#endif
  
#endif



