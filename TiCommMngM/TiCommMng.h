/*********************************************************************************

//			��ʱ����(Time Interval)��Ϊ����֡�ж����ݵ��������-������ʱʵ��

//��ģ��ΪԭTiCommMng�Ķ����������ڶ������ʱ�Ĺ��������以��(1����Ŀ��2��1)
//��ģ��ʹ��UsartDrvͨѶ,�ʶ�����Ӳ����Ӧ��
*********************************************************************************/
#ifndef _TI_COMM_MNG_H
#define	_TI_COMM_MNG_H

/*****************************************************************************
                             �������
******************************************************************************/

//��ģ��֧�֣�
//#define SUPPORT_TI_COMM_MNG 

//������Ԥ����ʱ���������ж����ݣ���ֱ�Ӳ��뷢������,ȫ�ֶ���
//#define SUPPORT_TI_COMM_MNG_PRE   

/*******************************************************************************
                              ��ؽṹ
*******************************************************************************/
#include "UsartTiny.h" //ͨ���˶�����ģ����Ƶײ�
#include "UsartDev.h" //

struct _TiCommMng{
  struct _UsartTiny UsartTiny; 
  unsigned char cbData;  //�ϲ�Ӧ����Ҫ���������(4byte����)
  unsigned char Count;   //�������շ���ʱ��װ��ֵ
  unsigned char Index;   //Rtuģ�������շ���ʱ��
  
  unsigned char Flag;//��ر�־,������
};

//��ر�־����Ϊ:
#define TI_COMM_MNG_RCV_DOING    0x01 //���ݽ��չ����б�־
#define TI_COMM_MNG_SEND_DOING   0x02 //���ݷ��͹����б�־
#define TI_COMM_MNG_SUSPEND      0x04 //֧��Ԥ����ʱ�������־

/*****************************************************************************
                             ��غ���
******************************************************************************/

//-----------------------------��ʼ������-------------------------------
//���ô˺���ǰ,���ʼ��UsartDev(��UsartId)����IO������Usart������������
void TiCommMng_Init(struct _TiCommMng *pMng,
                    const struct _UsartDevPt *pFun, //��̬��������
                    struct _UsartDev *pDev);

//-------------------------------�ж�����----------------------------
//���˺�������1ms����жϽ�����
void TiCommMng_IntTask(struct _TiCommMng *pMng);

//-------------------------------��ͨ��ѯ����----------------------------
//���˺�������ϵͳ1ms������
void TiCommMng_Task(struct _TiCommMng *pMng);

//-------------------------���ն�ʱ����λ����-------------------------------
void TiCommMng_ResetRcvTimer(struct _TiCommMng *pMng);

//-------------------------���Ͷ�ʱ����λ����-------------------------------
#define TiCommMng_ResetSendTimer(mng) \
  do{(mng)->Index = (mng)->Count;}while(0)

//-------------------------------������--------------------------------
//����󣬿ɲſ�ֱ��ʹ��pUsartDev�е��շ����ݻ�����
//���ô˺����󣬱������TiCommMng_PreInsertSend()���ܽ⿪
#ifdef SUPPORT_TI_COMM_MNG_PRE
   void TiCommMng_Suspend(struct _TiCommMng *pMng);
#else //��֧��ʱ
   #define TiCommMng_Suspend(mng) do{}while(0)
#endif

//----------------------------ǿ�Ʋ��뷢�ͺ���--------------------------------
#ifdef SUPPORT_TI_COMM_MNG_PRE
  void TiCommMng_InsertSend(struct _TiCommMng *pMng,
                            unsigned char SendLen); //�������ݳ���
#else //��֧��ʱ
   #define TiCommMng_PreInsertSend(mng, sendLen) do{}while(0)
#endif
  
//----------------------------�õ��豸ID--------------------------------
#define TiCommMng_GetDevId(mng)  UsartTiny_GetDevId(&(mng)->UsartTiny)
  
/*******************************************************************************
                            �ص�����
********************************************************************************/

//----------------------�õ�����֡���msʱ��----------------------------
//�벨�����йأ�������ʱ�䣬��Ϊ��һ֡���ݵĽ���
unsigned char TiCommMng_cbBuadId2FremeOv(const struct _TiCommMng *pMng);

//---------------------------------����Ԥ������-------------------------------
//������UsartTiny.Data��,0Ϊ��ַ,LenΪ����,�����跢�͵��ֽ���(����ַ)
//����: 255δ��ʶ��;  254��ͣ����, 0:������ȷ�������������� 
//����:�������ݸ���
#ifdef SUPPORT_TI_COMM_MNG_PRE 
  unsigned char  TiCommMng_cbRcvPreDataPro(struct _TiCommMng *pMng);
#endif

//--------------------------�յ�ÿ�����ݺ��ͨ��-------------------------------
//���ط�0ʱ���������������������������
unsigned char TiCommMng_cbRcvedNotify(struct _TiCommMng *pMng);   

//--------------------------���ݴ�����-------------------------------
//���������ݺ󽫵��ô˺���
//�˺����ڣ�Ӧ������UsartTiny.Data�������������,��������������
//����255��ʾ����;  0:������ȷ�������������� 
//����:�������ݸ���
unsigned char TiCommMng_cbDataPro(struct _TiCommMng *pMng);

  
#endif //_TI_COMM_MNG_H




