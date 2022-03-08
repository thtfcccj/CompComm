/***********************************************************************

//		               Adler32У��ģ��
��ģ����Ҫ����Deflateѹ���㷨��У�����ݵ���������
��ģ���޸ĳ���"lodepng->Adler32", ��л���ߣ���������
��ģ��Ϊ����������ѧϰ������������Ϊ����ӡ�񣬲������ֱ�������Է���ʹ��
***********************************************************************/
#ifndef _ADLER32_H
#define _ADLER32_H

//----------------------�õ�Adler32У����----------------------------
unsigned long Adler32_Get(unsigned long Adler,
                           const unsigned char *pData, 
                           unsigned long Len);

//----------------------�õ�Adler=1��32У����----------------------------
unsigned long Adler32_Get1(const unsigned char *pData, 
                            unsigned long Len);

#endif


