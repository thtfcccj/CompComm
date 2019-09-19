/***********************************************************************

                  ESP8266��������
ȫ˫������
***********************************************************************/

#include "ESP8266.h"
#include "ESP8266_Private.h"
#include "AtCmd.h"
#include "MemMng.h"
#include "StringEx.h"
#include <string.h>

struct _ESP8266 *pESP8266 = NULL; //������

/******************************************************************************
		                        ��غ���
******************************************************************************/

//---------------------------�ͷű�������豸----------------------------------
//�����Ƿ�ɹ��ͷ�
signed char ESP8266_RealseUsartDev(struct _UsartDev *pUsartDev)
{
  if(pESP8266->OrgUsartDev.pVoid == NULL) return 0;//û��͸��ģʽ
  memcpy(pESP8266->AtUsart.pUsartDev,
         &pESP8266->OrgUsartDev, 
         sizeof(struct _UsartDev));
  return 1;
}

//---------------------------�ͷź����»�ñ�������豸------------------------
void ESP8266_ReGetUsartDev(struct _UsartDev *pUsartDev)
{
  //����������
  memcpy(&pESP8266->OrgUsartDev, pUsartDev, sizeof(struct _UsartDev));
}

//-------------------------------д�ص�ʵ��---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
static signed char _AtUsartWrNotify(const void *pv,
                                     signed char State)
{ 
  pESP8266->HwWrResume = State;
  #ifdef SUPPORT_ESP8266_BASE
    pESP8266->Base.CommCount++; 
  #endif
  return 0;
}

//-------------------------------���ص�ʵ��---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
static signed char _AtUsartRdNotify(const void *pv,
                                     signed char State)
{
  pESP8266->HwRdResume = State;
  //͸��ģʽ����ִ��
  if((pESP8266->CurMode == ESP8266_MODE_TCP_PASS) && 
     (pESP8266->ModeState == ESP8266_MODE_PASS_DOING))
    pESP8266->Flag |= ESP8266_PASS_RCV_FINAL;
  
  #ifdef SUPPORT_ESP8266_BASE
    if(State) pESP8266->Base.InvalidCount++; 
    else pESP8266->Base.ValidCount++;
  #endif
  
  return 0;
}

//------------------------------�õ������---------------------------------
//ֻ�ܶ�һ��
signed char ESP8266_GetRdRusume(void)
{
  signed char Resume = pESP8266->HwRdResume;
  pESP8266->HwRdResume = 127;//���긴λ
  return Resume;
}

//------------------------------�õ�д���---------------------------------
//ֻ�ܶ�һ��
signed char ESP8266_GetWrRusume(void)
{
  signed char Resume = pESP8266->HwWrResume;
  pESP8266->HwWrResume = 127;//���긴λ
  return Resume;
}

//----------------------------����ͨѶ---------------------------------
void _ReStartComm(void)
{
  pESP8266->Flag &= ~(ESP8266_HW_RDY_MASK | ESP8266_PASS_RCV_FINAL | ESP8266_RD_WAIT);
  pESP8266->CurCommErrIndex = 0;
  pESP8266->CurMode = ESP8266_MODE_INIT;
  pESP8266->ModeState = ESP8266_MODE_INIT_ENTER;//��ESP8266�м��书�ܣ��ʿ���ֱ�Ӿ�������
  pESP8266->Timer = 0;//�����ڿ�ʼ
}     
         
//-------------------------------��ʼ������---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
void ESP8266_Init(struct _UsartDev *pUsartDev, //�ѳ�ʼ����ɵĵײ��豸 
                  unsigned char DevId,         //�豸���ص�ID��
                  unsigned char CwMode,       //ESP8266����ģʽ,0��,1:STA 2:AP 3:AP+STA
                  unsigned char PreMode)     //��ģ��Ԥ�õĹ���ģʽ 
{
  //����struct _SenMng�ڴ�
  pESP8266 = MemMng_pvMalloc(sizeof(struct _ESP8266));
  memset(pESP8266, 0, sizeof(struct _ESP8266));
  //��ʼ�����
  pESP8266->Flag = CwMode;
  pESP8266->PreMode = PreMode; 
  
  #ifdef SUPPORT_ESP8266_BASE
    pESP8266->Base.ComId = DevId;
  #endif
  
  ESP8266_ReGetUsartDev(pUsartDev); //�Ȼ�ȡ����Ȩ
  AtUsart_Init(&pESP8266->AtUsart, pUsartDev, DevId, 0); //�Զ��õ�����
  AtUsart_CfgSend(&pESP8266->AtUsart, ESP8266_WR_BUF_SIZE, 
                  pESP8266->WrBuf, _AtUsartWrNotify);
  AtUsart_CfgRcv(&pESP8266->AtUsart, ESP8266_RD_BUF_SIZE, 
                  pESP8266->RdBuf, _AtUsartRdNotify);
  //��ʼͨѶ
  _ReStartComm();
}
                                  
//-----------------------�ж��ַ��Ƿ�ΪOK����------------------------------------
signed char ESP8266_IsOk(const char *pStr)
{
  if(*pStr++ != 'O') return 0;
  if(*pStr++ != 'K') return 0;
  return 1;
}

//----------------------------����ַ�------------------------------------
const char En_OK[] =                    {"OK"};
const char En_PassExit[] =              {"+++"};

//-----------------------��ý��յ����ַ�------------------------------------
//�Զ���β�����ӽ����ַ�
char *ESP8266_pGetRcvStr(unsigned short RcvSize)
{
  char *pStr = (char*)AtUsart_pGetRcvBuf(&pESP8266->AtUsart);
  if(RcvSize < (ESP8266_RD_BUF_SIZE - 1)) //ǿ�����ӽ����ַ�
    *(pStr + RcvSize) = '\0'; 
  else  *(pStr + RcvSize - 1) = '\0';
  return pStr;
}

//-----------------------����������-------------------------------------
//���ڽ�����ɨ��
void ESP8266_FastTask(void)
{
  if(pESP8266 == NULL) return; //δ�ҽ�
  //ֻ����͸��ģʽ����ִ��
  if(!(pESP8266->Flag & ESP8266_PASS_RCV_FINAL)) return;
  pESP8266->Flag &= ~ESP8266_PASS_RCV_FINAL;
  ESP8266_PassModeRd();
}

//---------------------------������-------------------------------------
//128ms������һ��
void ESP8266_Task(void)
{
  if(pESP8266 == NULL) return; //δ�ҽ�
  
  if((pESP8266->Flag & ESP8266_CWMODE_MASK) == 0) return;//�ر�ʱ������
  
  //���յȴ��� 
  if(pESP8266->Timer){
    pESP8266->Timer--;
    return;
  }
  
  //=======================���ݹ���״ִ̬������=======================
  switch(pESP8266->CurMode){
    case ESP8266_MODE_INIT:  //��ʼ��ģʽ
      if((pESP8266->Flag & ESP8266_RD_WAIT)) //д�귵��ʱ,�����
        ESP8266_InitModeRd();
      else ESP8266_InitModeWr(); break;
    case ESP8266_MODE_TCP_PASS:  //TCP͸��ģʽ  
      if((pESP8266->Flag & ESP8266_RD_WAIT))//д�귵��ʱ,�����
        ESP8266_PassModeRd();
      else ESP8266_PassModeWr(); break;      
    default: break;
  }
  
  
  //============����̬ʱ����ͬ״̬�ﵽһ���׶���������״̬��ֹ�쳣========
  if(!(pESP8266->Flag & ESP8266_PASS_RCV_FINAL) && 
     (pESP8266->CurMode != ESP8266_MODE_CFG)){
    if(pESP8266->ModeState == pESP8266->PrvModeState){
      pESP8266->CurCommErrIndex++;
      if(pESP8266->CurCommErrIndex == 255){//��ʱ����
        _ReStartComm();
      }
    }
    else{
      pESP8266->PrvModeState = pESP8266->ModeState;
      pESP8266->CurCommErrIndex = 0;
    }
  }
  else  pESP8266->CurCommErrIndex = 0;
  
  
}

/******************************************************************************
		                          ����ģʽ���
******************************************************************************/
#include "UsartMng.h"
//----------------------���յ���Ч��ʼ�ַ���ͨ������----------------------------
//�����ڵ�������ָʾ��
void AtUsart_cbRcvValidNotify(unsigned char DevId)//�豸ID��
{
  //��������ģʽ������ʾ
  if((pESP8266->CurMode == ESP8266_MODE_INIT) &&
     (pESP8266->ModeState >= ESP8266_MODE_INIT_SMART_START))
    UsartMng_cbClrLight(DevId);
  else UsartMng_cbSetLight(DevId);
}

//--------------------------------���ս���ͨ������------------------------------
//�����ڹرս���ָʾ��
void AtUsart_cbRcvEndNotify(unsigned char DevId)//�豸ID��
{
  //��������ģʽ������ʾ
  if((pESP8266->CurMode == ESP8266_MODE_INIT) &&
     (pESP8266->ModeState >= ESP8266_MODE_INIT_SMART_START))
    UsartMng_cbSetLight(DevId);
  else UsartMng_cbClrLight(DevId);   
}


