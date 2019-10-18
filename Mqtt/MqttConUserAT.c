/***********************************************************************

                  Mqtt用户内容编码器
此模块将用户内容替换为当前内容并返回
此模块主要对MqttConUser模块进行管理
***********************************************************************/

#include "MqttConUserAT.h"
#include "StringEx.h"
#include "StrDefParser.h"
#include "NetData.h"
#include <string.h>

/******************************************************************************
		                        相关函数
******************************************************************************/
//------------------------------相关字符----------------------------------
static const char _Name[] = {"Name"};
static const char _Pass[] = {"Pass"};
static const char _Info[] = {"Info"};
static const char _CfgRd[] = {"Cfg"};
static const char _CfgWr[] = {"Cfg 0x"};

//-----------------------------主解析函数-------------------------------
//返回结果字符,不返回时为NULL
const char *MqttConUserAT_pParser(struct _MqttConUser *pConUser,
                                   char *pContent)//要解析内容,ascii码
{
  //get响应处理
  const char *pNextPos = StrFind(pContent, "get");
  if(pNextPos != NULL){//找到了
    if(pNextPos = StrFind(pContent,_Name)) 
      return pConUser->Info.UserName;
    else if(pNextPos = StrFind(pContent,_Pass)) 
      return pConUser->Info.UserPass;
    else if(pNextPos = StrFind(pContent,_Info)) 
      return pConUser->Info.Info;
    else if(pNextPos = StrFind(pContent,_CfgRd)){ //相关配置,十六进制表示,对齐相关
      *pContent++ = '0';
      *pContent++ = 'x';
      *pDataToAsc(pContent, (unsigned char*)&pConUser->Info.Cfg, 2) = '\0';
      return pContent;
    }
    else return Ch_NoFunCode;//没找到
  }
  //set响应处理
  pNextPos = StrFind(pContent, "set");
  if(pNextPos != NULL){//找到了
    if(pNextPos = StrFind(pContent,_Name)) 
      strcpyEx(pConUser->Info.UserName,pContent, MQTT_CON_USER_NAME_LEN - 1);
    else if(pNextPos = StrFind(pContent,_Pass)) 
      strcpyEx(pConUser->Info.UserPass, pContent, MQTT_CON_USER_PASS_LEN - 1);
    else if(pNextPos = StrFind(pContent,_Info)) 
      strcpyEx(pConUser->Info.Info, pContent, MQTT_CON_USER_INFO_LEN - 1);
    else if(pNextPos = StrFind(pContent,_CfgWr)){ //相关配置,十六进制表示,对齐相关
      AscToData((unsigned char*)&pConUser->Info.Cfg, (unsigned char*)pContent, 4);
    }
    else return Ch_NoFunCode;//没找到
  }
  return NULL;
}


