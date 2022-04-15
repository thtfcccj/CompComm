/*******************************************************************************

//		                 ZLIB解码
此模块修改出自"lodepnge", 感谢作者！！！！！
此模块为重新整理以学习分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统做了优化,不需动态分配内存
此模块支持多线程调用！
*******************************************************************************/
#ifndef _ZLIB_DECOMPRESS_H
#define _ZLIB_DECOMPRESS_H

#include "bReader.h"
#include "winWriter.h"
#include "DeflateNano.h"

//--------------------------------------主结构------------------------------
#define _ZlibDecompress   _DeflateNano //结构重命名,直接继承


//--------------------------接收数据缓冲out使用声明---------------------------
//1： out->Cfg, 配置位定义为:
#define ZLIB_DECOMPRESS_EN_CHECK       0x40 //允许数据校验

//2： out->U32Para 用作了Adler32中的校验结果缓存

//--------------------------------ZLib解压缩------------------------------------
//原lodepng_zlib_decompressv
signed char ZlibDecompress(struct _ZlibDecompress *pZ,//无需初始化
                           const unsigned char *in, //压缩数据包
                           brsize_t insize,           //idat区数据个数
                           winWriter_t *out);     //接收数据缓冲，见声明


//------------------------回调函数:得到校验结果-------------------------------
unsigned long ZlibDecompress_cbGetAdler32(winWriter_t *out); //接收数据缓冲

#endif //_PNG_IDAT_DECODER_H


