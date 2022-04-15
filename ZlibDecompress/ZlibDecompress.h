/*******************************************************************************

//		                 ZLIB����
��ģ���޸ĳ���"lodepnge", ��л���ߣ���������
��ģ��Ϊ����������ѧϰ����������Ϊ����ӡ�񣬲������ֱ�������Է���ʹ��
��ģ�����Ƕ��ʽϵͳ�����Ż�,���趯̬�����ڴ�
��ģ��֧�ֶ��̵߳��ã�
*******************************************************************************/
#ifndef _ZLIB_DECOMPRESS_H
#define _ZLIB_DECOMPRESS_H

#include "bReader.h"
#include "winWriter.h"
#include "DeflateNano.h"

//--------------------------------------���ṹ------------------------------
#define _ZlibDecompress   _DeflateNano //�ṹ������,ֱ�Ӽ̳�


//--------------------------�������ݻ���outʹ������---------------------------
//1�� out->Cfg, ����λ����Ϊ:
#define ZLIB_DECOMPRESS_EN_CHECK       0x40 //��������У��

//2�� out->U32Para ������Adler32�е�У��������

//--------------------------------ZLib��ѹ��------------------------------------
//ԭlodepng_zlib_decompressv
signed char ZlibDecompress(struct _ZlibDecompress *pZ,//�����ʼ��
                           const unsigned char *in, //ѹ�����ݰ�
                           brsize_t insize,           //idat�����ݸ���
                           winWriter_t *out);     //�������ݻ��壬������


//------------------------�ص�����:�õ�У����-------------------------------
unsigned long ZlibDecompress_cbGetAdler32(winWriter_t *out); //�������ݻ���

#endif //_PNG_IDAT_DECODER_H


