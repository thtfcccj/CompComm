/******************************************************************************

                ModbusRtuMng-QMenu参数接口实现

*******************************************************************************/

#include "ModbusRtuMngQMenu.h"
#include "QMenuPara.h"
#include "LedSign.h"

#include "ModbusRtuMng.h"

//---------------------------通讯位配置----------------------------------
//位定义见UsartDevCfg.h
static void _GetBitCfg(struct _QMenuPara *pPara, unsigned char Type)
{
  pPara->Cfg = QMENU_CFG_ADJ_LOGIC;
  pPara->Adj = ModbusRtuMng_GetCommCfg(); 
  pPara->Max = 0xff;//允许调整位
}
static void _SetBitCfg(struct _QMenuPara *pPara, unsigned char Type)
{
  ModbusRtuMng_SetCommCfg(pPara->Adj);
}

//主结结构
const struct _QMenuFun ModbusRtuMngQMenu_BitCfg = {
  {LED_SIGN_G,LED_SIGN_F,LED_SIGN_C, LED_SIGN_3 | LED_SIGN_DOT,}, //3.CFG(原兼容)
  _GetBitCfg, _SetBitCfg,
};

//----------------------------读写通讯地址---------------------------------
static void _GetAdr(struct _QMenuPara *pPara, unsigned char Type)
{
  pPara->Cfg = QMENU_CFG_ADJ_BIT;
  pPara->Adj = ModbusRtuMng_GetAdr();
  pPara->Min = 1;
  pPara->Max = 2;
}
static void _SetAdr(struct _QMenuPara *pPara, unsigned char Type)
{
  ModbusRtuMng_SetAdr(pPara->Adj);
}

//主结结构
const struct _QMenuFun ModbusRtuMngQMenu_Adr = {
  {LED_SIGN_R, LED_SIGN_D, LED_SIGN_A, LED_SIGN_3 | LED_SIGN_DOT,}, //3.ADR
  _GetAdr, _SetAdr,
};



