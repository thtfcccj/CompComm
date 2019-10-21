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
  //注：只能通过接口读写信息(直接存储的可能是经过处理的数据)
  //get响应处理
  const char *pNextPos = StrFind(pContent, "get");
  if(pNextPos != NULL){//找到了
    if(StrFind(pNextPos,_Name) != NULL) 
      MqttConUser_GetInfo(pConUser, 0, pContent);
    else if(StrFind(pNextPos,_Pass) != NULL) 
     MqttConUser_GetInfo(pConUser, 1, pContent);
    else if(StrFind(pNextPos,_Info) != NULL) 
      MqttConUser_GetInfo(pConUser, 2, pContent);
    else if(StrFind(pNextPos,_CfgRd) != NULL){ //相关配置,十六进制表示,对齐相关
      *pContent = '0';
      *(pContent+ 1) = 'x';
      unsigned char Cfg[2];
      Cfg[0] = MqttConUser_GetCfg(pConUser) >>8;
      Cfg[1] = MqttConUser_GetCfg(pConUser) & 0xff;     
      *pDataToAsc(pContent + 2, Cfg, 2) = '\0';
      return pContent;
    }
    else return NULL;//没找到
    if(*pContent == '\0') return Ch_NullString; //未定义
    return pContent;
  }
  //set响应处理
  pNextPos = StrFind(pContent, "set");
  if(pNextPos != NULL){//找到了
    const char *pDataPos;
    if((pDataPos = StrFind(pNextPos,_Name)) != NULL)
      MqttConUser_SetInfo(pConUser, 0, (char*)pGetStrSpaceEnd(pDataPos));
    else if((pDataPos = StrFind(pNextPos,_Pass)) != NULL)
      MqttConUser_SetInfo(pConUser, 1, (char*)pGetStrSpaceEnd(pDataPos));
    else if((pDataPos = StrFind(pNextPos,_Info)) != NULL)
      MqttConUser_SetInfo(pConUser, 2, (char*)pGetStrSpaceEnd(pDataPos));
    else if((pDataPos = StrFind(pNextPos,_CfgWr)) != NULL){//相关配置,十六进制表示
      unsigned char Cfg[2];
      AscToData(Cfg, (unsigned char*)pGetStrSpaceEnd(pDataPos), 4);
      MqttConUser_SetCfg(pConUser, (unsigned short)Cfg[0] << 8 | Cfg[1]);
    }
    else return NULL;//没找到
    return Ch_OK;//操作成功
  }
  return NULL;
}


