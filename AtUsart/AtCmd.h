/*******************************************************************************

                  AT命令封装层
此模块建在AtUsar上层，提供辅助用户接口
此接口数据以字符方式组织
*******************************************************************************/
#ifndef __AT_CMD_H
#define	__AT_CMD_H

/******************************************************************************
		                        相关配置 
******************************************************************************/

//阻塞方式工作时，默认读等待时间,ms为单位
#ifndef AT_CMD_BLOCKING_RD_WAIT 
  #define AT_CMD_BLOCKING_RD_WAIT_OV   800
#endif

//阻塞方式工作时，默认读数据间停止时间,ms为单位
#ifndef AT_CMD_BLOCKING_RD_WAIT 
  #define AT_CMD_BLOCKING_RD_DOING_OV   800
#endif

#include "AtUsart.h"
/******************************************************************************
		                     非阻塞方式操作函数
******************************************************************************/

//-----------------------------启动写函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_WrStart(struct _AtUsart *pAtUsart,
                   const unsigned char *pWrData, //待写入的数据
                   unsigned short Len,//数据长度
                   unsigned char Cfg);//AtUsart定义的写配置

//-----------------------------启动写AT函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_WrAtStart(struct _AtUsart *pAtUsart,
                     const char *pCmd);  //写命令,不含AT开始与结束字符

//-----------------------------启动读函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_RdStart(struct _AtUsart *pAtUsart,
                   unsigned char Cfg,//AtUsart定义的读配置
                   unsigned short WaitOv,//等待数据ms单位时间，0一直等待
                   unsigned short DongOv);//数据接收间隔ms单位时间，0一直收 

//-----------------------------启动读AT函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_RdAtStart(struct _AtUsart *pAtUsart);

//-----------------------------启动写后自动读AT函数-----------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_RwAtStart(struct _AtUsart *pAtUsart,
                     const char *pCmd,  //写命令,不含AT开始与结束字符
                     unsigned char Cfg); //AtUsart定义的读配置

/******************************************************************************
		                     阻塞方式读写数据函数
******************************************************************************/

//--------------------------带配置写函数---------------------------------
//阻塞方式工作，返回0成功,否则未写入
signed char AtCmd_CfgWr(struct _AtUsart *pAtUsart,
                         const unsigned char *pWrData, //待写入的数据
                         unsigned short Len,//数据长度
                         unsigned char Cfg);//AtUsart定义的写配置

//--------------------------带配置读函数---------------------------------
//阻塞方式工作，返回0成功,否则未写入
signed char AtCmd_CfgRd(struct _AtUsart *pAtUsart,
                         unsigned char Cfg);//AtUsart定义的读配置

//------------------------带配置写后读返回状态函数------------------------------
//阻塞方式工作，返回0成功,否则返回错误码
signed char AtCmd_CfgRw(struct _AtUsart *pAtUsart,
                         const unsigned char *pWrData, //待写入的数据
                         unsigned short Len,//数据长度
                         unsigned char WrCfg,  //AtUsart定义的读配置
                         unsigned char RdCfg);//AtUsart定义的读配置                      

/******************************************************************************
		                 阻塞方式读写字符函数,返回字符串
******************************************************************************/

//------------------------带配置写后读回字符串函数---------------------------------
//阻塞方式工作，成功时返回读取的字符串,否则返回NULL
const char *AtCmd_pCfgRwStr(struct _AtUsart *pAtUsart,
                            const char *pCmd,     //写命令,不含AT开始与结束字符
                            unsigned char WrCfg,  //AtUsart定义的读配置
                            unsigned char RdCfg);  //AtUsart定义的读配置  

//---------------------------写标准AT指令函数---------------------------------
//阻塞方式工作，返回结果，0正确
signed char AtCmd_WrAt(struct _AtUsart *pAtUsart,
                        const char *pCmd);       //写命令,不含开始结束字符

//---------------------------写并读启动标准AT指令函数---------------------------------
//阻塞方式工作，返回结果，0正确
signed char AtCmd_RwAtStrStart(struct _AtUsart *pAtUsart,
                               const char *pCmd);       //写命令,不含开始结束字符

//---------------------------写并读回标准AT指令函数---------------------------------
//阻塞方式工作，返回读回的数据缓冲区(不含返回的开始结束字符)，NULL表示错误
//数据正确时，接收长度通过AtCmd_GetRdLen()获得
const char *AtCmd_pRwAt(struct _AtUsart *pAtUsart,
                         const char *pCmd);       //写命令,不含开始结束字符

/******************************************************************************
		                        回调函数
******************************************************************************/

//---------------------------获得当前状态码-----------------------------------
signed char AtCmd_cbGetState(struct _AtUsart *pAtUsart);


#endif




