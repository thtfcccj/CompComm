/******************************************************************************

                ModbusRtuMng-QMenu�����ӿ�ʵ��

*******************************************************************************/

#include "ModbusRtuMngQMenu.h"
#include "QMenuPara.h"
#include "LedSign.h"

#include "ModbusRtuMng.h"

//---------------------------ͨѶλ����----------------------------------
//λ�����UsartDevCfg.h
static void _GetBitCfg(struct _QMenuPara *pPara, unsigned char Type)
{
  pPara->Cfg = QMENU_CFG_ADJ_LOGIC;
  pPara->Adj = ModbusRtuMng_GetCommCfg(); 
  pPara->Max = 0xff;//�������λ
}
static void _SetBitCfg(struct _QMenuPara *pPara, unsigned char Type)
{
  ModbusRtuMng_SetCommCfg(pPara->Adj);
}

//����ṹ
const struct _QMenuFun ModbusRtuMngQMenu_BitCfg = {
  {LED_SIGN_G,LED_SIGN_F,LED_SIGN_C, LED_SIGN_3 | LED_SIGN_DOT,}, //3.CFG(ԭ����)
  _GetBitCfg, _SetBitCfg,
};

//----------------------------��дͨѶ��ַ---------------------------------
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

//����ṹ
const struct _QMenuFun ModbusRtuMngQMenu_Adr = {
  {LED_SIGN_R, LED_SIGN_D, LED_SIGN_A, LED_SIGN_3 | LED_SIGN_DOT,}, //3.ADR
  _GetAdr, _SetAdr,
};



