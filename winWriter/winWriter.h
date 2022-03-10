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
  unsigned char *data;       //缓冲的数据流,应留足够空间,具体见使用者要求
  brsize_t capability;       //分配的data数据容量 
  volatile brsize_t start;  //往前为已使用的数据  
  //信息与交换部分：
  brsize_t MaxOutSize;       //允许译码出的数据大小
  brsize_t OutedSize;        //外部使用，已经流出的数据大小
  brsize_t (*LaterPro)(struct  _winWriter*); //后继数据处理  
  //利用此结构，用于交换的用户数据(由合用者声明与定义)：
  unsigned char Cfg;        //用户需要配置位
  unsigned char U8Para;     //用户缓冲可能需要的参数
  unsigned short U16Para;   //用户缓冲可能需要的参数
  unsigned long U32Para;    //用户缓冲可能需要的参数

}winWriter_t;


/*******************************************************************************
                             相关函数
*******************************************************************************/

//-------------------------------将结构数据清零------------------------------
//清零后，除start，OutedSize外，其它需用户手动置数
//void winWriter_Clr(winWriter *out);
#define winWriter_Clr(o) do{memset(o, 0, sizeof(winWriter_t)); }while(0)

//-------------------------输出数据用完处理函数---------------------------------
//缓冲区即将满时，调用此函数送出数据，返回0正常输出,否则异常
signed char winWriter_OutData(winWriter_t *out);

//-------------------------------copy数据--------------------------------
//此copy考虑到copy到start后数据满情况，若如此，则要送出部分数据
signed char winWriter_Copy(winWriter_t *out,
                           const unsigned char *pData,//数据
                           brsize_t DataLen,    //数据长度
                           brsize_t LeavedSize); //可留下的数据空间

//--------------------------从当前位置往前copy数据------------------------------
//此copy用于从输出数据里,copy数据至当前start位置,返回0正常输出,否则异常
//此copy考虑到copy到start后数据满情况，若如此，则要送出部分数据
signed char winWriter_CopyBackward(winWriter_t *out,
                                    brsize_t backward,//往前位置(由当前start开始)
                                    brsize_t distance,//copy距离
                                    brsize_t LeavedSize);//可留下的数据空间


#endif //_WIN_WRITER_H


