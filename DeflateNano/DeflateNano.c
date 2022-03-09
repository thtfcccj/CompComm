/*******************************************************************************

//		              Deflate 精简型 解码器实现

*******************************************************************************/

#include "DeflateNano.h"
#include <string.h>

//--------------------------常量查找有定义--------------------------------------
/*the base lengths represented by codes 257-285*/
static const unsigned short _LENGTHBASE[29]= {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
  67, 83, 99, 115, 131, 163, 195, 227, 258};

/*the extra bits used by codes 257-285 (added to base length)*/
static const unsigned char _LENGTHEXTRA[29] = {
  0, 0, 0, 0, 0, 0, 0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
  4,  4,  4,   4,   5,   5,   5,   5,   0};

/*the base backwards distances (the bits of distance codes appear after 
length codes and use their own huffman tree)*/
static const unsigned short _DISTANCEBASE[30]  = {
  1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
  769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

/*the extra bits of backwards distances (added to base)*/
static const unsigned char _DISTANCEEXTRA[30]  = {
  0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,
  8,    9,    9,   10,   10,   11,   11,   12,    12,    13,    13};


#define _ERROR_BREAK(num)  return (num)

/*******************************************************************************
                             相关函数实现
*******************************************************************************/

//--------------------------输出数据用完处理函数---------------------------------
//此函数应将用完的数据从后面清除掉，同时将未用完的数据移至头部
//缓冲区将用完时调用此函数,返回负处理错误
static signed char _OutDataEndPro(winWriter_t *out)
{
  if(out->LaterPro == NULL) return -1;//异常
  brsize_t used = out->LaterPro(out); //交由用户处理
  if((used > out->start) || (used > out->capability))
    return  -2;//数据异常 
  out->start -= used; //未处理完的数据
  memcpy(out->data, out->data + used, out->start);//清掉处理掉的数据
  return 0;
}

//--------------------------从当前位置往前copy数据------------------------------
static signed char _CopyBackward(winWriter_t *out,
                                 brsize_t backward,//往前位置(由当前start开始)
                                 brsize_t distance)//copy距离
{
  signed char error = 0;
  while(distance){
    brsize_t start = out->start;
    brsize_t CurLen = out->capability - start;
    if(distance <= CurLen) CurLen = distance;
    memcpy(out->data + start, out->data + backward, CurLen);
    start += CurLen;    
    backward += CurLen;
    distance -= CurLen; 
    //数据满了
    if((out->capability - start) < DEFLATE_NANO_OUT_LEAVED_SIZE){
      out->start = start;
      error = _OutDataEndPro(out);
      if(error) break;
      //部分数据被移走了，当前距离要休正
      backward =- (start - out->start);
    }
  };
  return error;
}

//--------------------------不压缩译码函数-------------------------------------
//原inflateNoCompression
static signed char _InflateNoCompression(bReader_t *reader,    //已准备好的输入位码流
                                           winWriter_t *out) //已准备好的接收数据缓冲
{
  /*go to first boundary of byte*/
  brsize_t bytepos = (reader->bp + 7u) >> 3u;

  /*read Len (2 bytes) and nLen (2 bytes)*/
  if(bytepos + 4 >= reader->size) return 52; /*error, bit pointer will jump past memory*/
  unsigned short Len = (unsigned short)reader->data[bytepos] + 
                        ((unsigned short)reader->data[bytepos + 1] << 8u); 
  bytepos += 2;
  unsigned short nLen = (unsigned short)reader->data[bytepos] + 
                         ((unsigned short)reader->data[bytepos + 1] << 8u); 
  bytepos += 2;
  reader->bp = bytepos << 3u;

  /*check if 16-bit nLen is really the one's complement of Len*/
  if(!(out->Cfg & WIN_WRITER_IGNORE_NLEN) && ((Len + nLen) != 65535)){//长度校验错误
    return 21; /*error: nLen is not one's complement of Len*/
  }
  //从源到出copy数据
  signed char error = 0;
  while(Len){
    brsize_t CurLen = out->capability - out->start;
    if(Len < CurLen) CurLen = Len;
    memcpy(out->data + out->start, reader->data + bytepos, CurLen);
    bytepos += CurLen;
    out->start += CurLen;
    Len -= CurLen;
    if((out->capability - out->start) < DEFLATE_NANO_OUT_LEAVED_SIZE){
      error = _OutDataEndPro(out);
      if(error) break;
    }
  };
  
  reader->bp = bytepos << 3u; 
  return error;
}

//--------------------------内部哈夫曼树译码函数-------------------------------------
//原inflateHuffmanBlock
static signed char _InflateHuffmanBlock(bReader_t *reader,    //已准备好的输入位码流
                                 winWriter_t *out, //已准备好的接收数据缓冲
                                 unsigned char btype)  //1固定哈夫曼树,否则动态
{

  const HuffmanTree_t *tree_ll; /*the huffman tree for literal and length codes*/
  const HuffmanTree_t *tree_d; /*the huffman tree for distance codes*/
  int done = 0;

  signed char error = 0;
  if(btype == 1){//标准定义的固定哈夫曼树
    tree_ll = HuffmanTree_pGetFixLL();
    tree_d = HuffmanTree_pGetFixD();    
  }
  else{ //if(btype == 2),从数据中读取以创建动态哈夫曼树
    error = HuffmanTree_UpdateDync(DeflateNono_cbGetuffmanTreeMng(),
                                   DeflateNono_cbGetuffmanTreeBuf(),reader);
    if(error) return error; //不成功或输入数据有误
    tree_ll = HuffmanTree_pGetDyncLL();
    tree_d = HuffmanTree_pGetDyncD();     
  }

  while(!error && !done){ /*decode all symbols until end reached, breaks at end code*/ 
    /*code_ll is literal, length or end code*/
    unsigned short code_ll;
    /* ensure enough bits for 2 huffman code reads (15 bits each): if the first is a literal, a second literal is read at once. This
    appears to be slightly faster, than ensuring 20 bits here for 1 huffman symbol and the potential 5 extra bits for the length symbol.*/
    ensureBits32(reader, 30);//继续读取字节(已读取的不变)至buffer，确保最少30个bit
    code_ll = HuffmanTree_DecodeSymbol(reader, tree_ll);//得到解压后的长度与下个符号组合值
    if(code_ll <= 255) {//表示下个压缩字节数
      /*slightly faster code path if multiple literals in a row*/
      out->data[out->start++] = (unsigned char)code_ll; //压入真实数据
      code_ll = HuffmanTree_DecodeSymbol(reader, tree_ll); //继续下个压缩数据解码
    }
    if(code_ll <= 255) /*literal symbol*/ {//表示下个压缩字节数
      out->data[out->start++] = (unsigned char)code_ll;//继续下个压缩数据解码
    }
    //值为257,285时，表示长度了，此从表里得到具体数据
    else if(code_ll >= HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX && 
            code_ll <= HUFFMAN_TREE_LAST_LENGTH_CODE_INDEX){ /*length code*/ 
      unsigned short code_d, distance;
      unsigned char numextrabits_l, numextrabits_d; /*extra bits for length and distance*/
      brsize_t start, backward, length;

      /*part 1: get length base*/;// 257-285值表示的长度
      length = _LENGTHBASE[code_ll - HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX];

      /*part 2: get extra bits and add the value of that to length*/
      //此长度不能真实表达需要copy的长度时，从数据流中获取附加的长度(又是差分及可变长度)  
      numextrabits_l = _LENGTHEXTRA[code_ll - HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX];
      if(numextrabits_l != 0) {
        /* bits already ensured above */
        ensureBits25(reader, 5);
        length += readBits(reader, numextrabits_l);
      }

      /*part 3: get distance code*/
      ensureBits32(reader, 28); /* up to 15 for the huffman symbol, up to 13 for the extra bits *///继续读取至缓冲以获到copy距离
      code_d = HuffmanTree_DecodeSymbol(reader, tree_d); //解压得到距离代码
      if(code_d > 29) {
        if(code_d <= 31) {
          _ERROR_BREAK(18); /*error: invalid distance code (30-31 are never used)*/
        } else /* if(code_d == INVALIDSYMBOL) */{
          _ERROR_BREAK(16); /*error: tried to read disallowed huffman symbol*/
        }
      }
      distance = _DISTANCEBASE[code_d];//距离代码查表获到实际copy距离

      /*part 4: get extra bits from distance*/
      numextrabits_d = _DISTANCEEXTRA[code_d];//实际copy距离不足以表达实际copy距离时，从从数据流中获取附加的距离数据(又是差分及可变长度，省到家了)
      if(numextrabits_d != 0) {
        /* bits already ensured above */
        distance += readBits(reader, numextrabits_d);//去除差分后具体位置
      }

      /*part 5: fill in all the out[n] values based on the length and dist*/
      start = out->start; //从以经解码的数里，copy数据，start为目标会增加， 即滑动窗口
      if(distance > start) _ERROR_BREAK(52); /*too long backward distance*/
      backward = start - distance;//往前coopy位置

      out->start += length;//预置copy了这么多个
      //需要copy的数据很近且现在数据不够copy了
      if(distance < length) {
        if(_CopyBackward(out, backward, distance)) _ERROR_BREAK(17);//够的部分直接memcpy
        start = out->start;//重新缓冲，out->start已加上distance了
        for(brsize_t forward = distance; forward < length; ++forward) {//不够部分窗口向前滑动
          out->data[start++] = out->data[backward++];
        }
      }else {//很靠前足够copy了，直接memcpy
        if(_CopyBackward(out, backward, length)) //够的部分直接memcpy
          _ERROR_BREAK(17);
      }
    }
    else if(code_ll == 256) {//此值表示结束了
      done = 1; /*end code, finish the loop*/
    }
    else{ /*if(code_ll == INVALIDSYMBOL)*/ 
      _ERROR_BREAK(16); /*error: tried to read disallowed huffman symbol*/
    }
    //余下空间不多了
    if((out->capability - out->start) < DEFLATE_NANO_OUT_LEAVED_SIZE){
      error = _OutDataEndPro(out);
    }
    /*check if any of the ensureBits above went out of bounds*/
    if(reader->bp > reader->bitsize) {
      /*return error code 10 or 11 depending on the situation that happened in HuffmanTree_DecodeSymbol
      (10=no endcode, 11=wrong jump outside of tree)*/
      /* TODO: revise error codes 10,11,50: the above comment is no longer valid */
      _ERROR_BREAK(51); /*error, bit pointer jumps past memory*/
    }
  }

  return error;
}


//--------------------------------译码函数-------------------------------------
//原inflateHuffmanBlock
signed char DeflateNano_Decoder(const unsigned char *data,//输入数据流
                                 brsize_t insize,           //数据个数
                                 winWriter_t *out)     //已准备好的接收数据缓冲
{
  if(bReader_SizeIsInvalid(insize)) return -1;//超过缓冲极限
  bReader_t reader;
  bReader_Init(&reader, data, insize);
  
  signed char error = 0;
  unsigned BFINAL = 0; 
  while(!BFINAL) {
    unsigned BTYPE;
    if(reader.bitsize - reader.bp < 3) return 52; /*error, bit pointer will jump past memory*/
    ensureBits9(&reader, 3); //读出至缓存最少3bit，实际双字
    BFINAL = readBits(&reader, 1);//从缓存取出1bit,同时向下移位1bit，BFINAL此位置位表示结束
    BTYPE = readBits(&reader, 2);//继续从缓存取出2bit

    if(BTYPE == 3) 
      return 20; /*error: invalid BTYPE*/
    else if(BTYPE == 0)
      error = _InflateNoCompression(&reader, out); /*no compression*/
    else 
      error = _InflateHuffmanBlock(&reader, out, BTYPE); /*compression, BTYPE 01 or 10*/
    
    if(!error){
      if(out->MaxOutSize && out->start > out->MaxOutSize) error = 109;
    }
    if(error) break;
  }

  return error;

}



