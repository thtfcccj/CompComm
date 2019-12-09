/***********************************************************************

                  Mqtt�û����ݱ�����
��ģ�齫�û������滻Ϊ��ǰ���ݲ�����
��ģ����Ҫ��ESP8266Scģ����й���
***********************************************************************/

#include "ESP8266ScAT.h"
#include "StringEx.h"
#include "StrDefParser.h"

#include "ESP8266Sc.h"
#include "Eeprom.h"
#include "InfoBase.h"

#include <string.h>

/******************************************************************************
		                        ��غ���
******************************************************************************/
//------------------------------����ַ�----------------------------------
static const char _ServerIp[] = {"Ip"};
static const char _ServerPort[] = {"Port"};

 #ifdef SUPPORT_ESP8266SC_LOCAL_IP//���汾��IPʱ
  static const char _LocalIp[] = {"lIp"};
#endif
  
//-----------------------------����������-------------------------------
//���ؽ���ַ�,������ʱΪNULL
const char *ESP8266ScAT_pParser(char *pContent)//Ҫ��������,ascii��
{
  //ע��ֻ��ͨ���ӿڶ�д��Ϣ(ֱ�Ӵ洢�Ŀ����Ǿ������������)
  //get��Ӧ����
  const char *pNextPos = StrFind(pContent, En_get);
  if(pNextPos != NULL){//�ҵ���
    if(StrFind(pNextPos,_ServerIp) != NULL) 
      *Ip4ToStr(ESP8266Sc.Info.ServerIp, pContent) = '\0';
    else if(StrFind(pNextPos,_ServerPort) != NULL) 
      Value2StringMin(ESP8266Sc.Info.ServerPort, pContent, 1);
    #ifdef SUPPORT_ESP8266SC_LOCAL_IP//���汾��IPʱ
    else if(StrFind(pNextPos,_LocalIp) != NULL) 
      *Ip4ToStr(ESP8266Sc.LocalIp, pContent) = '\0';
    #endif
    return pContent;
  }
  //set��Ӧ����
  pNextPos = StrFind(pContent, En_set);
  if(pNextPos != NULL){//�ҵ���
    const char *pDataPos;
    if((pDataPos = StrFind(pNextPos,_ServerIp)) != NULL){
      StrToIp4(pGetStrSpaceEnd(pDataPos), ESP8266Sc.Info.ServerIp);
    }
    else if((pDataPos = StrFind(pNextPos,_ServerPort)) != NULL){
       ESP8266Sc.Info.ServerPort = DecStr2Us(pGetStrSpaceEnd(pDataPos));
    }
    else return NULL;//û�ҵ�
    //������
    Eeprom_Wr(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
    return Ch_OK;//�����ɹ�
  }
  return NULL;
}


