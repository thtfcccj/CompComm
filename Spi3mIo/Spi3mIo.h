/******************************************************************************

                   3����SPI��ƬѡIOʵ��ģ��ӿ�
//��ģ�����ΪDS1302,TM1628��3�����豸��ͨѶ����(Ƭѡ+˫������+ʱ��)��
//���������߷�ʽ���ӣ�CS�����֣�,ͬʱ�������������в�ͬ����(��Spi3mIoCs)��
//��ģ��ʹ��IOģ�ⷽ��ʵ��3����SPIͨѶ��ֱ�ӵ����������������Ƭѡ��������ͬIC:
//SCLK: ����ʱ����
//DIO:  ����������
//CS[SPI3_MIO_CS_COUNT]: Ƭѡ�ߣ�ͬʱ�����ֲ�ͬIC������(�����ͺ��Ƿ���ͬ)
//��ģ���֧��������ʽͨѶ���ҽ��������̵߳��á�

******************************************************************************/
#ifndef __SPI3_MIO_H
#define __SPI3_MIO_H
#ifdef SUPPORT_EX_PREINCLUDE//��֧��Preinlude�r
  #include "Preinclude.h"
#endif

/******************************************************************************
                             �������
******************************************************************************/

//����֧�ֵ�CS,���豸����
#ifndef SPI3_MIO_CS_COUNT
  #define SPI3_MIO_CS_COUNT 1
#endif

/******************************************************************************
                             ��ؽṹ
******************************************************************************/

//---------------------------Spi3 CS����-------------------------------------
//CS���ã�����Բ�ͬIC
struct _Spi3mIoCs{
  volatile unsigned char Cfg;        //��ر�־,������
  unsigned char LastBitLen;          //λ���ȿ��ƹ���:����ֽ�λ����
  unsigned char BitDelay;            //λ���ƽ����ʱ��,������ϵͳʱ��������ù�ϵ(Ĭ��us)
  unsigned char ByteDelay;           //�ֽڼ��ƽ����ʱ��,������ϵͳʱ��������ù�ϵ(Ĭ��us)
};

//����,��ر�־����Ϊ:
#define SPI3_MIO_CPOL_H     0x80     //����ʱSCKΪ�ߵ�ƽ��־,����Ϊ�͵�ƽ
#define SPI3_MIO_CPHA_END   0x40     //SCK�ڽ����ز���,��ʼ�������������෴
#define SPI3_MIO_DORD_LSB   0x20     //�ֽڷ���˳��:��λLSB�ȷ�,����֮

/******************************************************************************
                         ��غ���
******************************************************************************/

//--------------------------------��ʼ������---------------------------------
void Spi3mIo_Init(void);

//------------------------------ֻд���ݺ���----------------------------------
void Spi3mIo_WO(unsigned char Cs,            //�������豸
                 const unsigned char *pSend, //����������ǰ����ָ��+������
                 unsigned char SendSize);     //�������ݴ�С

//------------------------------��д���ݺ���----------------------------------
void Spi3mIo_RW(unsigned char Cs,            //�������豸
                 const unsigned char *pSend, //����������ǰ����ָ��+������
                 unsigned char SendSize,     //�������ݴ�С��Ϊ0ʱֱ����
                 unsigned char *pRcv,       //�������ݻ��壬��Ϊ����
                 unsigned char CmdSize);     //�������ݴ�С��0ʱֻ������

/******************************************************************************
                         ����û��ײ���ƻص�����
//ע:��Ƭѡ����     
******************************************************************************/

//Ӧ�ò���أ����û����岢ʵ�ֵ�Ƭѡ��Ϣ��
extern const struct _Spi3mIoCs Spi3mIo_CbCs[SPI3_MIO_CS_COUNT];

//---------------------------------ѡ��Ƭѡ-----------------------------------
//һ��Ϊ�͵�ƽѡ��
#if SPI3_MIO_CS_COUNT < 2
  #define Spi3mIo_cbValidCs(cs) do{ValidSpi3_Cs();}while(0)  //ֱ��IOʵ��
#else
  void Spi3mIo_cbValidCs(unsigned char CsId);
#endif

//---------------------------------ȡ��ѡ��Ƭѡ---------------------------------
//һ��Ϊ�ߵ�ƽ�ͷ�
#if SPI3_MIO_CS_COUNT < 2
  #define Spi3mIo_cbInvalidCs(cs) do{InvalidSpi3_Cs();}while(0)  //ֱ��IOʵ��
#else
  void Spi3mIo_cbInvalidCs(unsigned char CsId);
#endif

/******************************************************************************
                             Ӳ��IO���ƻص�����
//ע:����Ƭѡ����
******************************************************************************/
#include "IOCtrl.h"  //ֱ�Ӻ�ʵ��:

//--------------------------IO��ʼ��--------------------------------
//�轫CS��IO����Ϊ��Ч״̬
#define Spi3mIo_cbIoInit() do{Spi3IoCfg();}while(0)

//--------------------------ʱ�Ӳ���--------------------------------
#define Spi3mIo_cbSetClk() do{SetSpi3_SCK();}while(0)
#define Spi3mIo_cbClrClk() do{ClrSpi3_SCK();}while(0)

//--------------------------DIO����--------------------------------
//��������������:
#define Spi3mIo_cbInDio() do{InSpi3_DIO();}while(0)
#define Spi3mIo_cbOutDio() do{OutSpi3_DIO();}while(0)
//�ߵ͵�ƽ�������:
#define Spi3mIo_cbSetDio() do{SetSpi3_DIO();}while(0)
#define Spi3mIo_cbClrDio() do{ClrSpi3_DIO();}while(0)
//�ж�DIO�����ƽ:
#define Spi3mIo_cbIsDio()   (IsSpi3_DIO())

/******************************************************************************
                             �����ص�����
******************************************************************************/

//------------------------��ʱ��������-------------------------------
//��ʱʱ�������õ�ʱ���Ӧ
#ifndef Spi3mIo_cbDelay
  #include "Delay.h" //Ĭ��usΪ��λ
  #define Spi3mIo_cbDelay(delay) DelayUs(delay)
#endif

//------------------------�����ٽ���-------------------------------
//void Spi3mIo_cbEnterCritical(void);
#define Spi3mIo_cbEnterCritical() do{}while(0)

//------------------------�˳��ٽ���-------------------------------
//void Spi3mIo_cbExitCritical(void);
#define Spi3mIo_cbExitCritical() do{}while(0)


#endif // #define __SPI3_MIO_SOFT_H
