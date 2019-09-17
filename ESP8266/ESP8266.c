/***********************************************************************

                  Sms�û�ͨѶ��
***********************************************************************/

#include "ESP8266.h"
#include "AtCmd.h"
#include "MemMng.h"
#include <string.h>

struct _ESP8266 *pESP8266 = NULL; //������

/******************************************************************************
		                        ��غ���
******************************************************************************/

//-------------------------------д�ص�ʵ��---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
static signed char _AtUsartWrNotify(const void *pv,
                                     signed char State)
{ 
  pESP8266->HwWrResume = State;
  return 0;
}

//-------------------------------���ص�ʵ��---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
static signed char _AtUsartRdNotify(const void *pv,
                                     signed char State)
{
  pESP8266->HwRdResume = State;
  return 0;
}

//------------------------------�õ������---------------------------------
//ֻ�ܶ�һ��
static signed char _GetRdRusume(void)
{
  signed char Resume = pESP8266->HwRdResume;
  pESP8266->HwRdResume = 127;//���긴λ
  return Resume;
}
//------------------------------�õ�д���---------------------------------
//ֻ�ܶ�һ��
static signed char _GetWrRusume(void)
{
  signed char Resume = pESP8266->HwWrResume;
  pESP8266->HwWrResume = 127;//���긴λ
  return Resume;
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
  AtUsart_Init(&pESP8266->AtUsart, pUsartDev, DevId, 0); //�Զ��õ�����
  AtUsart_CfgSend(&pESP8266->AtUsart, ESP8266_WR_BUF_SIZE, 
                  pESP8266->WrBuf, _AtUsartWrNotify);
  AtUsart_CfgRcv(&pESP8266->AtUsart, ESP8266_RD_BUF_SIZE, 
                  pESP8266->RdBuf, _AtUsartRdNotify);
}
                                  
//-----------------------�ж��ַ��Ƿ�ΪOK����------------------------------------
static signed char _IsOk(const char *pStr)
{
  if(*pStr++ != 'O') return 0;
  if(*pStr++ != 'K') return 0;
  return 1;
}

/******************************************************************************
		                          ��ʼ��ģʽ���
******************************************************************************/
//-----------------------�ڲ�ʹ�õ������ָ��------------------------------------
//����AT����
static const char _Rst[] =             {"+RST"};
static const char _DisReturn[] =       {"E0"};   //��ֹ����
static const char _GetIp[] =           {"+=CIFSR"};   //�õ�����IP
static const char _AutoSetWifi[] =     {"+CWSMARTSTART=2"};   //����������ʽ
//����ģʽ
static const char _SetCWMODE_Sta[] =   {"+CWMODE=1"};   //���ù���ģʽΪSTA
static const char _SetCWMODE_AP[] =    {"+CWMODE=2"};   //���ù���ģʽΪAP
static const char _SetCWMODE_StaAP[] = {"+CWMODE=3"};   //���ù���ģʽΪSTA+AP
static const char * const _pSetCWMODE[] = {
  _SetCWMODE_Sta, //�ر�ʱ
  _SetCWMODE_Sta,
  _SetCWMODE_AP,
  _SetCWMODE_StaAP,
};
//��ʼ��ģʽ��״̬��Ӧ�ַ�
static const char * const _pInitStr[] = {
  _Rst,
  _DisReturn,
  NULL,
  _GetIp,
  _AutoSetWifi,  
};

//-----------------------��ʼ��ģʽд����------------------------------------
static void _InitModeWr(void)
{
  const char *pCmd;
  if(pESP8266->ModeState == 2)//���ù���ģʽ
    pCmd = _pSetCWMODE[pESP8266->Flag & ESP8266_CWMODE_MASK];
  else pCmd = _pInitStr[pESP8266->ModeState];
  unsigned char RdCfg;
  if(pESP8266->ModeState == 0) RdCfg = AT_USART_RCV_DIS_ALL; //��λʱ�ַ�������ǰ��
  else RdCfg = 0; 
  AtCmd_RwAtStart(&pESP8266->AtUsart, pCmd, RdCfg); 
  pESP8266->Flag |= ESP8266_RD_WAIT;
  if(pESP8266->ModeState == 0) pESP8266->Timer = 80; //��λʱҪ��Щ
  else pESP8266->Timer = 8;// 1sΪ��λ
}

//-----------------------��ʼ��ģʽ����麯��------------------------------------
static void _InitModeRd(void)
{
  //���������ȴ������
  if(pESP8266->ModeState == 5){
    if(_GetRdRusume()){//û�н��,�����ȴ�
      pESP8266->Timer = 8;// 1sΪ��λ���
    }
    else{//�з�����
      if(_IsOk((char*)AtUsart_pGetRcvBuf(&pESP8266->AtUsart))){//OK������ת��
        pESP8266->ModeState = 3;//��IP��Ϣ
        _InitModeWr();
      }
      else{//�����ַ���λ���ɹ�,���¿�ʼ
        pESP8266->Flag &= ~ESP8266_HW_RDY_MASK;
        pESP8266->ModeState = 0;
      }
    }
    return;
  }
  
  //������д���
  if(_GetRdRusume() || _GetWrRusume()){//ʧ������д
    _InitModeWr();
    return;
  }
  
  //�������״̬
  unsigned short RcvSize = AtUsart_GetRcvSize(&pESP8266->AtUsart);
  const char *pStr = (char*)AtUsart_pGetRcvBuf(&pESP8266->AtUsart);
  
  //��λ�ɹ�ʱ�������ַ���Ϊ�豸��Ϣ
  if(pESP8266->ModeState == 0){//����ֻ��鳤��
    if(RcvSize >= 100) pESP8266->ModeState = 1;
  }
  //ATE0����OK�ɹ�
  else if((pESP8266->ModeState == 1)){
    if(_IsOk(pStr)){
      pESP8266->ModeState = 2;
      pESP8266->Flag |= ESP8266_HW_RDY;
    }
  }
  //дģʽ�ɹ�����OK
  else if((pESP8266->ModeState == 2)){
    if(_IsOk(pStr)){
      unsigned char CmMode = pESP8266->Flag & ESP8266_CWMODE_MASK;
      if(CmMode >= 2)//APģʽ��ʼ�����
        pESP8266->Flag |= ESP8266_AP_RDY;
      if(CmMode == 2){//APģʽǿ�µ�����ģʽ
        pESP8266->CurMode = ESP8266_MODE_CFG;
        return;
      }
      //������STAģʽ������
      pESP8266->ModeState = 3; 
    }
  }
  //����IP��Ϣ(ͬʱWIFI�ɹ�)
  else if((pESP8266->ModeState == 3)){
    if(RcvSize >= 10){//IP�ɹ���,��������ģʽ
      pESP8266->Flag |= ESP8266_WIFI_RDY;
      pESP8266->CurMode = ESP8266_MODE_CFG;
      pESP8266->ModeState = 0;  
      return;
    }
    else pESP8266->ModeState = 4;//��������״̬
  }
  //��������ָ����,OKָ��ɹ�
  else if((pESP8266->ModeState == 4)){
    if(_IsOk(pStr)){
      AtCmd_RdStart(&pESP8266->AtUsart,0,0,0);//ATģʽȫ��
      pESP8266->Timer = 80;//�״ε�10s
      pESP8266->ModeState = 5;
      return;//�ȴ����
    }
  }
  _InitModeWr(); //д����
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
  
  //���ݹ���״ִ̬������
  switch(pESP8266->CurMode){
    case ESP8266_MODE_INIT:  //д�귵��ʱ,�����
      if((pESP8266->Flag & ESP8266_RD_WAIT)) _InitModeRd();
      else _InitModeWr(); break;
    default: break;
  }
}

/******************************************************************************
		                          ����ģʽ���
******************************************************************************/
#include "UsartMng.h"
//----------------------���յ���Ч��ʼ�ַ���ͨ������----------------------------
//�����ڵ�������ָʾ��
void AtUsart_cbRcvValidNotify(unsigned char DevId)//�豸ID��
{
  UsartMng_cbSetLight(DevId);
}

//--------------------------------���ս���ͨ������------------------------------
//�����ڹرս���ָʾ��
void AtUsart_cbRcvEndNotify(unsigned char DevId)//�豸ID��
{
  UsartMng_cbClrLight(DevId);   
}


