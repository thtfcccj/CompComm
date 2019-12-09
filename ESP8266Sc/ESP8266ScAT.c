/***********************************************************************

                  Mqtt用户内容编码器
此模块将用户内容替换为当前内容并返回
此模块主要对ESP8266Sc模块进行管理
***********************************************************************/

#include "ESP8266ScAT.h"
#include "StringEx.h"
#include "StrDefParser.h"

#include "ESP8266Sc.h"
#include "Eeprom.h"
#include "InfoBase.h"

#include <string.h>

/******************************************************************************
		                        相关函数
******************************************************************************/
//------------------------------相关字符----------------------------------
static const char _ServerIp[] = {"Ip"};
static const char _ServerPort[] = {"Port"};

 #ifdef SUPPORT_ESP8266SC_LOCAL_IP//保存本地IP时
  static const char _LocalIp[] = {"lIp"};
#endif
  
//-----------------------------主解析函数-------------------------------
//返回结果字符,不返回时为NULL
const char *ESP8266ScAT_pParser(char *pContent)//要解析内容,ascii码
{
  //注：只能通过接口读写信息(直接存储的可能是经过处理的数据)
  //get响应处理
  const char *pNextPos = StrFind(pContent, En_get);
  if(pNextPos != NULL){//找到了
    if(StrFind(pNextPos,_ServerIp) != NULL) 
      *Ip4ToStr(ESP8266Sc.Info.ServerIp, pContent) = '\0';
    else if(StrFind(pNextPos,_ServerPort) != NULL) 
      Value2StringMin(ESP8266Sc.Info.ServerPort, pContent, 1);
    #ifdef SUPPORT_ESP8266SC_LOCAL_IP//保存本地IP时
    else if(StrFind(pNextPos,_LocalIp) != NULL) 
      *Ip4ToStr(ESP8266Sc.LocalIp, pContent) = '\0';
    #endif
    return pContent;
  }
  //set响应处理
  pNextPos = StrFind(pContent, En_set);
  if(pNextPos != NULL){//找到了
    const char *pDataPos;
    if((pDataPos = StrFind(pNextPos,_ServerIp)) != NULL){
      StrToIp4(pGetStrSpaceEnd(pDataPos), ESP8266Sc.Info.ServerIp);
    }
    else if((pDataPos = StrFind(pNextPos,_ServerPort)) != NULL){
       ESP8266Sc.Info.ServerPort = DecStr2Us(pGetStrSpaceEnd(pDataPos));
    }
    else return NULL;//没找到
    //保存结果
    Eeprom_Wr(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
    return Ch_OK;//操作成功
  }
  return NULL;
}


