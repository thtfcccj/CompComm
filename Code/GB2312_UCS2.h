/***********************************************************************

                  GB2312转UCS2模块

***********************************************************************/

#ifndef __GB2312_UCS2_H
#define __GB2312_UCS2_H

//------------------GB2312转换成Ucs2函数-------------------------------
//返回0未找到
unsigned short GB2312ToUcs2(unsigned short GB2312);

//------------------Ucs2转换成GB2312函数-------------------------------
//此为反向查表，效率很低，且因两者间无什么规律可言，故直接按出现的概率查找
//返回0未找到
unsigned short Ucs2ToGB2312(unsigned short Ucs2);


#endif
 












