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
  //ע��ֻ��ͨ���ӿڶ�д��Ϣ(ֱ�Ӵ洢�Ŀ����Ǿ������������)
  //get��Ӧ����
  const char *pNextPos = StrFind(pContent, "get");
  if(pNextPos != NULL){//�ҵ���
    if(StrFind(pNextPos,_Name) != NULL) 
      MqttConUser_GetInfo(pConUser, 0, pContent);
    else if(StrFind(pNextPos,_Pass) != NULL) 
     MqttConUser_GetInfo(pConUser, 1, pContent);
    else if(StrFind(pNextPos,_Info) != NULL) 
      MqttConUser_GetInfo(pConUser, 2, pContent);
    else if(StrFind(pNextPos,_CfgRd) != NULL){ //�������,ʮ�����Ʊ�ʾ,�������
      *pContent = '0';
      *(pContent+ 1) = 'x';
      unsigned char Cfg[2];
      Cfg[0] = MqttConUser_GetCfg(pConUser) >>8;
      Cfg[1] = MqttConUser_GetCfg(pConUser) & 0xff;     
      *pDataToAsc(pContent + 2, Cfg, 2) = '\0';
      return pContent;
    }
    else return NULL;//û�ҵ�
    if(*pContent == '\0') return Ch_NullString; //δ����
    return pContent;
  }
  //set��Ӧ����
  pNextPos = StrFind(pContent, "set");
  if(pNextPos != NULL){//�ҵ���
    const char *pDataPos;
    if((pDataPos = StrFind(pNextPos,_Name)) != NULL)
      MqttConUser_SetInfo(pConUser, 0, (char*)pGetStrSpaceEnd(pDataPos));
    else if((pDataPos = StrFind(pNextPos,_Pass)) != NULL)
      MqttConUser_SetInfo(pConUser, 1, (char*)pGetStrSpaceEnd(pDataPos));
    else if((pDataPos = StrFind(pNextPos,_Info)) != NULL)
      MqttConUser_SetInfo(pConUser, 2, (char*)pGetStrSpaceEnd(pDataPos));
    else if((pDataPos = StrFind(pNextPos,_CfgWr)) != NULL){//�������,ʮ�����Ʊ�ʾ
      unsigned char Cfg[2];
      AscToData(Cfg, (unsigned char*)pGetStrSpaceEnd(pDataPos), 4);
      MqttConUser_SetCfg(pConUser, (unsigned short)Cfg[0] << 8 | Cfg[1]);
    }
    else return NULL;//û�ҵ�
    return Ch_OK;//�����ɹ�
  }
  return NULL;
}


