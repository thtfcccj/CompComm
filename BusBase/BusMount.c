/***********************************************************************

                    Bus Id 对应的挂接实现

***********************************************************************/

#include "BusMount.h"
#include <string.h>

struct _BusMount BusMount; //直接单例化

/***********************************************************************
                           相关函数
***********************************************************************/

//------------------------------开机初始化-----------------------------
void BusMount_Init(void)
{
  memset(&BusMount, 0, sizeof(struct _BusMount));
  #ifdef SUPPORT_BUS_MOUNT_TYPE      //支持类型标识时
    //当前挂接的总线类型
    memset(&BusMount.Type, 0xff, BUS_ID_COUNT);    
  #endif
}

//----------------------------------挂接-------------------------------
#ifdef SUPPORT_BUS_MOUNT_TYPE      //支持类型标识时
  void BusMount_Mount(unsigned char BusId, 
                      unsigned char Type,   //挂接总线的类型
                      struct _BusCount *pBase)
  {
    if(BusId >= BUS_ID_COUNT) return;
    BusMount.Type[BusId] = Type;  
    BusMount.pBusMount[BusId] = pBase;
  }
#else
  void BusMount_Mount(unsigned char BusId, 
                      struct _BusCount *pBase)
  {
    if(BusId >= BUS_ID_COUNT) return;
    BusMount.pBusMount[BusId] = pBase;
  }
#endif

//--------------------------------得到结构------------------------------
//若由struct _BusCount派生，则可直接转换为其结构
//返回NULL未挂接或不支持
struct _BusCount *BusMount_GetBase(unsigned char BusId)
{
  if(BusId >= BUS_ID_COUNT) return NULL;
  return BusMount.pBusMount[BusId];  
}

