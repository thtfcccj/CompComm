/*******************************************************************************

//		                    位码读取器相关实现
此模块主要用于将字节流转换为位码流
*******************************************************************************/

#include "bReader.h"

/*******************************************************************************
                             相关函数
*******************************************************************************/

//--------------------------------初始化函数---------------------------------
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


//-------------------------------缓冲至少9b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=9b
void bReader_2BufferB9(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 1u < size){//一次性读完
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

//-------------------------------缓冲至少17b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=17b
void bReader_2BufferB17(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 2u < size){//一次性读完
    reader->buffer = (unsigned long)reader->data[start + 0] |
                     ((unsigned long)reader->data[start + 1] << 8u) |
                     ((unsigned long)reader->data[start + 2] << 16u);
    reader->buffer >>= (reader->bp & 7u);
  }
  else{//余多少读多少，不够补0
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer |= reader->data[start + 0];
    if(start + 1u < size) reader->buffer |= ((unsigned long)reader->data[start + 1] << 8u);
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------缓冲至少25b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=25b
void bReader_2BufferB25(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 3u < size){//一次性读完
    reader->buffer = (unsigned long)reader->data[start + 0] | 
                     ((unsigned long)reader->data[start + 1] << 8u) |
                     ((unsigned long)reader->data[start + 2] << 16u) |
                     ((unsigned long)reader->data[start + 3] << 24u);
    reader->buffer >>= (reader->bp & 7u);
  }
  else{//余多少读多少，不够补0
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer |= reader->data[start + 0];
    if(start + 1u < size) reader->buffer |= ((unsigned long)reader->data[start + 1] << 8u);
    if(start + 2u < size) reader->buffer |= ((unsigned long)reader->data[start + 2] << 16u);
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------缓冲至少32b------------------------------------
//读取数据到buffer中，确保当前缓冲buffer内的最低有效位>=32b
void bReader_2BufferB32(bReader_t* reader)
{
  brsize_t start = reader->bp >> 3u;
  brsize_t size = reader->size;
  if(start + 4u < size){//一次性读完
    reader->buffer = (unsigned long)reader->data[start + 0] | 
                     ((unsigned long)reader->data[start + 1] << 8u) |
                     ((unsigned long)reader->data[start + 2] << 16u) | 
                     ((unsigned long)reader->data[start + 3] << 24u);
    reader->buffer >>= (reader->bp & 7u);
    reader->buffer |= (((unsigned long)reader->data[start + 4] << 24u) << 
                                                      (8u - (reader->bp & 7u)));
  }
  else{//余多少读多少，不够补0
    reader->buffer = 0;
    if(start + 0u < size) reader->buffer |= reader->data[start + 0];
    if(start + 1u < size) reader->buffer |= ((unsigned long)reader->data[start + 1] << 8u);
    if(start + 2u < size) reader->buffer |= ((unsigned long)reader->data[start + 2] << 16u);
    if(start + 3u < size) reader->buffer |= ((unsigned long)reader->data[start + 3] << 24u);
    reader->buffer >>= (reader->bp & 7u);
  }
}

//-------------------------------窥视需要的位数据-----------------------
//原peekBits,如：需要10位，则最大返回 (1 >> 10) - 1 = 1023
unsigned long bReader_PeekB(bReader_t* reader, 
                             unsigned char needbits)//需要的位数 
{
  return reader->buffer & ((1u << needbits) - 1u);
}

//------------------------------推进位数据-------------------------------
//原AdvanceBits,将数据流向前推进
void bReader_AdvanceB(bReader_t* reader, 
                      unsigned char needbits)  //需要推进的位数 
{
  reader->buffer >>= needbits;
  reader->bp += needbits;
}

//------------------------------读取位数据-------------------------------
//原readBits,返回数据，同时将数据向前推进
unsigned long bReader_RdB(bReader_t* reader, 
                           unsigned char needbits)  //需要读取与推进的位数 
{
  //为加快速度，直接展开
  unsigned long result = reader->buffer & ((1u << needbits) - 1u);
  reader->buffer >>= needbits;
  reader->bp += needbits;
  return result;
}

