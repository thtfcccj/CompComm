/*******************************************************************************

                        总线基类结构
此结构可用于总线管理时，做为其基类使用
*******************************************************************************/
#ifndef __BUS_BASE_H
#define	__BUS_BASE_H

#include "BusCount.h"

//----------------------------主结构-------------------------------
//仅定义结构， 允许直接成员访问
struct _BusBase{
  struct _BusCount Count;         //总线计数器和基类以方便外部使用
  unsigned char Id;               //为此总线分配的ID号
  unsigned char ExU8;             //留给派生类的值
  union{
    unsigned char U8[2];          //留给派生类的值
    unsigned short U16;           //留给派生类的值
    signed short S16;             //留给派生类的值           
  }Ex;
};

//------------------------------初始化函数---------------------------
//派生类使用的变量将全部初始化为0
#include <string.h>
#define BusBase_Init(bus, id) do{\
  memset(bus, 0, sizeof(struct _BusBase)); (bus)->Id = id; }while(0)

//--------------------------由总线ID得主结构--------------------------------
#define BusBase_cbGet(busId) ((struct _BusBase*)BusCount_pcbGet(busId))
    
#endif




