/******************************************************************************

                DA_Adj-QMenu参数接口实现

*******************************************************************************/

#include "DA_AdjQMenu.h"
#include "QMenuPara.h"
#include "LedSign.h"

#include "Eeprom.h"
#include "InfoBase.h"
#include "DA_Adj.h"

//-----------------------------零点标定--------------------------------
static void _GetZero(struct _QMenuPara *pPara, unsigned char Type)
{
  pPara->Cfg = QMENU_CFG_WR | QMENU_CFG_REAL_WR | QMENU_CFG_ADJ_ALL;
  pPara->Cfg2 = QMENU_CFG2_QUIT_SAVE | QMENU_CFG2_NEGATIVE;
  signed short Offset = (signed short)DA_Adj_GetZero() - DA_ADJ_ZERO_DEFAULT;
  pPara->Adj = Offset >> 4;
  pPara->Min = -9999;
  pPara->Max = 9999;
  if(Type == QMENU_LAYER_WR)
    DA_Adj_SetZeroMode(); //立即进入模式
}
static void _SetZero(struct _QMenuPara *pPara, unsigned char Type)
{
  if(Type == QMENU_CFG_REAL_WR) 
    DA_Adj_SetZero((pPara->Adj << 4) + DA_ADJ_ZERO_DEFAULT); //实时写时置输出
  else{
   DA_Adj_SaveZero();
   DA_Adj_ClrZeroMode();//主动退出模式
  }
}

//主结结构
const struct _QMenuFun DA_AdjQMenu_Zero = {
  {LED_SIGN_Z, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT}, //4.20.Z(原兼容)
  _GetZero, _SetZero,
};

//-----------------------------满量程标定--------------------------------
static void _GetFull(struct _QMenuPara *pPara, unsigned char Type)
{
  pPara->Cfg = QMENU_CFG_WR | QMENU_CFG_REAL_WR | QMENU_CFG_ADJ_ALL;
  pPara->Cfg2 = QMENU_CFG2_QUIT_SAVE | QMENU_CFG2_NEGATIVE;
  signed short Offset = (unsigned short)DA_Adj_GetGain() - DA_ADJ_GAIN_DEFAULT;
  pPara->Adj = Offset >> 1;
  pPara->Min = -9999;
  pPara->Max = 9999;
  if(Type == QMENU_LAYER_WR)
    DA_Adj_SetGainMode(); //立即进入模式
}
static void _SetFull(struct _QMenuPara *pPara, unsigned char Type)
{
  if(Type == QMENU_CFG_REAL_WR) 
    DA_Adj_SetGain((pPara->Adj << 1) + DA_ADJ_GAIN_DEFAULT); //实时写时置输出
  else{//退出或按保存键时保存
   DA_Adj_SaveGain();
   DA_Adj_ClrGainMode();//主动退出模式
  }
}

//主结结构
const struct _QMenuFun DA_AdjQMenu_Full = {
  {LED_SIGN_F, LED_SIGN_0 | LED_SIGN_DOT,LED_SIGN_2,LED_SIGN_4 | LED_SIGN_DOT}, //4.20.F(原兼容)
  _GetFull, _SetFull,
};
