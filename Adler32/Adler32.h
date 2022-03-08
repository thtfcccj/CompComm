/***********************************************************************

//		               Adler32校验模块
此模块主要用于Deflate压缩算法中校验数据的完整性用
此模块修改出自"lodepng->Adler32", 感谢作者！！！！！
此模块为重新整理以学习，分析代码以为加深印像，并符合现编码规则，以方便使用
***********************************************************************/
#ifndef _ADLER32_H
#define _ADLER32_H

//----------------------得到Adler32校验码----------------------------
unsigned long Adler32_Get(unsigned long Adler,
                           const unsigned char *pData, 
                           unsigned long Len);

//----------------------得到Adler=1的32校验码----------------------------
unsigned long Adler32_Get1(const unsigned char *pData, 
                            unsigned long Len);

#endif


