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
  #define DA_ADJ_SMENU_NOTE_ZERO {LED_SIGN_4, LED_SIGN__,LED_SIGN__,LED_SIGN_I} //I--4
#endif
  
//����궨��ʾ
#ifndef DA_ADJ_SMENU_NOTE_FULL
  #define DA_ADJ_SMENU_NOTE_FULL {LED_SIGN_0, LED_SIGN_2,LED_SIGN__,LED_SIGN_I} //I-20
#endif
  
#endif



