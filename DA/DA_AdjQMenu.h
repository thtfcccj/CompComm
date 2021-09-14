/******************************************************************************

         DA_Adjģ��-QMenu�����ӿ�

*******************************************************************************/
#ifndef _DA_ADJ_QMENU_H
#define _DA_ADJ_QMENU_H
#ifdef SUPPORT_EX_PREINCLUDE//��֧��Preinlude�r
  #include "Preinclude.h"
#endif

#include "QMenuPara.h"

/******************************************************************************
                         QMenu�����ӿ�
*******************************************************************************/

//-----------------------------���궨--------------------------------
extern const struct _QMenuFun DA_AdjQMenu_Zero;

//-----------------------------�����̱궨--------------------------------
extern const struct _QMenuFun DA_AdjQMenu_Full;

/******************************************************************************
                         �ص������ӿ�
*******************************************************************************/

//���궨��ʾ
#ifndef DA_ADJ_QMENU_NOTE_ZERO
  #define DA_ADJ_QMENU_NOTE_ZERO {LED_SIGN_Z, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT} //4.20.Z
#endif
  
//����궨��ʾ
#ifndef DA_ADJ_QMENU_NOTE_FULL
  #define DA_ADJ_QMENU_NOTE_FULL   {LED_SIGN_F, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT} //4.20.F
#endif
  
#endif



