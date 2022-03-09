/*******************************************************************************

//		                   ��������д���ݽṹ
��ģ����Ҫ����������ݿռ䲻��ʱ��
     ʹ��һ����������(���ѹ)��һ���������(��дFLASH,��ʾ��)�ķ�ʽ����
*******************************************************************************/
#ifndef _WIN_WRITER_H
#define _WIN_WRITER_H

#include "bReader.h" //brsize_t

/*******************************************************************************
                             ��ؽṹ
*******************************************************************************/

//������ݸ�ʽ
typedef struct _winWriter{
  //��������ݻ���,ԭucvector����       
  unsigned char *data;    //�����������,Ӧ���㹻�ռ�,�����ʹ����Ҫ��
  brsize_t capability;     //�����data�������� 
  volatile brsize_t start; //��ǰΪ��ʹ�õ�����  
  //��Ϣ�뽻�����֣�
  brsize_t MaxOutSize;       //��������������ݴ�С
  brsize_t OutedSize;        //�ⲿʹ�ã��Ѿ����������ݴ�С  
  unsigned char Cfg;        //����λ��������
  unsigned char U8Para;    //�û����������Ҫ�Ĳ���
  unsigned short U16Para;   //�û����������Ҫ�Ĳ���
  
  unsigned long Checksum;    //�Ѿ����������ݵ�У���
  //��size >= capabilityʱ���������ݴ���,�Դ����(�����ԣ������)�ѻ����������
  //�����õ��˶�������
  brsize_t (*LaterPro)(struct  _winWriter*); 
}winWriter_t;

//����λ����Ϊ
#define WIN_WRITER_IGNORE_NLEN    0x80 //���Գ��ȸ�У�飬ԭignore_nlen
#define WIN_WRITER_EN_CHECK       0x40 //��������У��
#define WIN_WRITER_USER_MASK      0x0F //��4bit�����û�����������ʹ��

/*******************************************************************************
                             ��غ���
*******************************************************************************/

//-------------------------------���ṹ��������------------------------------
//����󣬳�start��OutedSize�⣬�������û��ֶ�����
//void winWriter_Clr(winWriter *out);
#define winWriter_Clr(o) do{memset(o, 0, sizeof(winWriter_t)); }while(0)

#endif //_WIN_WRITER_H


