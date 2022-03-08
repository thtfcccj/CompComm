/*******************************************************************************

//		                    λ���ȡ�����ʵ��
��ģ����Ҫ���ڽ��ֽ���ת��Ϊλ����
*******************************************************************************/

#include "bReader.h"

/*******************************************************************************
                             ��غ���
*******************************************************************************/

//--------------------------------��ʼ������---------------------------------
void bReader_Init(bReader_t *reader,
                  const unsigned char* data,
                  brsize_t size)
{
  reader->data = data;
  reader->size = size;
  reader->bitsize = size << 3;
  reader->bp = 0;
  reader->buffer = 0;  
}


//-------------------------------��������9b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=9b
void bReader_2BufferB9(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 1u < size){//һ���Զ���
    reader->buffer = (unsigned short)reader->data[start + 0] | 
                     ((unsigned short)reader->data[start + 1] << 8u);
    reader->buffer >>= (reader->bp & 7u);
  }
  else{
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer = reader->data[start + 0];
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------��������17b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=17b
void bReader_2BufferB17(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 2u < size){//һ���Զ���
    reader->buffer = (unsigned long)reader->data[start + 0] |
                     ((unsigned long)reader->data[start + 1] << 8u) |
                     ((unsigned long)reader->data[start + 2] << 16u);
    reader->buffer >>= (reader->bp & 7u);
  }
  else{//����ٶ����٣�������0
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer |= reader->data[start + 0];
    if(start + 1u < size) reader->buffer |= ((unsigned long)reader->data[start + 1] << 8u);
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------��������25b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=25b
void bReader_2BufferB25(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 3u < size){//һ���Զ���
    reader->buffer = (unsigned long)reader->data[start + 0] | 
                     ((unsigned long)reader->data[start + 1] << 8u) |
                     ((unsigned long)reader->data[start + 2] << 16u) |
                     ((unsigned long)reader->data[start + 3] << 24u);
    reader->buffer >>= (reader->bp & 7u);
  }
  else{//����ٶ����٣�������0
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer |= reader->data[start + 0];
    if(start + 1u < size) reader->buffer |= ((unsigned long)reader->data[start + 1] << 8u);
    if(start + 2u < size) reader->buffer |= ((unsigned long)reader->data[start + 2] << 16u);
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------��������32b------------------------------------
//��ȡ���ݵ�buffer�У�ȷ����ǰ����buffer�ڵ������Чλ>=32b
void bReader_2BufferB32(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 4u < size){//һ���Զ���
    reader->buffer = (unsigned long)reader->data[start + 0] | 
                     ((unsigned long)reader->data[start + 1] << 8u) |
                     ((unsigned long)reader->data[start + 2] << 16u) | 
                     ((unsigned long)reader->data[start + 3] << 24u);
    reader->buffer >>= (reader->bp & 7u);
    reader->buffer |= (((unsigned long)reader->data[start + 4] << 24u) << 
                                                      (8u - (reader->bp & 7u)));
  }
  else{//����ٶ����٣�������0
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer |= reader->data[start + 0];
    if(start + 1u < size) reader->buffer |= ((unsigned long)reader->data[start + 1] << 8u);
    if(start + 2u < size) reader->buffer |= ((unsigned long)reader->data[start + 2] << 16u);
    if(start + 3u < size) reader->buffer |= ((unsigned long)reader->data[start + 3] << 24u);
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------������Ҫ��λ����-----------------------
//ԭpeekBits,�磺��Ҫ10λ������󷵻� (1 >> 10) - 1 = 1023
unsigned long bReader_PeekB(bReader_t* reader, 
                             unsigned char needbits)//��Ҫ��λ�� 
{
  return reader->buffer & ((1u << needbits) - 1u);
}

//------------------------------�ƽ�λ����-------------------------------
//ԭAdvanceBits,����������ǰ�ƽ�
void bReader_AdvanceB(bReader_t* reader, 
                      unsigned char needbits)  //��Ҫ�ƽ���λ�� 
{
  reader->buffer >>= needbits;
  reader->bp += needbits;
}

//------------------------------��ȡλ����-------------------------------
//ԭreadBits,�������ݣ�ͬʱ��������ǰ�ƽ�
unsigned long bReader_RdB(bReader_t* reader, 
                           unsigned char needbits)  //��Ҫ��ȡ���ƽ���λ�� 
{
  //Ϊ�ӿ��ٶȣ�ֱ��չ��
  unsigned long result = reader->buffer & ((1u << needbits) - 1u);
  reader->buffer >>= needbits;
  reader->bp += needbits;
  return result;
}

