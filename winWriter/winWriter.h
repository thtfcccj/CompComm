/*******************************************************************************

//		                   滑动窗口写数据结构
此模块主要用于输出数据空间不够时，
     使用一边生成数据(如解压)，一边输出数据(如写FLASH,显示屏)的方式工作
*******************************************************************************/
#ifndef _WIN_WRITER_H
#define _WIN_WRITER_H

#include "bReader.h" //brsize_t

/*******************************************************************************
                             相关结构
*******************************************************************************/

//输出数据格式
typedef struct _winWriter{
  //输出的数据缓冲,原ucvector部分       
  unsigned char *data;    //缓冲的数据流,应留足够空间,具体见使用者要求
  brsize_t capability;     //分配的data数据容量 
  volatile brsize_t start; //往前为已使用的数据  
  //信息与交换部分：
  brsize_t MaxOutSize;       //允许译码出的数据大小
  brsize_t OutedSize;        //外部使用，已经流出的数据大小  
  unsigned char Cfg;        //配置位，见定义
  unsigned char U8Para;    //用户缓冲可能需要的参数
  unsigned short U16Para;   //用户缓冲可能需要的参数
  
  unsigned long Checksum;    //已经流出的数据的校验和
  //当size >= capability时，进行数据处理,以处理掉(如送显，缓存等)已缓冲的数据流
  //返回用掉了多少数据
  brsize_t (*LaterPro)(struct  _winWriter*); 
}winWriter_t;

//配置位定义为
#define WIN_WRITER_IGNORE_NLEN    0x80 //忽略长度各校验，原ignore_nlen
#define WIN_WRITER_EN_CHECK       0x40 //允许数据校验
#define WIN_WRITER_USER_MASK      0x0F //低4bit留给用户作其它功能使用

/*******************************************************************************
                             相关函数
*******************************************************************************/

//-------------------------------将结构数据清零------------------------------
//清零后，除start，OutedSize外，其它需用户手动置数
//void winWriter_Clr(winWriter *out);
#define winWriter_Clr(o) do{memset(o, 0, sizeof(winWriter_t)); }while(0)

#endif //_WIN_WRITER_H


