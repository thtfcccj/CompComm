/***********************************************************************

                  ESP8266��������-��ʼ��ģʽʵ��

***********************************************************************/

#include "ESP8266.h"
#include "ESP8266_Private.h"
#include "AtCmd.h"
#include "MemMng.h"
#include "StringEx.h"
#include <string.h>

/******************************************************************************
		                          ��ʼ��ģʽ���
******************************************************************************/

//--------------------------��䱾��IP------------------------------------
//pStr����:+CIFSR:STAIP,"192.168.88.152"+CIFSRSTAMAC,
const char En_DotDp[] =                 {",\""};
static void _FullLocalIp(char *pStr)
{
  pStr = StrFind(pStr, En_DotDp);
  if(pStr == NULL) return; //�쳣
  StrToIp4(pStr, pESP8266->LocalIp);
}

//-----------------------�ڲ�ʹ�õ������ָ��------------------------------------
//����AT����
static const char _Rst[] =             {"+RST"};
static const char _DisReturn[] =       {"E0"};   //��ֹ����
static const char _GetIp[] =           {"+CIFSR"};   //�õ�����IP
static const char _AutoSetWifi[] =     {"+CWSMARTSTART=2"};   //����������ʽ
//����ģʽ
static const char _GetCWMODE_Sta[] =   {"+CWMODE?"};   //�õ�����ģʽ
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
  En_PassExit,
  _Rst,
  _DisReturn,
  _GetCWMODE_Sta,
  NULL,
  _GetIp,
  _AutoSetWifi,  
};

//��ʼ��ģʽ��״̬λ���ұ�,��4bitΪDisALL
static const unsigned char  _InitLut[] = {
  AT_USART_RCV_DIS_ALL,//ǿ���˳�͸��ģʽ����������  
  AT_USART_RCV_DIS_ALL,//������Ϣ����������
  0,                   //���Է��ط���OK
  AT_USART_RCV_DIS_ALL,//����+CWMODE����������
  0,                   //����OK
  AT_USART_RCV_DIS_ALL,//,����+CIFSR
  0,                   //����OK�ɹ�
};

//-----------------------��ʼ��ģʽд����------------------------------------
void ESP8266_InitModeWr(void)
{
  //������д+++
  if(pESP8266->ModeState == ESP8266_MODE_INIT_ENTER){
    AtCmd_WrStart(&pESP8266->AtUsart, (const unsigned char*)En_PassExit, 
                  strlen(En_PassExit), AT_USART_SEND_DIS_ALL);
    pESP8266->Timer = 8;// 1sΪ��λ
    pESP8266->Flag |= ESP8266_RD_WAIT;
    return;
  }
  //����ATд
  const char *pCmd;
  if(pESP8266->ModeState == ESP8266_MODE_INIT_SET_CWMODE)//���ù���ģʽ
    pCmd = _pSetCWMODE[pESP8266->Flag & ESP8266_CWMODE_MASK];
  else pCmd = _pInitStr[pESP8266->ModeState];
  if(pESP8266->ModeState == ESP8266_MODE_INIT_RST)
    pESP8266->Timer = 80; //��λʱҪ��Щ
  else pESP8266->Timer = 8;// 1sΪ��λ
  AtCmd_RwAtStart(&pESP8266->AtUsart, pCmd, 
                  _InitLut[pESP8266->ModeState] & AT_USART_RCV_DIS_ALL); 
  
  pESP8266->Flag |= ESP8266_RD_WAIT;
}

//-----------------------��ʼ��ģʽ����麯��----------------------------------
void ESP8266_InitModeRd(void)
{
  //������д+++���
  if(pESP8266->ModeState == ESP8266_MODE_INIT_ENTER){
    pESP8266->ModeState = ESP8266_MODE_INIT_ATE0;//��ESP8266�м��书�ܣ��ʿ���ֱ�Ӿ�������
    pESP8266->Timer = 40; //����ʱҪ��Щ
    ESP8266_InitModeWr();
    return;
  }
  
  //�Ȼ���ַ�
  unsigned short RcvSize = AtUsart_GetRcvSize(&pESP8266->AtUsart);
  char *pStr = ESP8266_pGetRcvStr(RcvSize);
  
  //���������ȴ������
  if(pESP8266->ModeState == ESP8266_MODE_INIT_SMART_WAIT){
    if(ESP8266_GetRdRusume()){//û�н��,�����ȴ�
      pESP8266->Timer = 8;// 1sΪ��λ���
    }
    else{//�з�����
      if(StrFind(pStr, En_OK) != NULL){//��OK�� ������ת��
        pESP8266->Flag |= ESP8266_WIFI_RDY;
        pESP8266->ModeState = ESP8266_MODE_INIT_CIFSR;//��IP��Ϣ
        ESP8266_InitModeWr();
      }
      else{//�����ַ����ɹ�,���¿�ʼ
        
      }
    }
    return;
  }
  
  //������д���
  if(ESP8266_GetRdRusume() || ESP8266_GetWrRusume()){//ʧ������д
    if(!(pESP8266->Flag & ESP8266_HW_RDY) && 
       (pESP8266->ModeState == ESP8266_MODE_INIT_ATE0)){//�˳�͸�����ɹ�ʱ
      pESP8266->ModeState = ESP8266_MODE_INIT_ENTER;
    }
    ESP8266_InitModeWr();
    return;
  }
  
  //��λ�ɹ�ʱ�������ַ���Ϊ�豸��Ϣ
  if(pESP8266->ModeState == ESP8266_MODE_INIT_RST){//����ֻ��鳤��
    if(RcvSize >= 100) pESP8266->ModeState = ESP8266_MODE_INIT_ATE0;
  }
  //ATE0����OK�ɹ�
  else if(pESP8266->ModeState == ESP8266_MODE_INIT_ATE0){
    if(ESP8266_IsOk(pStr)){
      if(pESP8266->Flag & ESP8266_HW_RDY)//�������ý���ʱ
        pESP8266->ModeState = ESP8266_MODE_INIT_SET_CWMODE;
      else //��������ʱ,����ȡIP
        pESP8266->ModeState = ESP8266_MODE_INIT_CIFSR;
    }
  }
  //дģʽ�ɹ�����OK
  else if((pESP8266->ModeState == ESP8266_MODE_INIT_SET_CWMODE)){
    if(ESP8266_IsOk(pStr)){
      unsigned char CmMode = pESP8266->Flag & ESP8266_CWMODE_MASK;
      if(CmMode >= 2)//APģʽ��ʼ�����
        pESP8266->Flag |= ESP8266_AP_RDY;
      if(CmMode == 2){//APģʽǿ�µ�����ģʽ
        pESP8266->CurMode = ESP8266_MODE_CFG;
        return;
      }
      //������STAģʽ������
      pESP8266->ModeState = ESP8266_MODE_INIT_CIFSR; 
    }
  }
  //����IP��Ϣ(ͬʱWIFI�ɹ�)
  else if((pESP8266->ModeState == ESP8266_MODE_INIT_CIFSR)){
    if(RcvSize >= 30){//IP�ɹ���
      _FullLocalIp(pStr);
      if(pESP8266->LocalIp[3] != 0){//IP��Ч�ˣ�����Ϊ0.0.0.0��ʾδ��������
        pESP8266->Flag |= ESP8266_WIFI_RDY | ESP8266_HW_RDY;
        if(pESP8266->PreMode <= ESP8266_MODE_CFG)//ֱ�ӽ�������ģʽ
          pESP8266->CurMode = ESP8266_MODE_CFG;
        else//�����û�ָ��ģʽ
          pESP8266->CurMode = pESP8266->PreMode;
        pESP8266->ModeState = 0; 
        pESP8266->Flag &= ~ESP8266_RD_WAIT; //���¿�ʼ
        return;
      }
    }
    //IP��ȡ���ɹ�
    if(pESP8266->Flag & ESP8266_HW_RDY){//�������ý���ʱ
      if(!(pESP8266->Flag & ESP8266_WIFI_RDY))//û������WIFIʱ
        pESP8266->ModeState = ESP8266_MODE_INIT_SMART_START;//��������״̬
      //else ������ѯ
    }
    else{//��������ʱ
      pESP8266->Flag |= ESP8266_HW_RDY;//����׼��
      pESP8266->ModeState = ESP8266_MODE_INIT_RST;//���븴λ״̬
    }
  }
  //��������ָ����,OKָ��ɹ�
  else if((pESP8266->ModeState == ESP8266_MODE_INIT_SMART_START)){
    if(ESP8266_IsOk(pStr)){
      AtCmd_RdStart(&pESP8266->AtUsart,AT_USART_RCV_DIS_ALL,0,500);//ATģʽȫ��
      pESP8266->Timer = 80;//�״ε�10s
      pESP8266->ModeState = ESP8266_MODE_INIT_SMART_WAIT;
      return;//�ȴ����
    }
  }
  ESP8266_InitModeWr(); //д����
}


