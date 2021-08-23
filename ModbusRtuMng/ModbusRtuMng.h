/***********************************************************************
//					 ModbusRtuģʽ�����ʵ��
//��ģ�鸺��ײ�ͨѶ����������Ӧ��
***********************************************************************/
#ifndef _MODBUS_RTU_MNG_H
#define	_MODBUS_RTU_MNG_H

/*****************************************************************************
                             �������
******************************************************************************/

//֧�ֵ�ַ����λ��Ϊ������ʱ,֧��ʱǿ����У��8����λ, SUPPORT_MODBUS_BUND_ADR67����һ
//#define SUPPORT_MODBUS_BUND_ADR67

//֧�������ַʱ����ȫ���ﶨ��, SUPPORT_MODBUS_BUND_ADR67����һ
//#define SUPPORT_MODBUS_SW_ADR

//������Ԥ����ʱ���������ж����ݣ���ֱ�Ӳ��뷢������,ȫ�ֶ���
//#define SUPPORT_MODBUS_PRE   

//�յ���ȷ���ݺ�����ͨ��,����δʶ�����,�����ڲ��ƻ�����ͨ������´�����������
//#define SUPPORT_MODBUS_DATA_REMAIN

/*****************************************************************************
                             �������
******************************************************************************/





/*******************************************************************************
                              ��ؽṹ
*******************************************************************************/
#include "UsartTiny.h"//������ݳ���

struct _ModbusRtuMngInfo{
  unsigned char CommCfg;  //ͨѶ����λ,�ο�UsartDevCfg Tinyģʽ����
  unsigned char Adr;     //ͨѶ��ַ
};

struct _ModbusRtuMng
{
  struct _ModbusRtuMngInfo Info;
  unsigned char Count;   //Rtuģ�������շ���ʱ��װ��ֵ
  unsigned char Index;   //Rtuģ�������շ���ʱ��

  unsigned char Flag;//��ر�־,������
};

//��ر�־����Ϊ:
#define MODBUS_RTU_MNG_RCV_DOING    0x01 //���ݽ��չ����б�־
#define MODBUS_RTU_MNG_SEND_DOING   0x02 //���ݷ��͹����б�־
#define MODBUS_RTU_MNG_SUSPEND      0x04 //֧��Ԥ����ʱ�������־

extern struct _ModbusRtuMng ModbusRtuMng;//ֱ��ʵ����

/*****************************************************************************
                             ��غ���
******************************************************************************/

//-----------------------------��ʼ������-------------------------------
void ModbusRtuMng_Init(unsigned char Inited);

//-------------------------------��ͨ��ѯ����----------------------------
//���˺�������ϵͳ1ms������
void ModbusRtuMng_Task(void);

//-------------------------------������--------------------------------
//����󣬿ɲſ�ֱ��ʹ��UsartTiny.Data������
//���ô˺����󣬱������ModbusRtuMng_PreInsertSend()���ܽ⿪
#ifdef SUPPORT_MODBUS_PRE
   void ModbusRtuMng_Suspend(void);
#else //��֧��ʱ
   #define ModbusRtuMng_Suspend() do{}while(0)
#endif

//----------------------------ǿ�Ʋ��뷢�ͺ���--------------------------------
#ifdef SUPPORT_MODBUS_PRE
  void ModbusRtuMng_InsertSend(unsigned char SendLen); //�������ݳ���
#else //��֧��ʱ
   #define ModbusRtuMng_PreInsertSend(sendLen ) do{}while(0)
#endif

//------------------------�ӻ���ַ��ز�������---------------------------
#define ModbusRtuMng_GetAdr() (ModbusRtuMng.Info.Adr)
void ModbusRtuMng_SetAdr(unsigned char Adr);

//--------------------------ͨѶ������������-----------------------------
#define ModbusRtuMng_GetCommCfg() (ModbusRtuMng.Info.CommCfg)
void ModbusRtuMng_SetCommCfg(unsigned char CommCfg);

//-------------------------���ն�ʱ����λ����-------------------------------
#define ModbusRtuMng_ResetRcvTimer() \
  do{ModbusRtuMng.Index = ModbusRtuMng.Count;\
     ModbusRtuMng.Flag |= MODBUS_RTU_MNG_RCV_DOING;}while(0)

//-------------------------���Ͷ�ʱ����λ����-------------------------------
#define ModbusRtuMng_ResetSendTimer() \
  do{ModbusRtuMng.Index = ModbusRtuMng.Count;}while(0)

/*******************************************************************************
                            �ص�����
********************************************************************************/

//------------------�յ���ȷ���ݺ������ͨ��-------------------------
//������Data��,0Ϊ��ַ,LenΪ����,�����跢�͵��ֽ���(����ַ)
//����255��ʾδ��ʶ��;  0:������ȷ�������������� 
//����:�������ݸ���
unsigned char ModbusRtuMng_cbDataNotify(unsigned char Data[],unsigned char Len);

//------------------�յ���ȷ���ݺ�����ͨ��δʶ�����-------------------------
#ifdef SUPPORT_MODBUS_DATA_REMAIN//����ʶ�������ʱ����
    //������Data��,0Ϊ��ַ,LenΪ����,�����跢�͵��ֽ���(����ַ)
    //����255��ʾδ��ʶ��;  0:������ȷ�������������� 
    //����:�������ݸ���
    unsigned char ModbusRtuMng_cbDataRemain(unsigned char Data[],unsigned char Len);
#endif

//---------------------------------����Ԥ������-------------------------------
//������UsartTiny.Data��,0Ϊ��ַ,LenΪ����,�����跢�͵��ֽ���(����ַ)
//����: 255δ��ʶ��;  254��ͣ����, 0:������ȷ�������������� 
//����:�������ݸ���
#ifdef SUPPORT_MODBUS_PRE 
  unsigned char  ModbusRtuMng_cbRcvPreDataPro(void);
#endif
  
#endif

