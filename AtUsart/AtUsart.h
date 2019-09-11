/***********************************************************************

                  AT(����AT)ָ����ͨѶ�ϵ�ʵ��ģ��
1. ��ģ�齨��AT(����AT)ָ��ģ����ײ�UsartDev֮��,ʵ����˫��ͨѶ
2. ��׼ATָ���ʽΪ:
     ����:��AT....<CR>�� 
    ����:��<CR><LF><response><CR><LF>��
3. ��ģ��ʵ��Ϊ��������ͬʱ֧�ֲ�ͬATָ��
4. ֧�ְ��״̬,��״̬�£��ɹ��ý��ջ�����
5. ֧������������
***********************************************************************/
#ifndef __AT_USART_H
#define	__AT_USART_H

/****************************************************************************
		                      �������
****************************************************************************/

//��������ʱ��ÿ�����ĵȴ�ʱ�䣬1msΪ��λ, >= 2
#define	AT_USART_RCV_DOING_BYTE_OV        32

//��������ʱ��������ȴ�ʱ�䣬1msΪ��λ, >= 2
#define	AT_USART_SEND_OV                  512

/****************************************************************************
		                      ��ؽṹ
****************************************************************************/
#include "UsartDev.h"

//����ص������������� 
typedef  signed char(*AtUsartNotify_t)(const void *, signed char State);

//�շ�����������:
struct _AtUsartBuf{
  unsigned short Count;         //��������С
  unsigned short Len;           //Ҫ��������ݳ���
  unsigned char *pBuf;          //������
  AtUsartNotify_t Notify;        //�ص�����
};

//���ṹ
struct _AtUsart{
  //�ڲ����� 
  struct _UsartDev *pUsartDev; //�ҽӵĵײ��豸   
  unsigned char DevId;         //�豸ID��
  unsigned char Flag;          //��ر�־,������
  unsigned char SendFlag;       //������ر�־,������
  unsigned char RcvFlag;       //������ر�־,������
  
  unsigned short SendTimer;      //���Ͷ�ʱ��ֵ
  unsigned short RcvTimer;      //���ն�ʱ��ֵ
  unsigned short RcvWaitOv;     //���յȴ�ʱ��, 0һֱ�ȴ�
  unsigned short RcvDoingOv;    //�����г�ʱʱ��,0һֱ����ֱ����������  
  //������  
  struct _AtUsartBuf Send;      //������
  struct _AtUsartBuf Rcv;       //������
};

//��ر�־����Ϊ:
#define AT_USART_HALF_MODE_MASK   0xC0   //����ģʽ,����Ϊ:
#define AT_USART_HALF_DUPLEX      0x80   //��ʶ�ڰ�˫������״̬(����һ��������)
#define AT_USART_WR_AUTO_RCV      0x40   //�������Զ��ý���,����ֹͣ��


//������ر�־����Ϊ:
#define AT_USART_SEND_DIS_AT     0x80    //���Զ���ǰ���ַ�"AT"
#define AT_USART_SEND_DIS_CR     0x40    //���Զ��Ӻ��ַ�"<CR>"
#define AT_USART_SEND_DIS_LF     0x20    //���Զ��Ӻ��ַ�"<LF>"
#define AT_USART_SEND_DIS_ALL    0xE0    //���Զ�������ǰ���ַ�(��ʵ��͸��)
//�������:
#define AT_USART_SEND_STATE_MASK   0x03   //����״̬,����Ϊ:
#define AT_USART_SEND_STATE_IDIE   0x00   //����
#define AT_USART_SEND_STATE_DOING  0x01   //������
#define AT_USART_SEND_STATE_FINAL  0x03   //�������

//������ر�־����Ϊ:
//����ʶ�����:
#define AT_USART_RCV_DIS_SCR     0x80     //���Զ�ʶ��ǰ���ַ�"<CR>"
#define AT_USART_RCV_DIS_SLF     0x40     //���Զ�ʶ��ǰ���ַ�"<LF>"
#define AT_USART_RCV_DIS_ECR     0x20     //���Զ�ʶ����ַ�"<CR>"
#define AT_USART_RCV_DIS_ELF     0x10     //���Զ�ʶ����ַ�"<LF>"
#define AT_USART_RCV_DIS_ALL     0xF0    //���Զ�������ǰ���ַ�(��ʵ��ȫ����)
//�������:
#define AT_USART_RCV_ALL_MODE           0x08   //��ʶ�������ֽ���״̬
  
#define AT_USART_RCV_STATE_MASK         0x07   //����״̬,����Ϊ:
#define AT_USART_RCV_STATE_IDIE         0x00   //����
#define AT_USART_RCV_STATE_WAIT         0x01   //���յȴ���
#define AT_USART_RCV_STATE_WAIT_START2  0x02   //����ǰ���ַ�2
#define AT_USART_RCV_STATE_DOING        0x03   //������
#define AT_USART_RCV_STATE_DOING_END2   0x04   //�����н����ַ�2 
#define AT_USART_RCV_STATE_FINAL        0x07   //�������

/******************************************************************************
		                           ��غ���-ϵͳ���
******************************************************************************/

//-------------------------------��ʼ������---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
//�շ�����ǰ����ָ��������
void AtUsart_Init(struct _AtUsart *pAtUsart,
                  struct _UsartDev *pUsartDev,  //�ѳ�ʼ����ɵĵײ��豸
                  unsigned char DevId,         //�豸ID��
                  unsigned char ModeMask);   //AT_USART_HALF_MODE_MASK����      

//---------------------------1msӲ������������---------------------------------
//����Ӳ����ʱ����
void AtUsart_1msHwTask(struct _AtUsart *pAtUsart);

//-----------------------------���÷��ͻ���������-------------------------------
//��ʼ�������
void AtUsart_CfgSend(struct _AtUsart *pAtUsart,
                     unsigned short Count,         //��������С
                     unsigned char *pBuf,          //������
                     AtUsartNotify_t Notify);    //�ص�����

//-----------------------------���ý��ջ���������-------------------------------
//ȫ˫ʽģʽ�������÷��ͻ��������������
void AtUsart_CfgRcv(struct _AtUsart *pAtUsart,
                    unsigned short Count,          //��������С
                    unsigned char *pBuf,           //������ 
                    AtUsartNotify_t Notify);    //�ص�����

//------------------------��ģ��UsartDev��������жϵ��ú���ʵ��--------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
//�������ж�
signed char AtUsart_UsartDevSendEndNotify(void *pVoid);

/******************************************************************************
		                     �շ����ݲ�������
******************************************************************************/
//--------------------------------���Ͳ���--------------------------------------
//�������ã�AT_USART_SEND_DIS_ALL��
void AtUsart_SendCfg(struct _AtUsart *pAtUsart, unsigned char Cfg);
//�õ����ͻ�����,��ATָ��ʱ�����Զ���ָ���ʼ
unsigned char *AtUsart_pGetSendBuf(const struct _AtUsart *pAtUsart);
//�õ����ͻ�������С,��ǰ��׺ʱ�����ų�
unsigned short AtUsart_GetSendCount(const struct _AtUsart *pAtUsart);
//��������,����ǰ����ǰд�뻺����
void AtUsart_SendBuf(struct _AtUsart *pAtUsart, unsigned short SendLen);
//ǿ��ֹͣ��������
void AtUsart_SendStop(struct _AtUsart *pAtUsart);
//�Ƿ����,������������ʱ��ѯ
signed char  AtUsart_IsSendFinal(const struct _AtUsart *pAtUsart);
//���Զ�����
#define AtUsart_EnWrAutoRcv(atUsart) do{(atUsart)->Flag |= AT_USART_WR_AUTO_RCV;}while(0) 
//ȡ���Զ�����
#define AtUsart_DisWrAutoRcv(atUsart) do{(atUsart)->Flag &= ~AT_USART_WR_AUTO_RCV;}while(0) 
//--------------------------------���ղ���--------------------------------------
//�������ã�AT_USART_RCV_DIS_ALL��
void AtUsart_RcvCfg(struct _AtUsart *pAtUsart, unsigned char Cfg);
//�ý������ݵȴ�ʱ��,��ʱ�����ûص�, ��1msΪ��λ
#define AtUsart_SetRcvWaitOv(atUsart, ms) do{(atUsart)->RcvWaitOv = ms;}while(0)
//�ý������ݹ����г�ʱʱ��,��ʱ��Ϊһ֡���ݽ���, ��1msΪ��λ
#define AtUsart_SetRcvDoingOv(atUsart, ms) do{(atUsart)->RcvDoingOv = ms;}while(0)
//��ʼ��������
void AtUsart_RcvStart(struct _AtUsart *pAtUsart);
//ǿ��ֹͣ��������
void AtUsart_RcvStop(struct _AtUsart *pAtUsart);

//�õ����ջ�����,��ǰ��ʱ�����Զ���ǰ����ʼ
const unsigned char *AtUsart_pGetRcvBuf(const struct _AtUsart *pAtUsart);
//�õ����ջ�������С,��ǰ��׺ʱ�����ų�
unsigned short AtUsart_GetRcvCount(const struct _AtUsart *pAtUsart);
//������ȷʱ���õ����յ������ݴ�С
unsigned short AtUsart_GetRcvSize(const struct _AtUsart *pAtUsart);
//�Ƿ����,������������ʱ��ѯ
signed char  AtUsart_IsRcvFinal(const struct _AtUsart *pAtUsart);

/******************************************************************************
		                          �ص�����
******************************************************************************/

//-------------------------������ɻص���������˵��-----------------------------
//pAtUsart->Send.Notify(, State)
//���ж������,�����Ƿ�������գ� State����Ϊ��
//0: ���,1: ���ݳ�ʱδ������

//---------------------------������ɻص�����-----------------------------------
//pAtUsart->Rcv.Notify(, State)
//���ж������,�����Ƿ�������գ� State����Ϊ��
//Err����Ϊ:
// 0: ��ȷ
// 1: ���յȴ�����Ӧ(���޿�ʼ�ַ�)
// 2: ���ݽ��ճ�ʱ(û�н����ַ�������)
//��ֵΪ���մ���״̬��ϵͳ�쳣
//-1: ������ʼ�ַ�1����
//-2: ������ʼ�ַ�2����                       
//-3: ���ս�������2����
//����: �ײ�ͨѶ�쳣
//�����Ƿ����½���

//------------------------��ʼ�����ַ�ͨ������--------------------------------
//�����ڵ�������ָʾ��
//void AtUsart_cbSendStartNotify(unsigned char DevId);//�豸ID��
#define AtUsart_cbSendStartNotify(devId) do{}while(0) //������ʱ

//--------------------------------���ͽ���ͨ������------------------------------
//�����ڹرշ���ָʾ��
//void AtUsart_cbSendFinalNotify(unsigned char DevId);//�豸ID��
#define AtUsart_cbSendEndNotify(devId) do{}while(0) //������ʱ

//----------------------���յ���Ч��ʼ�ַ���ͨ������----------------------------
//�����ڵ�������ָʾ��
//void AtUsart_cbRcvValidNotify(unsigned char DevId);//�豸ID��
#define AtUsart_cbRcvValidNotify(devId) do{}while(0) //������ʱ

//--------------------------------���ս���ͨ������------------------------------
//�����ڹرս���ָʾ��
//void AtUsart_cbRcvEndNotify(unsigned char DevId);//�豸ID��
#define AtUsart_cbRcvEndNotify(devId) do{}while(0) //������ʱ

#endif




