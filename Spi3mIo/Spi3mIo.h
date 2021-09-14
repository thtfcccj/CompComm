/******************************************************************************

                   3线制SPI多片选IO实现模块接口
//此模块设计为DS1302,TM1628等3线制设备的通讯驱动(片选+双向数据+时钟)，
//并允许总线方式连接（CS线区分）,同时各个驱动允许有不同配置(见Spi3mIoCs)。
//此模块使用IO模拟方案实现3线制SPI通讯，直接单例化，但允许多个片选以驱动不同IC:
//SCLK: 公共时钟线
//DIO:  公共数据线
//CS[SPI3_MIO_CS_COUNT]: 片选线，同时可区分不同IC及配置(不管型号是否相同)
//此模块仅支持阻塞方式通讯，且仅允许单线程调用。

******************************************************************************/
#ifndef __SPI3_MIO_H
#define __SPI3_MIO_H
#ifdef SUPPORT_EX_PREINCLUDE//不支持Preinlude時
  #include "Preinclude.h"
#endif

/******************************************************************************
                             相关配置
******************************************************************************/

//定义支持的CS,即设备数量
#ifndef SPI3_MIO_CS_COUNT
  #define SPI3_MIO_CS_COUNT 1
#endif

/******************************************************************************
                             相关结构
******************************************************************************/

//---------------------------Spi3 CS定义-------------------------------------
//CS配置，以针对不同IC
struct _Spi3mIoCs{
  volatile unsigned char Cfg;        //相关标志,见定义
  unsigned char LastBitLen;          //位长度控制功能:最后字节位长度
  unsigned char BitDelay;            //位间电平保持时间,具体与系统时钟与调用用关系(默认us)
  unsigned char ByteDelay;           //字节间电平保持时间,具体与系统时钟与调用用关系(默认us)
};

//其中,相关标志定义为:
#define SPI3_MIO_CPOL_H     0x80     //空闲时SCK为高电平标志,否则为低电平
#define SPI3_MIO_CPHA_END   0x40     //SCK在结束沿采样,起始沿置数，否则相反
#define SPI3_MIO_DORD_LSB   0x20     //字节发送顺序:置位LSB先发,否则反之

/******************************************************************************
                         相关函数
******************************************************************************/

//--------------------------------初始化函数---------------------------------
void Spi3mIo_Init(void);

//------------------------------只写数据函数----------------------------------
void Spi3mIo_WO(unsigned char Cs,            //操作的设备
                 const unsigned char *pSend, //发送数据在前，含指令+数据区
                 unsigned char SendSize);     //发送数据大小

//------------------------------读写数据函数----------------------------------
void Spi3mIo_RW(unsigned char Cs,            //操作的设备
                 const unsigned char *pSend, //发送数据在前，含指令+数据区
                 unsigned char SendSize,     //发送数据大小，为0时直接收
                 unsigned char *pRcv,       //接收数据缓冲，仅为数据
                 unsigned char CmdSize);     //接收数据大小，0时只发不收

/******************************************************************************
                         相关用户底层控制回调函数
//注:含片选控制     
******************************************************************************/

//应用层相关，由用户定义并实现的片选信息。
extern struct _Spi3mIoCs Spi3mIo_CbCs[SPI3_MIO_CS_COUNT];

//---------------------------------选中片选-----------------------------------
//一搬为低电平选中
#if SPI3_MIO_CS_COUNT < 2
  #define Spi3mIo_cbValidCs(cs) do{ValidSpi3_Cs();}while(0)  //直接IO实现
#else
  void Spi3mIo_cbValidCs(unsigned char CsId);
#endif

//---------------------------------取消选中片选---------------------------------
//一搬为高电平释放
#if SPI3_MIO_CS_COUNT < 2
  #define Spi3mIo_cbInvalidCs(cs) do{InvalidSpi3_Cs();}while(0)  //直接IO实现
#else
  void Spi3mIo_cbInvalidCs(unsigned char CsId);
#endif

/******************************************************************************
                             硬件IO控制回调函数
//注:不含片选控制
******************************************************************************/
#include "IOCtrl.h"  //直接宏实现:

//--------------------------IO初始化--------------------------------
//需将CS等IO口置为无效状态
#define Spi3mIo_cbIoInit() do{Spi3IoCfg();}while(0)

//--------------------------时钟操作--------------------------------
#define Spi3mIo_cbSetClk() do{SetSpi3_SCK();}while(0)
#define Spi3mIo_cbClrClk() do{ClrSpi3_SCK();}while(0)

//--------------------------DIO操作--------------------------------
//输入输出方向控制:
#define Spi3mIo_cbInDio() do{InSpi3_DIO();}while(0)
#define Spi3mIo_cbOutDio() do{OutSpi3_DIO();}while(0)
//高低电平输出控制:
#define Spi3mIo_cbSetDio() do{SetSpi3_DIO();}while(0)
#define Spi3mIo_cbClrDio() do{ClrSpi3_DIO();}while(0)
//判断DIO输入电平:
#define Spi3mIo_cbIsDio()   (IsSpi3_DIO())

/******************************************************************************
                             其它回调函数
******************************************************************************/

//------------------------延时函数调用-------------------------------
//延时时间与设置的时间对应
#ifndef Spi3mIo_cbDelay
  #include "Delay.h" //默认us为单位
  #define Spi3mIo_cbDelay(delay) DelayUs(delay)
#endif

//------------------------进入临界区-------------------------------
//void Spi3mIo_cbEnterCritical(void);
#define Spi3mIo_cbEnterCritical() do{}while(0)

//------------------------退出临界区-------------------------------
//void Spi3mIo_cbExitCritical(void);
#define Spi3mIo_cbExitCritical() do{}while(0)


#endif // #define __SPI3_MIO_SOFT_H

