/***********************************************************************

                    Bus Id 对应的挂接
此接口与实现独立于应用
***********************************************************************/
#ifndef __BUS_MOUNT_H
#define	__BUS_MOUNT_H

/***********************************************************************
                        相关配置
***********************************************************************/

//#define SUPPORT_BUS_MOUNT_TYPE      //支持总线类型标识时定义

/***********************************************************************
                        相关结构
***********************************************************************/
#include "BusIdDef.h" //总线ID及类型定义(应用相关)
#include "BusCount.h" //基类

//----------------------------主结构-----------------------------------
struct _BusMount{
  #ifdef SUPPORT_BUS_MOUNT_TYPE      //支持类型标识时
    //当前挂接的总线类型
    unsigned char Type[BUS_ID_COUNT];    
  #endif
  //与BusId对应的总线指针, 最终类结构必须派生至struct _BusCount
  struct _BusCount *pBusMount[BUS_ID_COUNT];
};

extern struct _BusMount BusMount; //直接单例化

/***********************************************************************
                           相关函数
***********************************************************************/

//------------------------------开机初始化-----------------------------
void BusMount_Init(void); 

//----------------------------------挂接-------------------------------
//总线创建后调用
#ifdef SUPPORT_BUS_MOUNT_TYPE      //支持类型标识时
  void BusMount_Mount(unsigned char BusId, 
                      unsigned char Type,   //挂接总线的类型
                      struct _BusCount *pBase); 
#else
  void BusMount_Mount(unsigned char BusId, 
                      struct _BusCount *pBase);   
#endif

//----------------------------得到挂接的总线类型-------------------------
#define BusMount_GetType(busId)   (BusMount.Type[busId])

//--------------------------------得到结构------------------------------
//若由struct _BusCount派生，则可直接转换为其结构
//返回NULL未挂接或不支持
struct _BusCount *BusMount_GetBase(unsigned char BusId); 
//直接操作时
#define BusMount_pGetBase(busId)   (BusMount.pBusMount[busId])


#endif //#define	__BUS_MOUNT_H

