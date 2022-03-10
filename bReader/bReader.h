/*******************************************************************************

//		                            位码读取器
此模块主要用于将字节流转换为位码流, 主要用于嵌入式对资源要求很低的场合
此模块修改出自"lodepng->LodePNGBitReader", 感谢作者！！！！！
此模块为重新整理以学习，分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统，做了部分优化
*******************************************************************************/
#ifndef _BREADER_H
#define _BREADER_H

/*******************************************************************************
                             相关配置
********************************************************************************/

//原size_t,字节流 < =8192(65536/8)时，可考虑用unsigned short或
#ifndef brsize_t  
  #define  brsize_t unsigned short   //or unsigned long
  #define  brlen_t 2  //非short时不用定义
#endif

/*******************************************************************************
                             相关结构
*******************************************************************************/

typedef struct _bReader{//成员保留原命名方式
  const unsigned char *data; //原始数据
  brsize_t size;              //data字节大小
  brsize_t bitsize;          //码流大小，为字节*8
  brsize_t bp;               //已解码码流大小
  unsigned long buffer;      //正在被缓冲的位码流
}bReader_t;

/*******************************************************************************
                             相关函数
*******************************************************************************/

//-----------------------判断数据流是否超限-----------------------------------
//void bReader_SizeIsInvalid (unsigned long size);//原型
#ifdef  brlen_t
  #define bReader_SizeIsInvalid(sz)  ((sz) > 0x1fff)
#else 
  #define bReader_SizeIsInvalid(sz)  ((sz) > 0x1fffffff)
#endif

//--------------------------------初始化函数---------------------------------
//不能确定数据长度时，需提前调用()bReader_SizeIsInvalid()确保本不超限
void bReader_Init(bReader_t *reader,
                  const unsigned char* data,
                  brsize_t size);
             
//-------------------------------缓冲至少9b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=9b，LSB低位在前
void bReader_2BufferB9(bReader_t* reader);
//建议使用此函数以说明实际需要的字节个数：
#define bReader_BufferB9(rd, needbits) do{bReader_2BufferB9(rd); }while(0)
#define ensureBits9(r,n)  bReader_BufferB9(r, n) //兼容性考虑

//-------------------------------缓冲至少17b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=17b，LSB低位在前
void bReader_2BufferB17(bReader_t* reader);
//建议使用此函数以说明实际需要的字节个数：
#define bReader_BufferB17(rd, needbits) do{bReader_2BufferB17(rd); }while(0)
#define ensureBits17(r,n)  bReader_BufferB17(r, n) //兼容性考虑

//-------------------------------缓冲至少25b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=25b，LSB低位在前
void bReader_2BufferB25(bReader_t* reader);
//建议使用此函数以说明实际需要的字节个数：
#define bReader_BufferB25(rd, needbits) do{bReader_2BufferB25(rd); }while(0)
#define ensureBits25(r,n)  bReader_BufferB25(r, n) //兼容性考虑

//-------------------------------缓冲至少32b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=32b，LSB低位在前
void bReader_2BufferB32(bReader_t* reader);
//建议使用此函数以说明实际需要的字节个数：
#define bReader_BufferB32(rd, needbits) do{bReader_2BufferB32(rd); }while(0)
#define ensureBits32(r,n)  bReader_BufferB32(r, n) //兼容性考虑

//-------------------------------窥视需要的位数据-----------------------
//原peekBits,如：需要10位，则最大返回 (1 >> 10) - 1 = 1023
unsigned long bReader_PeekB(bReader_t* reader, 
                             unsigned char needbits);//需要的位数
#define peekBits(r,n)  bReader_RdB(r,n)  //兼容性考虑

//------------------------------推进位数据-------------------------------
//原AdvanceBits,将数据流向前推进
void bReader_AdvanceB(bReader_t* reader, 
                      unsigned char needbits);  //需要推进的位数 
#define advanceBits(r,n)  bReader_RdB(r,n)  //兼容性考虑

//------------------------------读取位数据-------------------------------
//原readBits,返回数据，同时将数据向前推进
unsigned long bReader_RdB(bReader_t* reader, 
                           unsigned char needbits);  //需要读取与推进的位数 
#define readBits(r,n)  bReader_RdB(r,n)  //兼容性考虑

#endif //_BREADER_H


