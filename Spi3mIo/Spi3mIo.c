/******************************************************************************

                   ����3����SPI�豸����ģ��ʵ��

******************************************************************************/

#include "Spi3mIo.h"
#include <string.h>

/******************************************************************************
                         �ڲ�����ʵ��
******************************************************************************/

//---------------------------д���ݺ���---------------------------------
//�˺��������ݴ�DIO�Ƴ���DIO����ǰ��Ϊ���״̬��
static void _WrData(const struct _Spi3mIoCs *pCs,
                    unsigned char BitLen,//λ����,֧�ְ�λ������
                    unsigned char Data)//����
{
  unsigned char Cfg = pCs->Cfg; //����
  unsigned char Shift;
  if(Cfg & SPI3_MIO_DORD_LSB) Shift = 0x01;//��λ��ǰ
  else  Shift = 0x80; //��λ��ǰ
  while(BitLen > 0){
    //׼�������Ƴ�������
    if(Data & Shift) Spi3mIo_cbSetDio();
    else Spi3mIo_cbClrDio();
    //ʱ����ʼ��
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbClrClk();
    else  Spi3mIo_cbSetClk();
    //���ݱ���
    Spi3mIo_cbDelay(pCs->BitDelay);
    //ʱ�ӽ�����
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbSetClk();
    else Spi3mIo_cbClrClk();
    //���ݱ���
    Spi3mIo_cbDelay(pCs->BitDelay);

    //׼����һ���跢�͵�����
    BitLen--;
    if(Cfg & SPI3_MIO_DORD_LSB) Shift <<= 1;//��λ��ǰ
    else Shift >>= 1; //��λ��ǰ
  }//end while
}

//-------------------------��ȡ���ݺ���------------------------------
//�˺��������ݴ�DIO�Ƴ�,ͬʱ��DIO����
static unsigned char _RdData(const struct _Spi3mIoCs *pCs,
                             unsigned char BitLen)    //λ����,֧�ְ�λ������
{
  unsigned char Data = 0;
  unsigned char Cfg = pCs->Cfg; //����
  unsigned char Shift;
  if(Cfg & SPI3_MIO_DORD_LSB) Shift = 0x01;//��λ��ǰ
  else  Shift = 0x80; //��λ��ǰ
  while(BitLen > 0){
    //ʱ����ʼ��
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbClrClk();
    else  Spi3mIo_cbSetClk();
    //��ʼ�ز���ʱ��ȡ����
    if(!(Cfg & SPI3_MIO_CPHA_END)){
      if(Spi3mIo_cbIsDio()) Data |= Shift;
    }
    //���ݱ���
    Spi3mIo_cbDelay(pCs->BitDelay);
    //ʱ�ӽ�����
    if(Cfg & SPI3_MIO_CPOL_H) Spi3mIo_cbSetClk();
    else Spi3mIo_cbClrClk();
    //�����ز���ʱ��ȡ����
    if(Cfg & SPI3_MIO_CPHA_END){
      if(Spi3mIo_cbIsDio()) Data |= Shift;
    }
    //���ݱ���
    Spi3mIo_cbDelay(pCs->BitDelay);

    //׼����һ���跢�͵�����
    BitLen--;
    if(Cfg & SPI3_MIO_DORD_LSB) Shift <<= 1;//��λ��ǰ
    else Shift >>= 1; //��λ��ǰ
  }//end while
  return Data;
}

/******************************************************************************
                         ��غ���ʵ��
******************************************************************************/

//--------------------------------��ʼ������---------------------------------
void Spi3mIo_Init(void)
{
  Spi3mIo_cbIoInit();
}

//------------------------------ֻд���ݺ���----------------------------------
void Spi3mIo_WO(unsigned char Cs,            //�������豸
                 const unsigned char *pSend, //����������ǰ����ָ��+������
                 unsigned char SendSize)    //�������ݴ�С
{
  Spi3mIo_RW(Cs, pSend, SendSize, NULL, 0);
}
//------------------------------��д���ݺ���----------------------------------
void Spi3mIo_RW(unsigned char Cs,            //�������豸
                 const unsigned char *pSend, //����������ǰ����ָ��+������
                 unsigned char SendSize,     //�������ݴ�С��Ϊ0ʱֱ����
                 unsigned char *pRcv,       //�������ݻ��壬��Ϊ����
                 unsigned char RcvSize)     //�������ݴ�С��0ʱֻ������
{
  unsigned char BitLen;//λ��
  struct _Spi3mIoCs *pCs = &Spi3mIo_CbCs[Cs];

  //���SPIֱ��ǿ�Ʒ���
  Spi3mIo_cbEnterCritical();//�����ٽ���
  Spi3mIo_cbValidCs(Cs);//Spi3mIo

  //��������
  if(SendSize > 0){
    Spi3mIo_cbOutDio(); //���״̬
    for(; SendSize > 0; SendSize--, pSend++){
      //�����λ�����ж�
      if(SendSize > 1) BitLen = 8;
      else BitLen = pCs->LastBitLen;
       _WrData(pCs, BitLen, *pSend);
      Spi3mIo_cbDelay(pCs->ByteDelay); //�ֽڼ���ʱ
    }
  }
  if(!RcvSize){//������
    Spi3mIo_cbInvalidCs(Cs); //����
    return;
  }  

  //��������
  Spi3mIo_cbInDio(); //����״̬
  for(; RcvSize > 0; RcvSize--, pRcv++){
    //�����λ�����ж�
    if(RcvSize > 1) BitLen = 8;
    else BitLen = pCs->LastBitLen;
     *pRcv = _RdData(pCs, BitLen);
    Spi3mIo_cbDelay(pCs->ByteDelay); //�ֽڼ���ʱ
  }

  Spi3mIo_cbInvalidCs(Cs); //����
  Spi3mIo_cbOutDio();    //Ĭ��Ϊ���״̬
}
