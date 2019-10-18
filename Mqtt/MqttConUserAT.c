/***********************************************************************

                  Mqtt�û����ݱ�����
��ģ�齫�û������滻Ϊ��ǰ���ݲ�����
��ģ����Ҫ��MqttConUserģ����й���
***********************************************************************/

#include "MqttConUserAT.h"
#include "StringEx.h"
#include "StrDefParser.h"
#include "NetData.h"
#include <string.h>

/******************************************************************************
		                        ��غ���
******************************************************************************/
//------------------------------����ַ�----------------------------------
static const char _Name[] = {"Name"};
static const char _Pass[] = {"Pass"};
static const char _Info[] = {"Info"};
static const char _CfgRd[] = {"Cfg"};
static const char _CfgWr[] = {"Cfg 0x"};

//-----------------------------����������-------------------------------
//���ؽ���ַ�,������ʱΪNULL
const char *MqttConUserAT_pParser(struct _MqttConUser *pConUser,
                                   char *pContent)//Ҫ��������,ascii��
{
  //get��Ӧ����
  const char *pNextPos = StrFind(pContent, "get");
  if(pNextPos != NULL){//�ҵ���
    if(pNextPos = StrFind(pContent,_Name)) 
      return pConUser->Info.UserName;
    else if(pNextPos = StrFind(pContent,_Pass)) 
      return pConUser->Info.UserPass;
    else if(pNextPos = StrFind(pContent,_Info)) 
      return pConUser->Info.Info;
    else if(pNextPos = StrFind(pContent,_CfgRd)){ //�������,ʮ�����Ʊ�ʾ,�������
      *pContent++ = '0';
      *pContent++ = 'x';
      *pDataToAsc(pContent, (unsigned char*)&pConUser->Info.Cfg, 2) = '\0';
      return pContent;
    }
    else return Ch_NoFunCode;//û�ҵ�
  }
  //set��Ӧ����
  pNextPos = StrFind(pContent, "set");
  if(pNextPos != NULL){//�ҵ���
    if(pNextPos = StrFind(pContent,_Name)) 
      strcpyEx(pConUser->Info.UserName,pContent, MQTT_CON_USER_NAME_LEN - 1);
    else if(pNextPos = StrFind(pContent,_Pass)) 
      strcpyEx(pConUser->Info.UserPass, pContent, MQTT_CON_USER_PASS_LEN - 1);
    else if(pNextPos = StrFind(pContent,_Info)) 
      strcpyEx(pConUser->Info.Info, pContent, MQTT_CON_USER_INFO_LEN - 1);
    else if(pNextPos = StrFind(pContent,_CfgWr)){ //�������,ʮ�����Ʊ�ʾ,�������
      AscToData((unsigned char*)&pConUser->Info.Cfg, (unsigned char*)pContent, 4);
    }
    else return Ch_NoFunCode;//û�ҵ�
  }
  return NULL;
}


