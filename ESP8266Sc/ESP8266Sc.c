/***********************************************************************

        ��ESP8266Station(S����ĸ)��+TCP Client(C����ĸ)ʵ��
 * ע��strcpy�ڴ˺����ڵ���ʱ,��xc8 1.37�б������л����Ӳ���
***********************************************************************/

#include "ESP8266Sc.h"
#include "Eeprom.h"
#include "InfoBase.h"

#include "StringEx.h"
#include <string.h>

struct _ESP8266Sc ESP8266Sc;  //������

/******************************************************************************
		                        ��غ���
******************************************************************************/

//-------------------------------��ʼ������---------------------------------
void ESP8266Sc_Init(signed char Inited)
{
  memset(&ESP8266Sc, 0, sizeof(struct _ESP8266Sc));
  
  if(!Inited){//����MPLABX memcpy�����
    memcpyL((char*)&ESP8266Sc.Info, (const char*)&ESP8266_cbInfoDefault, sizeof(struct _ESP8266ScInfo));
    Eeprom_Wr(ESP8266Sc_GetInfoBase(), 
              &ESP8266_cbInfoDefault, sizeof(struct _ESP8266ScInfo));
  }
  else{
    Eeprom_Rd(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
    #ifdef SUPPORT_PIC//�ڵ���ʱ�������¿�ʼ�����Ϊȫ0,
    if(ESP8266Sc.Info.ServerPort == 0)
      memcpyL((char*)&ESP8266Sc.Info, (const char*)&ESP8266_cbInfoDefault, sizeof(struct _ESP8266ScInfo));
    #endif
  }
}

//---------------------------����������-----------------------------------
//�β�Ϊ�Ƿ������������ģʽ
void ESP8266Sc_ReConfigStart(unsigned char IsSmartConn)
{
  //���б�־һ��λ
  if(IsSmartConn) 
    ESP8266Sc.Flag = ESP8266SC_STATE_RUNNED | ESP8266SC_SEND_RDY | ESP8266SC_SMART_CONN;
  else ESP8266Sc.Flag = ESP8266SC_STATE_RUNNED | ESP8266SC_SEND_RDY;
  ESP8266Sc.eState = ESP8266Sc_eExitPass;
  ESP8266Sc.Timer = 0;//�����ڿ�ʼ
  ESP8266Sc.RetryIndex = 0;
}

//---------------------------״̬��Ϣλ-----------------------------------
//��λ����Ϊ��//0xC0:�ȴ�״̬(0:���ȣ�1�ȴ�ͨѶ��Ӧ��2�ȴ�������Ӧ��3�ȴ��û���Ӧ) 
//0x20:Ҫ������, 0x10����OK���, 0x08 ��ʱ������ֱ�ӽ�����һģʽ
static const unsigned char _StateInfo[] = {
  0x00 | 0x00 | 0x00 | 0x00 | 0x00,   //����״̬
  0x00 | 0x00 | 0x00 | 0x00 | 0x00,   //�˳�͸��ģʽ
  0xC0 | 0x20 | 0x00 | 0x08 | 0x00,   //��λ
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //�رջ���,����OKȷ��ģ�����
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //��Statinģʽ,����OK�ɹ�
  0x40 | 0x20 | 0x00 | 0x00 | 0x00,   //��ȡ����IP��ַ��ȷ��wifi������
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //ת����������ģʽ,�ȴ�OK����
  0xC0 | 0x20 | 0x00 | 0x00 | 0x00,   //���÷�����,����connect�ɹ�
                                     //(���������˳���˲�ʱ��ϳ�,���Զ�β��ܽ���)
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //����Ϊ͸��ģʽ,����OK�ɹ�
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //��ʼ͸��,����OK�ɹ�,����>��ʼ
  0x80 | 0x00 | 0x00 | 0x00 | 0x00,  //͸����ʼ����׸��̶�����
};

//---------------------------״̬gc ״̬λ-----------------------------------
//��λ����Ϊ��0-3Bit:��ȷʱ��һ״̬�� 4-7Bit ����ʱ��һ״̬
//0x20:Ҫ������, 0x10����OK��飬����Ϊ������, 0x0f�´�״̬
static const unsigned char _State2State[] = {
  (ESP8266Sc_eIdie << 4)     | ESP8266Sc_eIdie,         //����״̬
  (ESP8266Sc_eExitPass << 4) | ESP8266Sc_eRst,          //�˳�͸��ģʽ
  (ESP8266Sc_eExitPass << 4) | ESP8266Sc_eIsRdy,        //��λ
  (ESP8266Sc_eExitPass << 4) | ESP8266Sc_eSetStation,   //�رջ���,����OKȷ��ģ�����
  (ESP8266Sc_eRst << 4)      | ESP8266Sc_eIsConn,       //��Statinģʽ,����OK�ɹ�
  (ESP8266Sc_eIsConn << 4)   | ESP8266Sc_eSetServer,    //��ȡ����IP��ַ��ȷ����wifi������
  (ESP8266Sc_eRst << 4)      | ESP8266Sc_eSetServer,    //ת����������ģʽ,�ȴ�OK����
  (ESP8266Sc_eSetServer << 4)| ESP8266Sc_eSetPass,      //���÷�����,����connect�ɹ�
  (ESP8266Sc_eSetPass << 4)  | ESP8266Sc_eStartPass,    //����Ϊ͸��ģʽ,����OK�ɹ�
  (ESP8266Sc_eExitPass << 4)  | ESP8266Sc_ePassData1st,  //��ʼ͸��,��δȷ�ϣ��������¿�ʼ���˳�
  //(ESP8266Sc_eIdie << 4)     | ESP8266Sc_eIdie,         //͸����ʼ����׸��̶�����  
};

//--------------------------�ȴ���Ϣת��Ϊʱ��----------------------------------
static const unsigned char _WaitInfo2Time[] = {//128msΪ��λ,������ʱ��
  2,  //0�ȴ��������
  8,  //1�ȴ�ͨѶӦ��(�緢��ATE0)
  36, //�ȴ�������Ӧ
  200,// 3�ȴ���ʱ������Ӧ(�������ģʽ��λ,��ʱ����WIFI���ɹ��Լ����IP,Ӧ��Ȼ�)
};

//-----------------------״̬����Ӧ��ָ���ַ�������-----------------------------
//->MPLABX��������,����ַ������еĻ�����������Ϊ��ͳһ�����(��෢)һ���ַ��Ա���
static const char _DotSpace[] =   {"\", "};      //����ַ�
static const char _RN[] =   {"\r\n\n"};      //�س�������(��ʱ������ֽڻ��ΪNULL)
static const char _DefaultServer[] = {"192.168.88.101\",\"10001\"\r\n "}; //Ĭ�Ϸ�����
//״̬����Ӧ,�����ַ�
static const char _ExitPass[] =  {"+++ "};         //�˳�͸��ģʽ,ע��û�лس����з�
static const char _Rst[] =       {"AT+RST\r\n "};      //��λ
static const char _DisReturn[] = {"ATE0\r\n "};        //��ֹ����(IsRdy���)
static const char _SetStation[] ={"AT+CWMODE=1\r\n "}; //��STAģʽ
static const char _GetIp[] =     {"AT+CIFSR\r\n "};    //�õ�����IP(IsConn���)
static const char _SmartConn[]=  {"AT+CWSTARTSMART=3\r\n "};//����������ʽ
static const char _SetServer[] =  {"AT+CIPSTART=\"TCP\",\" "}; //���÷�����,������Ϣ
static const char _SetPass[] =   {"AT+CIPMODE=1\r\n "};//����Ϊ͸��ģʽ
static const char _EnterPass[] = {"AT+CIPSEND\r\n "};        //��ʼ͸��

//�°棺
//static const char _SetStation[] ={"AT+CWMODE_DEF=1\r\n "}; //��STAģʽ���������ñ�����flash
//static const char _SetAutoAP[]  ={"AT+CWAUTOCONN=1\r\n "}; //ʹ���ϵ��Զ�����AP

//��ʼ��ģʽ��״̬��Ӧ�ַ�
static const char * const _pCmd[] = {
  _ExitPass,             //�˳�͸��ģʽ
  _Rst,                  //��λ
  _DisReturn,            //�رջ���,�ȴ�OKȷ��ģ�����
  _SetStation,           //��Statinģʽ
  _GetIp,                //��ȡ����IP��ַ��ȷ���������������
  _SmartConn,            //ת����������ģʽ
  _SetServer,            //���÷�����
  _SetPass,              //����Ϊ͸��ģʽ
  _EnterPass,            //��ʼ͸��  
};

//------------------------ǿ���˳���������ģʽ---------------------------------
void ESP8266Sc_ExitSmartConn(void)
{
  if(!(ESP8266Sc.Flag & ESP8266SC_WAIT_SMART_CONN)) return;
  ESP8266Sc.Timer = 0;
  ESP8266Sc.Flag &= ~(ESP8266SC_SMART_CONN | ESP8266SC_WAIT_SMART_CONN);
}

//-------------------------ת����һ״̬����----------------------------------
//��ȷʱ����
static void _ToNextState(unsigned char IsErr)
{
  unsigned char NextState = _State2State[ESP8266Sc.eState];
  if(IsErr) //�д���ʱ������״̬
    ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState >> 4);
  else{//�ù���ģʽ����Ϊ��������ģʽ,�������ӵ�û����WIFIʱ��ǿ��ת����������ģʽ
    if ((ESP8266Sc.eState == ESP8266Sc_eSetStation) && (ESP8266Sc.Flag & ESP8266SC_SMART_CONN) || 
      (ESP8266Sc.eState == ESP8266Sc_eIsConn) && !(ESP8266Sc.Flag & ESP8266SC_WIFI_FINAL)){
      ESP8266Sc.eState = ESP8266Sc_eSmartConn;
    }
    else{ //��ȷʱ����һ״̬
      ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState & 0x0f);
      ESP8266Sc.ErrIndex = 0;//�ܹ���״̬��λ
    }
  }
  ESP8266Sc.RetryIndex = 0;//��һ״̬��λ
  ESP8266Sc.Flag |= ESP8266SC_SEND_RDY;//��һ״̬����    
}

//---------------------------������-------------------------------------
//128ms����һ��
void ESP8266Sc_Task(void)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return; //������
  //����δ��ɻ����������ȴ�ģʽ
  if(ESP8266Sc.Flag & (ESP8266SC_SEND_RDY | ESP8266SC_WAIT_SMART_CONN)) return; 
  
  if(ESP8266Sc.Timer){//ʱ��δ��
    ESP8266Sc.Timer--;
    return;
  }
  
  //�ȴ�ʱ�䵽�ˣ���鳬ʱ���
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if((Info & 0x28) == 0x20){//�������ʱ��û�н�ֹ��ʱ���ʱ����
    ESP8266Sc.RetryIndex++;
    ESP8266Sc.ErrIndex++;
    if(ESP8266Sc.ErrIndex >= ESP8266SC_ERR_COUNT){ //�ܹ���������ʱ��,���¿�ʼ
      ESP8266Sc.eState = ESP8266Sc_eExitPass;
      _ToNextState(0);
    }
    else //������ǰ����ת����״̬
      if(ESP8266Sc.RetryIndex >= ESP8266SC_RETRY_COUNT) _ToNextState(1);
    else ESP8266Sc.Flag |= ESP8266SC_SEND_RDY;//������ǰ״̬����
    return;
  }
  //�ȴ���Ӧʱ�䵽�ˣ���һ״̬
  //͸��ģʽ�׸����ݽ�����
  if(ESP8266Sc.eState == ESP8266Sc_ePassData1st){
    ESP8266Sc.Flag |= ESP8266SC_SERVER_FINAL;
    ESP8266Sc.eState = ESP8266Sc_eIdie;   
    ESP8266Sc_cbConfigFinal(); //ͨ�����
  }
  else _ToNextState(0);  //��һ״̬
}
  
//------------------------����������-------------------------------------
void ESP8266Sc_FastTask(void)
{
  //ֻ������
  if(!(ESP8266Sc.Flag & ESP8266SC_SEND_RDY)) return;
  ESP8266Sc.Flag &= ~ESP8266SC_SEND_RDY;
  
  char *pSendBuf = pESP8266Sc_pcbGetBuf();
  
  //͸��ģʽ�ˣ�����û���ӭ��Ϣ��ֱ�ӽ���״̬
  if(ESP8266Sc.eState == ESP8266Sc_ePassData1st){
    *pSendBuf = ':';//���һ�ַ���ֹ����-1����
    *(pSendBuf + 1) = '\0';//�û�����������ַ�������
    ESP8266Sc_cbFulPassData1st(pSendBuf);   //���û����(�ɲ���伴������)
    goto _FullEnd;
  }
  
  //����ESP8266ָ��
  pSendBuf = strcpyL(pSendBuf, _pCmd[ESP8266Sc.eState - 1]) - 1;//β����
  //*pSendBuf = '\0';//������˻��ƻ�ǰ�����
  //Ҫ���ӷ�������Ϣ
  if(ESP8266Sc.eState == ESP8266Sc_eSetServer){
    if(ESP8266Sc_cbIsDefaultServer()){//Ĭ�Ϸ�����
      strcpyL(pSendBuf, _DefaultServer);
    }
    else{//���õķ�����,��ʽ�磺AT+CIPSTART="TCP","192.168.88.250",10001
      pSendBuf = Ip4ToStr(ESP8266Sc.Info.ServerIp, (char*)pSendBuf);//IP
      pSendBuf = strcpyL(pSendBuf, _DotSpace) - 1;
      pSendBuf = Value2StringMin(ESP8266Sc.Info.ServerPort, pSendBuf, 1);//�˿ں�
      strcpyL(pSendBuf, _RN); //����س������ַ�
    }
  }
  _FullEnd:
  ESP8266Sc_pcbSendBuf(strlen(pESP8266Sc_pcbGetBuf()) - 1); //�����ͳ�,ȥ����ӵ��ַ�
  //����ģʽ��λ��ֱֹ�����ϣ�ֻ�м�Сʱ��ȴ��ܲŽ���
  if((ESP8266Sc.Flag & ESP8266SC_SMART_CONN) && (ESP8266Sc.eState == ESP8266Sc_eRst))
    ESP8266Sc.Timer = 15;
  else  ESP8266Sc.Timer = _WaitInfo2Time[_StateInfo[ESP8266Sc.eState] >> 6];//��ʱ
}

//----------------------------��������ַ�------------------------------
static const char _OK[] =         {"OK"};          //OK�ַ�
static const char _Got[] =         {"GOT"};          //WIFI GOT IP�ַ�
static const char _CONNECT[] =  {"CONNECT"};//���������ӳɹ���־
//����IPǰ׺(����������+CIFSR:STAIP,"192.168.88.152"+CIFSRSTAMAC,)
static const char _DotDp[] =       {",\""}; //���ڼ���ʽ
static const char _NullIp[] =      {"0.0"}; //ʾ����MISTAIP,"0.0.0.0"��ʾ��IP
static const char _ctedWifi[] =      {"cted"}; //�����ɹ������ʾ��smartconfig connected wifi
//static const char _PassEnter[] =   {">"};        //��ʼ͸����־\r\n

//--------------------------�������ݴ���----------------------------------
//����0û����,���������
signed char ESP8266Sc_RcvData(const unsigned char *pData,
                               unsigned char Len)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return 0; //������
  //ע��δ��鳤�ȣ�������
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if(!(Info & 0x20)) return 1; //��������
  *((char*)pData + Len) = '\0';//ǿ�����ӽ����ַ�
  const char *pStr = (const char *)pData;
  //�������Ƿ���ȷ(pStr ��= NULLΪ��ȷ)  
  if(ESP8266Sc.Flag  & ESP8266SC_WAIT_SMART_CONN){//��������ʱ��鷵��״̬
    if(StrFind(pStr, _ctedWifi) == NULL) return 1;
    ESP8266Sc.Flag  &= ~(ESP8266SC_SMART_CONN | ESP8266SC_WAIT_SMART_CONN);//�����ɹ���,����
    ESP8266Sc_cbSmartConnFinal();//���ͨ��
  }
  else if(Info & 0x10){//OK�ַ����
    pStr = StrFind(pStr, _OK);
    if(ESP8266Sc.eState == ESP8266Sc_eSmartConn){//���������ȴ�ģʽ
      ESP8266Sc.Flag |= ESP8266SC_WAIT_SMART_CONN;
      ESP8266Sc_cbEnterSmartConnNotify();//����ͨ��
      return 1;
    }
    if(ESP8266Sc.eState == ESP8266Sc_eStartPass){//��ʼ͸��ʱ���ܴ��OK,�����ʼ
      //if(pStr == NULL) pStr = StrFind((const char *)pData, _PassEnter);
      //if(pStr == NULL) return;//����û�ҵ�������ǰ�����������ȴ�
      pStr = pData;//�޸�ΪֻҪ�յ�������Ϊ����
    }
  }
  else if(ESP8266Sc.eState == ESP8266Sc_eRst){//��λ�ڼ���IPʱ��ǰ�˳�
    if(StrFind(pStr, _Got) == NULL) return 1;  
  }
  else if(ESP8266Sc.eState == ESP8266Sc_eSetServer)//���÷�����ʱ
    pStr = StrFind(pStr, _CONNECT);
  else if(ESP8266Sc.eState == ESP8266Sc_eIsConn){//���ӳɹ�ʱ
    pStr = StrFind(pStr, _DotDp);
    if(pData != NULL){//���ݸ�ʽ��ȷ��
      if(StrFind(pStr, _NullIp) == NULL){//��IP��ʾû����WIFI,�����ʾ����WIFI��
        ESP8266Sc.Flag |= ESP8266SC_WIFI_FINAL;
        #ifdef SUPPORT_ESP8266SC_LOCAL_IP//���汾��IPʱ
          StrToIp4(pStr, ESP8266Sc.LocalIp);
        #endif 
      }
    }
  }
  // else if(ESP8266Sc.eState == ESP8266Sc_eStartPass){//��ʼ͸��ʱ
  //  pStr = StrFind(pStr, _PassEnter);//��ʼ͸��ʱ
  //}
  
  //��������
  if(pStr != NULL) _ToNextState(0);
  else ESP8266Sc.Timer = 0;//��ǰ����
  return 1;
}
      
      
      
