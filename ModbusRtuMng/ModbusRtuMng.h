/***********************************************************************
//					 ModbusRtu模式管理层实现
//此模块负责底层通讯，但独立于应用
***********************************************************************/
#ifndef _MODBUS_RTU_MNG_H
#define	_MODBUS_RTU_MNG_H

/*****************************************************************************
                             配局配置
******************************************************************************/

//支持地址高两位保为波特率时,支持时强制无校验8数据位, SUPPORT_MODBUS_BUND_ADR67二先一
//#define SUPPORT_MODBUS_BUND_ADR67

//支持软件地址时，在全局里定义, SUPPORT_MODBUS_BUND_ADR67二先一
//#define SUPPORT_MODBUS_SW_ADR

//仅数据预处理时，允许先判断数据，或直接插入发送数据,全局定义
//#define SUPPORT_MODBUS_PRE   

//收到正确数据后数据通报,返回未识别后处理,可用于不破坏数据通报情况下处理其它数据
//#define SUPPORT_MODBUS_DATA_REMAIN

/*****************************************************************************
                             相关配置
******************************************************************************/





/*******************************************************************************
                              相关结构
*******************************************************************************/
#include "UsartTiny.h"//最大数据长度

struct _ModbusRtuMngInfo{
  unsigned char CommCfg;  //通讯配置位,参考UsartDevCfg Tiny模式定义
  unsigned char Adr;     //通讯地址
};

struct _ModbusRtuMng
{
  struct _ModbusRtuMngInfo Info;
  unsigned char Count;   //Rtu模数数据收发定时器装载值
  unsigned char Index;   //Rtu模数数据收发定时器

  unsigned char Flag;//相关标志,见定义
};

//相关标志定义为:
#define MODBUS_RTU_MNG_RCV_DOING    0x01 //数据接收过程中标志
#define MODBUS_RTU_MNG_SEND_DOING   0x02 //数据发送过程中标志
#define MODBUS_RTU_MNG_SUSPEND      0x04 //支持预处理时，挂起标志

extern struct _ModbusRtuMng ModbusRtuMng;//直接实例化

/*****************************************************************************
                             相关函数
******************************************************************************/

//-----------------------------初始化函数-------------------------------
void ModbusRtuMng_Init(unsigned char Inited);

//-------------------------------普通查询任务----------------------------
//将此函数放入系统1ms进程中
void ModbusRtuMng_Task(void);

//-------------------------------挂起函数--------------------------------
//挂起后，可才可直接使用UsartTiny.Data缓冲区
//调用此函数后，必须调用ModbusRtuMng_PreInsertSend()才能解开
#ifdef SUPPORT_MODBUS_PRE
   void ModbusRtuMng_Suspend(void);
#else //不支持时
   #define ModbusRtuMng_Suspend() do{}while(0)
#endif

//----------------------------强制插入发送函数--------------------------------
#ifdef SUPPORT_MODBUS_PRE
  void ModbusRtuMng_InsertSend(unsigned char SendLen); //发送数据长度
#else //不支持时
   #define ModbusRtuMng_PreInsertSend(sendLen ) do{}while(0)
#endif

//------------------------从机地址相关操作函数---------------------------
#define ModbusRtuMng_GetAdr() (ModbusRtuMng.Info.Adr)
void ModbusRtuMng_SetAdr(unsigned char Adr);

//--------------------------通讯参数操作函数-----------------------------
#define ModbusRtuMng_GetCommCfg() (ModbusRtuMng.Info.CommCfg)
void ModbusRtuMng_SetCommCfg(unsigned char CommCfg);

//-------------------------接收定时器复位函数-------------------------------
#define ModbusRtuMng_ResetRcvTimer() \
  do{ModbusRtuMng.Index = ModbusRtuMng.Count;\
     ModbusRtuMng.Flag |= MODBUS_RTU_MNG_RCV_DOING;}while(0)

//-------------------------发送定时器复位函数-------------------------------
#define ModbusRtuMng_ResetSendTimer() \
  do{ModbusRtuMng.Index = ModbusRtuMng.Count;}while(0)

/*******************************************************************************
                            回调函数
********************************************************************************/

//------------------收到正确数据后的数据通报-------------------------
//数据在Data中,0为地址,Len为长度,返回需发送的字节数(含地址)
//返回255表示未能识别;  0:数据正确，但不返回数据 
//其它:返回数据个数
unsigned char ModbusRtuMng_cbDataNotify(unsigned char Data[],unsigned char Len);

//------------------收到正确数据后数据通报未识别后处理-------------------------
#ifdef SUPPORT_MODBUS_DATA_REMAIN//不能识别的数据时后处理
    //数据在Data中,0为地址,Len为长度,返回需发送的字节数(含地址)
    //返回255表示未能识别;  0:数据正确，但不返回数据 
    //其它:返回数据个数
    unsigned char ModbusRtuMng_cbDataRemain(unsigned char Data[],unsigned char Len);
#endif

//---------------------------------数据预处理函数-------------------------------
//数据在UsartTiny.Data中,0为地址,Len为长度,返回需发送的字节数(含地址)
//返回: 255未能识别;  254暂停接收, 0:数据正确，但不返回数据 
//其它:返回数据个数
#ifdef SUPPORT_MODBUS_PRE 
  unsigned char  ModbusRtuMng_cbRcvPreDataPro(void);
#endif
  
#endif

