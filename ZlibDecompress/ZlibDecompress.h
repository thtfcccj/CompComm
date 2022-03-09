/*******************************************************************************

//		                 ZLIB解码
此模块修改出自"lodepnge", 感谢作者！！！！！
此模块为重新整理以学习分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统做了优化,不需动态分配内存
*******************************************************************************/
#ifndef _ZLIB_DECOMPRESS_H
#define _ZLIB_DECOMPRESS_H

#include "bReader.h"
#include "winWriter.h"
#include "DeflateNano.h"


/*******************************************************************************
                             相关函数
*******************************************************************************/


//--------------------------------ZLib解压缩------------------------------------
//原lodepng_zlib_decompressv
signed char ZlibDecompress(const unsigned char *in, //压缩数据包
                           brsize_t insize,           //idat区数据个数
                           winWriter_t *out);     //已准备好的接收数据缓冲


#endif //_PNG_IDAT_DECODER_H


