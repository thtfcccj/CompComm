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
  if(!Inited){
    memcpy(&ESP8266Sc.Info, 
          ESP8266Sc_pcbGetInfoDefault(), sizeof(struct _ESP8266ScInfo));
    Eeprom_Wr(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
  }
  else{
    Eeprom_Rd(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
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
//0x20:Ҫ������, 0x10����OK���
static const unsigned char _StateInfo[] = {
  0x00 | 0x00 | 0x00 | 0x00 | 0x00,   //����״̬
  0x40 | 0x00 | 0x00 | 0x00 | 0x00,   //�˳�͸��ģʽ
  0x80 | 0x00 | 0x00 | 0x00 | 0x00,   //��λ
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //�رջ���,����OKȷ��ģ�����
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //��Statinģʽ,����OK�ɹ�
  0x40 | 0x20 | 0x00 | 0x00 | 0x00,   //��ȡ����IP��ַ��ȷ���������������
  0xC0 | 0x20 | 0x10 | 0x00 | 0x00,   //ת����������ģʽ,�ȴ�OK����
  0x80 | 0x20 | 0x00 | 0x00 | 0x00,   //���÷�����,����connect�ɹ�
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //����Ϊ͸��ģʽ,����OK�ɹ�
  0x40 | 0x00 | 0x00 | 0x00 | 0x00,   //��ʼ͸��
  0x40 | 0x00 | 0x00 | 0x00 | 0x00,  //͸����ʼ����׸��̶�����
};

//---------------------------״̬gc ״̬λ-----------------------------------
//��λ����Ϊ��0-3Bit:��ȷʱ��һ״̬�� 4-7Bit ����ʱ��һ״̬
//0x20:Ҫ������, 0x10����OK��飬����Ϊ������, 0x0f�´�״̬
static const unsigned char _State2State[] = {
  ESP8266Sc_eIdie     | ESP8266Sc_eIdie,         //����״̬
  ESP8266Sc_eExitPass | ESP8266Sc_eRst,          //�˳�͸��ģʽ
  ESP8266Sc_eExitPass | ESP8266Sc_eIsRdy,        //��λ
  ESP8266Sc_eExitPass | ESP8266Sc_eSetStation,   //�رջ���,����OKȷ��ģ�����
  ESP8266Sc_eRst      | ESP8266Sc_eIsConn,       //��Statinģʽ,����OK�ɹ�
  ESP8266Sc_eIsConn   | ESP8266Sc_eSetServer,    //��ȡ����IP��ַ��ȷ���������������
  ESP8266Sc_eRst      | ESP8266Sc_eSetServer,    //ת����������ģʽ,�ȴ�OK����
  ESP8266Sc_eSetServer| ESP8266Sc_eSetPass,      //���÷�����,����connect�ɹ�
  ESP8266Sc_eSetPass  | ESP8266Sc_eStartPass,    //����Ϊ͸��ģʽ,����OK�ɹ�
  ESP8266Sc_eSetPass  | ESP8266Sc_ePassData1st,  //��ʼ͸��
  //ESP8266Sc_eIdie     | ESP8266Sc_eIdie,         //͸����ʼ����׸��̶�����  
};

//--------------------------�ȴ���Ϣת��Ϊʱ��----------------------------------
static const unsigned char _WaitInfo2Time[] = {
  0,//0:���ȣ�
  3,  //1�ȴ�ͨѶ��Ӧ(������ATE0����ֱ�ӻ����Ӧ)
  15, //�ȴ�������Ӧ(�縴λ)
  255,// 3�ȴ��û���Ӧ(����������)
};

//-----------------------״̬����Ӧ��ָ���ַ�������-----------------------------
static const char _DotSpace[] =   {"\",\""};      //����ַ�
static const char _DefaultServer[] = {"192.168.88.101\",\"10001\"\r\n"}; //Ĭ�Ϸ�����

//״̬����Ӧ,�����ַ�
static const char _ExitPass[] =  {"+++"};         //�˳�͸��ģʽ
static const char _Rst[] =       {"AT+RST\r\n"};      //��λ
static const char _DisReturn[] = {"ATE0\r\n"};        //��ֹ����(IsRdy���)
static const char _SetStation[] ={"AT+CWMODE=1\r\n"}; //��Statinģʽ
static const char _GetIp[] =     {"AT+CIFSR\r\n"};    //�õ�����IP(IsConn���)
static const char _SmartConn[]=  {"AT+CWSMARTSTART=2\r\n"};//����������ʽ
static const char _SetServer[] =  {"AT+CIPSTART=\"TCP\",\""}; //���÷�����,�����Ϣ
static const char _SetPass[] =   {"AT++CIPMODE=1"};//����Ϊ͸��ģʽ
static const char _EnterPass[] = {"\r\n>"};        //��ʼ͸��

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

//-------------------------ת����һ״̬����----------------------------------
//��ȷʱ����
static void _ToNextState(unsigned char IsErr)
{
  unsigned char NextState = _State2State[ESP8266Sc.eState];
  if(IsErr) //�д���ʱ������״̬
    ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState >> 4);
  else //��ȷʱ����һ״̬
    ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState & 0x0f);
  ESP8266Sc.RetryIndex = 0;//��һ״̬��λ
  ESP8266Sc.Flag |= ESP8266SC_SEND_RDY;//��һ״̬����    
}

//---------------------------������-------------------------------------
//128ms����һ��
void ESP8266Sc_Task(void)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return; //������
  if(ESP8266Sc.Flag & ESP8266SC_SEND_RDY) return; //����δ���
  if(ESP8266Sc.Timer){//ʱ��δ��
    ESP8266Sc.Timer--;
    return;
  }
  
  //�ȴ�ʱ�䵽�ˣ���鳬ʱ���
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if(Info & 0x20){//�������ʱ��ʱ��
    ESP8266Sc.RetryIndex++;
    //��������ת����״̬
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
    *pSendBuf = '\0';//�û�����������ַ�������
    ESP8266Sc_cbFulPassData1st(pSendBuf);   //���û����
    unsigned short Len = strlen(pSendBuf);
    if(Len) ESP8266Sc_pcbSendBuf(Len); //����
  }
  
  //����ESP8266ָ��
  pSendBuf = strcpyL(pSendBuf, _pCmd[ESP8266Sc.eState - 1]);//β����
  //Ҫ���ӷ�������Ϣ
  if(ESP8266Sc.eState == ESP8266Sc_eSetServer){
    if(ESP8266Sc_cbIsDefaultServer()){//Ĭ�Ϸ�����
      strcpyL(pSendBuf, _DefaultServer);
    }
    else{//���õķ�����
      pSendBuf = Ip4ToStr(ESP8266Sc.Info.ServerIp, (char*)pSendBuf);//IP
      pSendBuf = strcpyL((char*)pSendBuf, _DotSpace); //","  
      pSendBuf = Value2StringMin(ESP8266Sc.Info.ServerPort, pSendBuf, 1);//�˿ں�
      strcpyL(pSendBuf, _DefaultServer + (sizeof(_DefaultServer) - 4)); //�����ַ�
    }
  }
  ESP8266Sc_pcbSendBuf(strlen(pESP8266Sc_pcbGetBuf())); //�����ͳ�
  ESP8266Sc.Timer = _WaitInfo2Time[_StateInfo[ESP8266Sc.eState] >> 6];//��ʱ
}

//----------------------------��������ַ�------------------------------
static const char _OK[] =         {"OK"};          //OK�ַ�
static const char _CONNECTED[] =  {"CONNECTED"};//���������ӳɹ���־
//����IPǰ׺(����������+CIFSR:STAIP,"192.168.88.152"+CIFSRSTAMAC,)
static const char _DotDp[] =       {",\""}; 

//--------------------------�������ݴ���----------------------------------
//����0û����,���������
signed char ESP8266Sc_RcvData(const unsigned char *pData,
                               unsigned char Len)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return 0; //������
  //ע��δ��鳤�ȣ�������
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if(!(Info & 0x80)) return 1; //���������
  const char *pStr = (const char *)pData;
  //�������Ƿ���ȷ(pStr ��= NULLΪ��ȷ)
  if(Info & 0x40)//OK�ַ����
    pStr = StrFind(pStr, _OK); 
  else if(ESP8266Sc.eState == ESP8266Sc_eSetServer)//���÷�����ʱ
    pStr = StrFind(pStr, _CONNECTED); 
  else if(ESP8266Sc.eState == ESP8266Sc_eIsConn){//���ӳɹ�ʱ
    pStr = StrFind(pStr, _DotDp);
    if(pData != NULL){//�����ϱ�����
      ESP8266Sc.eState |= ESP8266SC_WIFI_FINAL;
      #ifdef SUPPORT_ESP8266SC_LOCAL_IP//���汾��IPʱ
        StrToIp4(pStr, ESP8266Sc.LocalIp);
      #endif 
    }
  }
  //��������
  if(pStr != NULL) _ToNextState(0);  
  else ESP8266Sc.Timer = 0;//��ǰ����
  return 1;
}
      
      
      
