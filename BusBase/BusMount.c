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
}

//----------------------------------挂接-------------------------------
//总线创建后调用
void BusMount_Mount(unsigned char BusId, struct _BusCount *pBase)
{
  if(BusId >= BUS_ID_COUNT) return;
  BusMount.pBusMount[BusId] = pBase;
}

//--------------------------------得到结构------------------------------
//若由struct _BusCount派生，则可直接转换为其结构
//返回NULL未挂接或不支持
struct _BusCount *BusMount_GetBase(unsigned char BusId)
{
  if(BusId >= BUS_ID_COUNT) return NULL;
  return BusMount.pBusMount[BusId];  
}

