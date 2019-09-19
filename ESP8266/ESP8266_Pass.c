/***********************************************************************

                  ESP8266��������-͸��ģʽʵ��

***********************************************************************/

#include "ESP8266.h"
#include "ESP8266_Private.h"
#include "AtCmd.h"
#include "MemMng.h"
#include "StringEx.h"
#include <string.h>

/******************************************************************************
		                     ����TCP͸��ģʽ���
******************************************************************************/
//-----------------------�ڲ�ʹ�õ������ָ��------------------------------------
//����AT����
static const char _Dot3[] =           {"..."};
static const char _Port[] =           {"Port"};
static const char _CONNECTED[] =           {"CONNECTED"};
static const char _EnterPass[] =           {"\r\n>"};
static const char _PassDataRdy[] =           {"Pass Rdy!"};

static const char _SetServer[] =           {"+CIPSTART=\"TCP\",\"...\",Port"};
static const char _PassRdy[] =             {"+CIPMODE=1"};
static const char _PassEnter[] =           {"+CIPSEND"};

//��ģʽ��״̬��Ӧ�ַ�
static const char * const _pPassStr[] = {
  _SetServer, 
  _PassRdy,
  _PassEnter,
  NULL,
  En_PassExit,
};

//��״̬λ���ұ�,��4bitΪDisALL
static const unsigned char  _PassLut[] = {
  AT_USART_RCV_DIS_ALL,//���������������������
  0,                   //�ɹ�����OK
  AT_USART_RCV_DIS_ALL,//����󣬷���OK,�ٷ���<
  AT_USART_RCV_DIS_ALL,   //͸����
  AT_USART_RCV_DIS_ALL,//�˳�����
};

//-----------------------͸��ģʽд����------------------------------------
void ESP8266_PassModeWr(void)
{
  const char *pCmd;
  if(pESP8266->ModeState == ESP8266_MODE_PASS_SERVER){//����������ʱ
    //��֯�ַ���
    char *pBuf = (char*)AtUsart_pGetSendBuf(&pESP8266->AtUsart);    
    memcpy(pBuf, _SetServer, sizeof(_SetServer));
    //IP��ַ�滻
    char *pCurBuf = (char*)pESP8266->RdBuf; //��ʱ���ô˻���
    
    const unsigned char *pIp = pESP8266_cbGetGlobalServerIp(pESP8266->AtUsart.DevId);
    if(pIp == NULL){//����ʱ
      unsigned char LocalServerIp[4];
      memcpy(LocalServerIp, pESP8266->LocalIp, 3);
      LocalServerIp[3] = ESP8266_cbGetLocalServerIpLowest(pESP8266->AtUsart.DevId);;
      Ip4ToStr(LocalServerIp, pCurBuf);
    }
    else{ //ȫ��ʱ
      Ip4ToStr(pIp,pCurBuf);
    }
      
    StringReplace(pBuf, _Dot3, pCurBuf);
    //�˿��滻(<=32767)
    Value2StringMin(ESP8266_cbGetServerPort(pESP8266->AtUsart.DevId),pCurBuf, 1);
    StringReplace(pBuf, _Port, pCurBuf);
    pCmd = pBuf;
    pESP8266->Timer = 50; //ʱ�䳤��
  }
  else{
    pCmd = _pPassStr[pESP8266->ModeState];
    if(pCmd == NULL) return; //�쳣
    if(pESP8266->ModeState == ESP8266_MODE_PASS_ENTER) 
      pESP8266->Timer = 30;//�ȴ�ʱ�䳤��
    else pESP8266->Timer = 8;// 1sΪ��λ
  }
  AtCmd_RwAtStart(&pESP8266->AtUsart, pCmd, 
                  _PassLut[pESP8266->ModeState] & AT_USART_RCV_DIS_ALL); 
  pESP8266->Flag |= ESP8266_RD_WAIT;
}

//-------------------------͸��ģʽ����麯��----------------------------------
void ESP8266_PassModeRd(void)
{
  unsigned short RcvSize = AtUsart_GetRcvSize(&pESP8266->AtUsart);
  //͸��ģʽ�ȴ���
  if(pESP8266->ModeState == ESP8266_MODE_PASS_DOING){
    pESP8266->Timer = 255;//��ȴ�
    if(ESP8266_GetRdRusume()) return;//������
    unsigned char *pBuf = AtUsart_pGetRcvBuf(&pESP8266->AtUsart);
    if(RcvSize){//�յ�����ʱ
      RcvSize = ESP8266_cbPassEncoder(pBuf, RcvSize, ESP8266_WR_BUF_SIZE);
      if(RcvSize){//�л���ʱֱ��͸�����ͳ�ȥ
        AtCmd_WrStart(&pESP8266->AtUsart, pBuf,RcvSize, AT_USART_SEND_DIS_ALL);
        //ȫ˫�����ڷ���ʱ����
      }
    }
    //�ٴ������ڲ�ȫ����
    AtCmd_RdStart(&pESP8266->AtUsart,AT_USART_RCV_DIS_ALL, 0, 200);
    return;
  }
  
  char *pStr = ESP8266_pGetRcvStr(RcvSize);

  //������д���
  if(ESP8266_GetRdRusume() || ESP8266_GetWrRusume()){//ʧ������д
    ESP8266_PassModeWr();
    return;
  }
  
  //���ӷ�����״̬���
  if(pESP8266->ModeState == ESP8266_MODE_PASS_SERVER){
    if(StrFind(pStr, _CONNECTED) != NULL)//�����ӱ�ʶ�� ������ת��
      pESP8266->ModeState = ESP8266_MODE_PASS_RDY;
  }
  //����Ϊ͸��ģʽ,����OK
  else if((pESP8266->ModeState == ESP8266_MODE_PASS_RDY)){
    if(ESP8266_IsOk(pStr))
      pESP8266->ModeState = ESP8266_MODE_PASS_ENTER;
  }
  //��ʼ͸��,����<
  else if((pESP8266->ModeState == ESP8266_MODE_PASS_ENTER)){
    if(StrFind(pStr, _EnterPass) != NULL){//�л��м�>��ʶ�� ������ת��
      pESP8266->ModeState = ESP8266_MODE_PASS_DOING; 
      if(!ESP8266_RealseUsartDev(pESP8266->AtUsart.pUsartDev)){//û�н�������Ȩʱ
        //ͨ��׼�����Խ���
        AtCmd_WrStart(&pESP8266->AtUsart, (const unsigned char*)_PassDataRdy,
                      sizeof(_PassDataRdy) - 1, AT_USART_SEND_DIS_ALL);
        AtCmd_RdStart(&pESP8266->AtUsart,AT_USART_RCV_DIS_ALL, 0, 200);//�����ڲ�ȫ����
        pESP8266->Timer = 255;//��ȴ�
      }
      return;
    }
  }
  //͸���˳�
  else if((pESP8266->ModeState == ESP8266_MODE_PASS_EXIT)){
    if(ESP8266_IsOk(pStr)){
      pESP8266->ModeState = 0;
      pESP8266->CurMode = ESP8266_MODE_CFG; //ǿ���˵�����ģʽ
    }
  }

  ESP8266_PassModeWr(); //д����
}




