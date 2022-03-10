/*******************************************************************************

//		                            λ���ȡ��
��ģ����Ҫ���ڽ��ֽ���ת��Ϊλ����, ��Ҫ����Ƕ��ʽ����ԴҪ��ܵ͵ĳ���
��ģ���޸ĳ���"lodepng->LodePNGBitReader", ��л���ߣ���������
��ģ��Ϊ����������ѧϰ������������Ϊ����ӡ�񣬲������ֱ�������Է���ʹ��
��ģ�����Ƕ��ʽϵͳ�����˲����Ż�
*******************************************************************************/
#ifndef _BREADER_H
#define _BREADER_H

/*******************************************************************************
                             �������
********************************************************************************/

//ԭsize_t,�ֽ��� < =8192(65536/8)ʱ���ɿ�����unsigned short��
#ifndef brsize_t  
  #define  brsize_t unsigned short   //or unsigned long
  #define  brlen_t 2  //��shortʱ���ö���
#endif

/*******************************************************************************
                             ��ؽṹ
*******************************************************************************/

typedef struct _bReader{//��Ա����ԭ������ʽ
  const unsigned char *data; //ԭʼ����
  brsize_t size;              //data�ֽڴ�С
  brsize_t bitsize;          //������С��Ϊ�ֽ�*8
  brsize_t bp;               //�ѽ���������С
  unsigned long buffer;      //���ڱ������λ����
}bReader_t;

/*******************************************************************************
                             ��غ���
*******************************************************************************/

//-----------------------�ж��������Ƿ���-----------------------------------
//void bReader_SizeIsInvalid (unsigned long size);//ԭ��
#ifdef  brlen_t
  #define bReader_SizeIsInvalid(sz)  ((sz) > 0x1fff)
#else 
  #define bReader_SizeIsInvalid(sz)  ((sz) > 0x1fffffff)
#endif

//--------------------------------��ʼ������---------------------------------
//����ȷ�����ݳ���ʱ������ǰ����()bReader_SizeIsInvalid()ȷ����������
void bReader_Init(bReader_t *reader,
                  const unsigned char* data,
                  brsize_t size);
             
//-------------------------------��������9b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=9b��LSB��λ��ǰ
void bReader_2BufferB9(bReader_t* reader);
//����ʹ�ô˺�����˵��ʵ����Ҫ���ֽڸ�����
#define bReader_BufferB9(rd, needbits) do{bReader_2BufferB9(rd); }while(0)
#define ensureBits9(r,n)  bReader_BufferB9(r, n) //�����Կ���

//-------------------------------��������17b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=17b��LSB��λ��ǰ
void bReader_2BufferB17(bReader_t* reader);
//����ʹ�ô˺�����˵��ʵ����Ҫ���ֽڸ�����
#define bReader_BufferB17(rd, needbits) do{bReader_2BufferB17(rd); }while(0)
#define ensureBits17(r,n)  bReader_BufferB17(r, n) //�����Կ���

//-------------------------------��������25b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=25b��LSB��λ��ǰ
void bReader_2BufferB25(bReader_t* reader);
//����ʹ�ô˺�����˵��ʵ����Ҫ���ֽڸ�����
#define bReader_BufferB25(rd, needbits) do{bReader_2BufferB25(rd); }while(0)
#define ensureBits25(r,n)  bReader_BufferB25(r, n) //�����Կ���

//-------------------------------��������32b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=32b��LSB��λ��ǰ
void bReader_2BufferB32(bReader_t* reader);
//����ʹ�ô˺�����˵��ʵ����Ҫ���ֽڸ�����
#define bReader_BufferB32(rd, needbits) do{bReader_2BufferB32(rd); }while(0)
#define ensureBits32(r,n)  bReader_BufferB32(r, n) //�����Կ���

//-------------------------------������Ҫ��λ����-----------------------
//ԭpeekBits,�磺��Ҫ10λ������󷵻� (1 >> 10) - 1 = 1023
unsigned long bReader_PeekB(bReader_t* reader, 
                             unsigned char needbits);//��Ҫ��λ��
#define peekBits(r,n)  bReader_RdB(r,n)  //�����Կ���

//------------------------------�ƽ�λ����-------------------------------
//ԭAdvanceBits,����������ǰ�ƽ�
void bReader_AdvanceB(bReader_t* reader, 
                      unsigned char needbits);  //��Ҫ�ƽ���λ�� 
#define advanceBits(r,n)  bReader_RdB(r,n)  //�����Կ���

//------------------------------��ȡλ����-------------------------------
//ԭreadBits,�������ݣ�ͬʱ��������ǰ�ƽ�
unsigned long bReader_RdB(bReader_t* reader, 
                           unsigned char needbits);  //��Ҫ��ȡ���ƽ���λ�� 
#define readBits(r,n)  bReader_RdB(r,n)  //�����Կ���

#endif //_BREADER_H


