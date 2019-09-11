/***********************************************************************

                  字符编解码模块实现

***********************************************************************/

#include "Code.h"
#include "GB2312_UCS2.h"
#include <string.h>


/****************************************************************************
		                      相关结构及函数-SUPPORT_CODE_TASK相关
****************************************************************************/
#ifdef SUPPORT_CODE_TASK
struct _Code Code;

//-----------------------------初始化函数------------------------------------
//开机时调用
void Code_Init(void)
{
  memset(&Code, 0, sizeof(struct _Code));
}

//----------利用空闲时间处理Ucs2码流到混合GB2312码流启动函数---------------------
//MSB方式(高位在前)处理, 返回是否接收成功!
signed char Code_Ucs2ToGB2312Start(const unsigned char *pUcs2,  //MSB方式
                                   unsigned char *pGB2312,       //MSB方式
                                   unsigned short Len,          //Unicode码字节流长度
                                   void(*FinalNotify)(unsigned short)) //处理完成通报函数
{
  if(Code.pUcs2 != NULL) return -1; //忙中
  //记住现场
  Code.pUcs2 = pUcs2;
  Code.pGB2312 = pGB2312; 
  Code.FinalNotify = FinalNotify;
  Code.UnicodeLen = Len; 
  Code.UnicodePos = 0;   
  Code.GB2312Pos = 0; 
  return 0;
}

//-----------------------------空闲任务函数------------------------------------
//放入系统空闲进程中扫描
void Code_FastTask(void)
{
  unsigned char CurLen;
  do{
    CurLen = Code_Ucs2ToGB2312(Code.pUcs2 + Code.UnicodePos,
                               Code.pGB2312 + Code.GB2312Pos,
                               2);
    Code.GB2312Pos += CurLen;
    Code.UnicodePos += 2;
    if(Code.UnicodePos >= Code.UnicodeLen){//找完了
      Code.FinalNotify(Code.GB2312Pos);
      break;
    }
  }while(CurLen <= 1); //ASC码时重复执行
}

#endif //#ifdef SUPPORT_CODE_TASK



/****************************************************************************
		                      相关结构-独立调用
****************************************************************************/

//-------------------混合GB2312码流转换成Ucs2码流函数------------------------
//返回转换后的字符长度
unsigned short Code_GB2312ToUcs(const char *pGB2312, //MSB方式
                                unsigned char *pUcs2,         //MSB方式
                                unsigned short Len)           //字符长度
{
  const char *pGB2312End = pGB2312 + Len;
  const unsigned char *pUcs2Start = pUcs2;  
  for( ;pGB2312 < pGB2312End; pGB2312++, pUcs2++){
    unsigned char H = *pGB2312;
    if(H < 0x80){ //ASCII直接转换
      *pUcs2++= 0x00;
      *pUcs2 = H;
      continue;
    }
    if((H >= 0xA1) || (H <= 0xF7)){//GB2312两个组成1个
      pGB2312++;
      unsigned char L = *pGB2312;
      if((L >= 0xA1) || (L <= 0xF7)){//低位判断，GB2312强制转换
        unsigned short Unicode = GB2312ToUcs2(((unsigned short)H << 8) + L);
        *pUcs2++= Unicode >> 8;
        *pUcs2 = Unicode & 0xFF;
        continue;
      }
    }
    //异常填充0
    *pUcs2++= 0x00;
    *pUcs2 = 0x00;
  }
  return pUcs2 - pUcs2Start;
}

//--------------------------Ucs2码流到混合GB2312码流函数---------------------
//MSB方式(高位在前)处理, 返回转换后的字符长度
//注意此函数需要的处理时间较长！！
unsigned short Code_Ucs2ToGB2312(const unsigned char *pUcs2,  //MSB方式
                                 char *pGB2312,       //MSB方式
                                 unsigned short Len)        //Unicode码字节流长度     
{
  const unsigned char *pUcs2End = pUcs2 + Len;
  const char *pGB2312Start = pGB2312;    
  for( ;pUcs2 < pUcs2End; pUcs2 += 2, pGB2312++){  
    unsigned short Unicode = ((unsigned short)(*pUcs2) << 8) + *(pUcs2 + 1);
    if(Unicode < 0x0080){//ASCII直接转换
      *pGB2312 = Unicode;
    }
    else{ //Unicode码转换为GB2312
      Unicode = Ucs2ToGB2312(Unicode);
      *pGB2312++= Unicode >> 8;
      *pGB2312 = Unicode & 0xFF;
    }
  }
  return pGB2312 - pGB2312Start;
}






 