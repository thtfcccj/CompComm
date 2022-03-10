/*******************************************************************************

//		                   Deflate 精简型 解码器
此模块主要用于Deflate压缩格式，如GZIP PNG图片的解码
此模块修改出自"lodepng", 感谢作者！！！！！
此模块为重新整理以学习，分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统，做了部分优化
此模块支持多线程调用！
*******************************************************************************/
#ifndef _DEFLATE_NANO_H
#define _DEFLATE_NANO_H

#include "bReader.h"
#include "WinWriter.h"
#include "HuffmanTree.h"


/*******************************************************************************
                             相关配置
********************************************************************************/


//must be at least 258 for max length, and a few extra for adding a few extra literals */
//out->data数据区应有足够的空间，需大于下述累架值,建议>=其1.5倍
#define DEFLATE_NANO_OUT_RESERVED_SIZE  260 //定义保留字节,out->data数据足够时最少260
#define DEFLATE_NANO_OUT_LEAVED_SIZE    260 //定义留空字节,以备后用,建立260


/*******************************************************************************
                             相关结构
*******************************************************************************/

//-----------------------------主结构----------------------------------
struct _DeflateNano{
  //HuffmanTree在整个解码过程中都不会释放，故直接组合在一起
  struct _HuffmanTreeMng HfMng; //Base继承
  struct _HuffmanTreeBuf HfBuf;
  //本模块缓冲
  bReader_t Reader;
};

/*******************************************************************************
                             相关函数
*******************************************************************************/


//--------------------------接收数据缓冲out使用声明---------------------------
//此模块合用了out->Cfg, 配置位定义为:
#define DEFLATE_NANO_IGNORE_NLEN    0x80 //忽略长度校验，原ignore_nlen

//--------------------------------译码函数-------------------------------------
//原inflateHuffmanBlock
signed char DeflateNano_Decoder(struct _DeflateNano *pDeflate,//无需初始化
                                 const unsigned char *data,//输入数据流
                                 brsize_t insize,           //数据个数
                                 winWriter_t *out);       //接收数据缓冲，见声明

#endif //_DEFLATE_NANO_H


