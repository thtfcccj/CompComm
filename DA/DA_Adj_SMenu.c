/******************************************************************************

                DA_Adj-SMenu�����ӿ�ʵ��

*******************************************************************************/

#include "DA_Adj_SMenu.h"
#include "LedSign.h"

#include "Eeprom.h"
#include "InfoBase.h"
#include "DA_Adj.h"

//-----------------------------���궨--------------------------------
static void _GetZero(struct _SMenuUser *pUser, unsigned char Type)
{
  pUser->Cfg = SMENU_USR_WR | SMENU_USR_REAL_WR | SMENU_USR_ADJ_ALL;
  pUser->Cfg2 = SMENU_USR2_QUIT_SAVE | SMENU_USR2_NEGATIVE;
  signed short Offset = (signed short)DA_Adj_GetZero() - DA_ADJ_ZERO_DEFAULT;
  pUser->Adj = Offset >> 4;
  pUser->Min = -9999;
  pUser->Max = 9999;
  if(Type == SMENU_LAYER_WR)
    DA_Adj_SetZeroMode(); //��������ģʽ
}
static void _SetZero(struct _SMenuUser *pUser, unsigned char Type)
{
  if(Type == SMENU_USR_REAL_WR) 
    DA_Adj_SetZero((pUser->Adj << 4) + DA_ADJ_ZERO_DEFAULT); //ʵʱдʱ�����
  else{
   DA_Adj_SaveZero();
   DA_Adj_ClrZeroMode();//�����˳�ģʽ
  }
}

//����ṹ
const struct _SMenuFun SMenu_DA_Adj_Zero = {
  DA_ADJ_SMENU_NOTE_ZERO, _GetZero, _SetZero,
};

//-----------------------------�����̱궨--------------------------------
static void _GetFull(struct _SMenuUser *pUser, unsigned char Type)
{
  pUser->Cfg = SMENU_USR_WR | SMENU_USR_REAL_WR | SMENU_USR_ADJ_ALL;
  pUser->Cfg2 = SMENU_USR2_QUIT_SAVE | SMENU_USR2_NEGATIVE;
  signed short Offset = (unsigned short)DA_Adj_GetGain() - DA_ADJ_GAIN_DEFAULT;
  pUser->Adj = Offset >> 1;
  pUser->Min = -9999;
  pUser->Max = 9999;
  if(Type == SMENU_LAYER_WR)
    DA_Adj_SetGainMode(); //��������ģʽ
}
static void _SetFull(struct _SMenuUser *pUser, unsigned char Type)
{
  if(Type == SMENU_USR_REAL_WR) 
    DA_Adj_SetGain((pUser->Adj << 1) + DA_ADJ_GAIN_DEFAULT); //ʵʱдʱ�����
  else{//�˳��򰴱����ʱ����
   DA_Adj_SaveGain();
   DA_Adj_ClrGainMode();//�����˳�ģʽ
  }
}

//����ṹ
const struct _SMenuFun SMenu_DA_Adj_Full = {
  DA_ADJ_SMENU_NOTE_FULL, _GetFull, _SetFull,
};
