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
  unsigned char *data;       //�����������,Ӧ���㹻�ռ�,�����ʹ����Ҫ��
  brsize_t capability;       //�����data�������� 
  volatile brsize_t start;  //��ǰΪ��ʹ�õ�����  
  //��Ϣ�뽻�����֣�
  brsize_t MaxOutSize;       //��������������ݴ�С
  brsize_t OutedSize;        //�ⲿʹ�ã��Ѿ����������ݴ�С
  brsize_t (*LaterPro)(struct  _winWriter*); //������ݴ���  
  //���ô˽ṹ�����ڽ������û�����(�ɺ����������붨��)��
  unsigned char Cfg;        //�û���Ҫ����λ
  unsigned char U8Para;     //�û����������Ҫ�Ĳ���
  unsigned short U16Para;   //�û����������Ҫ�Ĳ���
  unsigned long U32Para;    //�û����������Ҫ�Ĳ���

}winWriter_t;


/*******************************************************************************
                             ��غ���
*******************************************************************************/

//-------------------------------���ṹ��������------------------------------
//����󣬳�start��OutedSize�⣬�������û��ֶ�����
//void winWriter_Clr(winWriter *out);
#define winWriter_Clr(o) do{memset(o, 0, sizeof(winWriter_t)); }while(0)

//-------------------------����������괦����---------------------------------
//������������ʱ�����ô˺����ͳ����ݣ�����0�������,�����쳣
signed char winWriter_OutData(winWriter_t *out);

//-------------------------------copy����--------------------------------
//��copy���ǵ�copy��start�����������������ˣ���Ҫ�ͳ���������
signed char winWriter_Copy(winWriter_t *out,
                           const unsigned char *pData,//����
                           brsize_t DataLen,    //���ݳ���
                           brsize_t LeavedSize); //�����µ����ݿռ�

//--------------------------�ӵ�ǰλ����ǰcopy����------------------------------
//��copy���ڴ����������,copy��������ǰstartλ��,����0�������,�����쳣
//��copy���ǵ�copy��start�����������������ˣ���Ҫ�ͳ���������
signed char winWriter_CopyBackward(winWriter_t *out,
                                    brsize_t backward,//��ǰλ��(�ɵ�ǰstart��ʼ)
                                    brsize_t distance,//copy����
                                    brsize_t LeavedSize);//�����µ����ݿռ�


#endif //_WIN_WRITER_H


