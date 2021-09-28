/******************************************************************************

                   精简3线制SPI设备抽像模块实现

******************************************************************************/

#include "Spi3mIo.h"
#include <string.h>

/******************************************************************************
                         内部函数实现
******************************************************************************/

//---------------------------写数据函数---------------------------------
//此函数将数据从DIO移出（DIO需提前置为输出状态）
static void _WrData(const struct _Spi3mIoCs *pCs,
                    unsigned char BitLen,//位长度,支持按位长发送
                    unsigned char Data)//数据
{
  unsigned char Cfg = pCs->Cfg; //配置
  unsigned char Shift;
  if(Cfg & SPI3_MIO_DORD_LSB) Shift = 0x01;//低位在前
  else  Shift = 0x80; //高位在前
  while(BitLen > 0){
    //准备好需移出的数据
    if(Data & Shift) Spi3mIo_cbSetDio();
    else Spi3mIo_cbClrDio();
    //时钟起始沿
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbClrClk();
    else  Spi3mIo_cbSetClk();
    //数据保持
    Spi3mIo_cbDelay(pCs->BitDelay);
    //时钟结束沿
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbSetClk();
    else Spi3mIo_cbClrClk();
    //数据保持
    Spi3mIo_cbDelay(pCs->BitDelay);

    //准备下一个需发送的数据
    BitLen--;
    if(Cfg & SPI3_MIO_DORD_LSB) Shift <<= 1;//低位在前
    else Shift >>= 1; //高位在前
  }//end while
}

//-------------------------读取数据函数------------------------------
//此函数将数据从DIO移出,同时从DIO读数
static unsigned char _RdData(const struct _Spi3mIoCs *pCs,
                             unsigned char BitLen)    //位长度,支持按位长发送
{
  unsigned char Data = 0;
  unsigned char Cfg = pCs->Cfg; //配置
  unsigned char Shift;
  if(Cfg & SPI3_MIO_DORD_LSB) Shift = 0x01;//低位在前
  else  Shift = 0x80; //高位在前
  while(BitLen > 0){
    //时钟起始沿
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbClrClk();
    else  Spi3mIo_cbSetClk();
    //起始沿采样时读取数据
    if(!(Cfg & SPI3_MIO_CPHA_END)){
      if(Spi3mIo_cbIsDio()) Data |= Shift;
    }
    //数据保持
    Spi3mIo_cbDelay(pCs->BitDelay);
    //时钟结束沿
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbSetClk();
    else Spi3mIo_cbClrClk();
    //结束沿采样时读取数据
    if(Cfg & SPI3_MIO_CPHA_END){
      if(Spi3mIo_cbIsDio()) Data |= Shift;
    }
    //数据保持
    Spi3mIo_cbDelay(pCs->BitDelay);

    //准备下一个需发送的数据
    BitLen--;
    if(Cfg & SPI3_MIO_DORD_LSB) Shift <<= 1;//低位在前
    else Shift >>= 1; //高位在前
  }//end while
  return Data;
}

/******************************************************************************
                         相关函数实现
******************************************************************************/

//--------------------------------初始化函数---------------------------------
void Spi3mIo_Init(void)
{
  Spi3mIo_cbIoInit();
}

//------------------------------只写数据函数----------------------------------
void Spi3mIo_WO(unsigned char Cs,            //操作的设备
                 const unsigned char *pSend, //发送数据在前，含指令+数据区
                 unsigned char SendSize)    //发送数据大小
{
  Spi3mIo_RW(Cs, pSend, SendSize, NULL, 0);
}
//------------------------------读写数据函数----------------------------------
void Spi3mIo_RW(unsigned char Cs,            //操作的设备
                 const unsigned char *pSend, //发送数据在前，含指令+数据区
                 unsigned char SendSize,     //发送数据大小，为0时直接收
                 unsigned char *pRcv,       //接收数据缓冲，仅为数据
                 unsigned char RcvSize)     //接收数据大小，0时只发不收
{
  unsigned char BitLen;//位长
  struct _Spi3mIoCs *pCs = &Spi3mIo_CbCs[Cs];

  //软件SPI直接强制发送
  Spi3mIo_cbEnterCritical();//进入临界区
  Spi3mIo_cbValidCs(Cs);//Spi3mIo

  //发送数据
  if(SendSize > 0){
    Spi3mIo_cbOutDio(); //输出状态
    for(; SendSize > 0; SendSize--, pSend++){
      //最后发送位长度判断
      if(SendSize > 1) BitLen = 8;
      else BitLen = pCs->LastBitLen;
       _WrData(pCs, BitLen, *pSend);
      Spi3mIo_cbDelay(pCs->ByteDelay); //字节间延时
    }
  }
  if(!RcvSize){//仅发送
    Spi3mIo_cbInvalidCs(Cs); //结束
    return;
  }  

  //接收数据
  Spi3mIo_cbInDio(); //输入状态
  for(; RcvSize > 0; RcvSize--, pRcv++){
    //最后发送位长度判断
    if(RcvSize > 1) BitLen = 8;
    else BitLen = pCs->LastBitLen;
     *pRcv = _RdData(pCs, BitLen);
    Spi3mIo_cbDelay(pCs->ByteDelay); //字节间延时
  }

  Spi3mIo_cbInvalidCs(Cs); //结束
  Spi3mIo_cbOutDio();    //默认为输出状态
}
