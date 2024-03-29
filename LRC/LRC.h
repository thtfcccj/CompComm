/***********************************************************************

//		               LRC校验程序集合

***********************************************************************/
#ifndef _LRC_H
#define _LRC_H



//----------------------得到十六位LRC校验码----------------------------
//输出没有取反
unsigned short LRC16_Get(const unsigned char *pData, 
                         unsigned short Len);

//----------------------得到8位LRC校验码----------------------------
//输出没有取反
unsigned char LRC8_Get(const unsigned char *pData, 
                       unsigned short Len);

#endif


