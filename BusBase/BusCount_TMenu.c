/*******************************************************************************

                     ComCount结构菜单实现
此菜单数据为只读
*******************************************************************************/

#include "BusCount_TMenu.h"

#include "TMenu_EditSel.h"
#include "stringEx.h"
#include <string.h>
#include "TMenuBuf.h"

//--------------------------------内部字符资源--------------------------------- 
#ifdef TM_EN_MUTI_LAN            //常量选择型多国语言时
  //暂不支持常量选择型字符串
#elif defined(TM_RESOURCE_LAN)  //由资源文件获取多国语言时
  //菜单
  #include "lsAry.h"
#else  //单一语言字符串时
  //菜单
  static const char ls_PacketCount[]   = {"通讯次数"};
  static const char ls_PacketValid[]   = {"有效数据"};
  static const char ls_PacketInvalid[] = {"无效数据"};
  static const char ls_PacketQuality[] = {"报文统计"};
#endif

const LanCode_t* const lsAry_Packet[] = {
  ls_PacketCount, ls_PacketValid, ls_PacketInvalid};
  
//----------------------------回调函数实现--------------------------------------
//枚举型调整方式
static void _Notify(unsigned char Type, void *pv)
{
  struct _EditSelUser *pUser = (struct _EditSelUser *)pv;
  switch(Type){
  //得到子菜单头
  case TM_NOTIFY_GET_SUB_HEADER:
    strcpy((char*)pv, pLanCodeToChar(lsAry_Packet[*((unsigned char*)pv)]));
    break;
  case TM_NOTIFY_GET_DATA:{ //将当前值装入
    unsigned long Data;
    const struct _BusCount *pBus = BusCountTMenu_pcbGet(); 
    if(pUser->CurItem == 0) Data = pBus->Comm;
    else if(pUser->CurItem == 1) Data = pBus->Valid;  
    else  Data = pBus->Invalid; 
    pUser->pData = Value4StringMin(Data, TMenuBuf.Str, 8);
    break;
  }
  default:break;
  }
}

//---------------------------------菜单结构----------------------------------
const TMenu_t BusCount_TMenu = {//菜单结构
  TMTYPE_EDITSEL,          //菜单类型为编辑选择模式模式与用户区标志,带查找表
  3,                       //可供选择的调整项个数
  ls_PacketQuality,        //菜单头,为NULL时从回调里读取
  BUS_COUNT_TMENU_PARENT,  //自已的父菜单
  NULL,                    //存放自已的子菜单阵列连接头,只读时可为struct _TMenuSelRO
  _Notify,                 //与用户空间交互的通报函数
};


