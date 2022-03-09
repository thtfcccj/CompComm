/*******************************************************************************

//		              Deflate ������ ������ʵ��

*******************************************************************************/

#include "DeflateNano.h"
#include <string.h>

//--------------------------���������ж���--------------------------------------
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
                             ��غ���ʵ��
*******************************************************************************/

//--------------------------����������괦����---------------------------------
//�˺���Ӧ����������ݴӺ����������ͬʱ��δ�������������ͷ��
//������������ʱ���ô˺���,���ظ��������
static signed char _OutDataEndPro(winWriter_t *out)
{
  if(out->LaterPro == NULL) return -1;//�쳣
  brsize_t used = out->LaterPro(out); //�����û�����
  if((used > out->start) || (used > out->capability))
    return  -2;//�����쳣 
  out->start -= used; //δ�����������
  memcpy(out->data, out->data + used, out->start);//��������������
  return 0;
}

//--------------------------�ӵ�ǰλ����ǰcopy����------------------------------
static signed char _CopyBackward(winWriter_t *out,
                                 brsize_t backward,//��ǰλ��(�ɵ�ǰstart��ʼ)
                                 brsize_t distance)//copy����
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
    //��������
    if((out->capability - start) < DEFLATE_NANO_OUT_LEAVED_SIZE){
      out->start = start;
      error = _OutDataEndPro(out);
      if(error) break;
      //�������ݱ������ˣ���ǰ����Ҫ����
      backward =- (start - out->start);
    }
  };
  return error;
}

//--------------------------��ѹ�����뺯��-------------------------------------
//ԭinflateNoCompression
static signed char _InflateNoCompression(bReader_t *reader,    //��׼���õ�����λ����
                                           winWriter_t *out) //��׼���õĽ������ݻ���
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
  if(!(out->Cfg & WIN_WRITER_IGNORE_NLEN) && ((Len + nLen) != 65535)){//����У�����
    return 21; /*error: nLen is not one's complement of Len*/
  }
  //��Դ����copy����
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

//--------------------------�ڲ������������뺯��-------------------------------------
//ԭinflateHuffmanBlock
static signed char _InflateHuffmanBlock(bReader_t *reader,    //��׼���õ�����λ����
                                 winWriter_t *out, //��׼���õĽ������ݻ���
                                 unsigned char btype)  //1�̶���������,����̬
{

  const HuffmanTree_t *tree_ll; /*the huffman tree for literal and length codes*/
  const HuffmanTree_t *tree_d; /*the huffman tree for distance codes*/
  int done = 0;

  signed char error = 0;
  if(btype == 1){//��׼����Ĺ̶���������
    tree_ll = HuffmanTree_pGetFixLL();
    tree_d = HuffmanTree_pGetFixD();    
  }
  else{ //if(btype == 2),�������ж�ȡ�Դ�����̬��������
    error = HuffmanTree_UpdateDync(DeflateNono_cbGetuffmanTreeMng(),
                                   DeflateNono_cbGetuffmanTreeBuf(),reader);
    if(error) return error; //���ɹ���������������
    tree_ll = HuffmanTree_pGetDyncLL();
    tree_d = HuffmanTree_pGetDyncD();     
  }

  while(!error && !done){ /*decode all symbols until end reached, breaks at end code*/ 
    /*code_ll is literal, length or end code*/
    unsigned short code_ll;
    /* ensure enough bits for 2 huffman code reads (15 bits each): if the first is a literal, a second literal is read at once. This
    appears to be slightly faster, than ensuring 20 bits here for 1 huffman symbol and the potential 5 extra bits for the length symbol.*/
    ensureBits32(reader, 30);//������ȡ�ֽ�(�Ѷ�ȡ�Ĳ���)��buffer��ȷ������30��bit
    code_ll = HuffmanTree_DecodeSymbol(reader, tree_ll);//�õ���ѹ��ĳ������¸��������ֵ
    if(code_ll <= 255) {//��ʾ�¸�ѹ���ֽ���
      /*slightly faster code path if multiple literals in a row*/
      out->data[out->start++] = (unsigned char)code_ll; //ѹ����ʵ����
      code_ll = HuffmanTree_DecodeSymbol(reader, tree_ll); //�����¸�ѹ�����ݽ���
    }
    if(code_ll <= 255) /*literal symbol*/ {//��ʾ�¸�ѹ���ֽ���
      out->data[out->start++] = (unsigned char)code_ll;//�����¸�ѹ�����ݽ���
    }
    //ֵΪ257,285ʱ����ʾ�����ˣ��˴ӱ���õ���������
    else if(code_ll >= HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX && 
            code_ll <= HUFFMAN_TREE_LAST_LENGTH_CODE_INDEX){ /*length code*/ 
      unsigned short code_d, distance;
      unsigned char numextrabits_l, numextrabits_d; /*extra bits for length and distance*/
      brsize_t start, backward, length;

      /*part 1: get length base*/;// 257-285ֵ��ʾ�ĳ���
      length = _LENGTHBASE[code_ll - HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX];

      /*part 2: get extra bits and add the value of that to length*/
      //�˳��Ȳ�����ʵ�����Ҫcopy�ĳ���ʱ�����������л�ȡ���ӵĳ���(���ǲ�ּ��ɱ䳤��)  
      numextrabits_l = _LENGTHEXTRA[code_ll - HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX];
      if(numextrabits_l != 0) {
        /* bits already ensured above */
        ensureBits25(reader, 5);
        length += readBits(reader, numextrabits_l);
      }

      /*part 3: get distance code*/
      ensureBits32(reader, 28); /* up to 15 for the huffman symbol, up to 13 for the extra bits *///������ȡ�������Ի�copy����
      code_d = HuffmanTree_DecodeSymbol(reader, tree_d); //��ѹ�õ��������
      if(code_d > 29) {
        if(code_d <= 31) {
          _ERROR_BREAK(18); /*error: invalid distance code (30-31 are never used)*/
        } else /* if(code_d == INVALIDSYMBOL) */{
          _ERROR_BREAK(16); /*error: tried to read disallowed huffman symbol*/
        }
      }
      distance = _DISTANCEBASE[code_d];//����������ʵ��copy����

      /*part 4: get extra bits from distance*/
      numextrabits_d = _DISTANCEEXTRA[code_d];//ʵ��copy���벻���Ա��ʵ��copy����ʱ���Ӵ��������л�ȡ���ӵľ�������(���ǲ�ּ��ɱ䳤�ȣ�ʡ������)
      if(numextrabits_d != 0) {
        /* bits already ensured above */
        distance += readBits(reader, numextrabits_d);//ȥ����ֺ����λ��
      }

      /*part 5: fill in all the out[n] values based on the length and dist*/
      start = out->start; //���Ծ���������copy���ݣ�startΪĿ������ӣ� ����������
      if(distance > start) _ERROR_BREAK(52); /*too long backward distance*/
      backward = start - distance;//��ǰcoopyλ��

      out->start += length;//Ԥ��copy����ô���
      //��Ҫcopy�����ݺܽ����������ݲ���copy��
      if(distance < length) {
        if(_CopyBackward(out, backward, distance)) _ERROR_BREAK(17);//���Ĳ���ֱ��memcpy
        start = out->start;//���»��壬out->start�Ѽ���distance��
        for(brsize_t forward = distance; forward < length; ++forward) {//�������ִ�����ǰ����
          out->data[start++] = out->data[backward++];
        }
      }else {//�ܿ�ǰ�㹻copy�ˣ�ֱ��memcpy
        if(_CopyBackward(out, backward, length)) //���Ĳ���ֱ��memcpy
          _ERROR_BREAK(17);
      }
    }
    else if(code_ll == 256) {//��ֵ��ʾ������
      done = 1; /*end code, finish the loop*/
    }
    else{ /*if(code_ll == INVALIDSYMBOL)*/ 
      _ERROR_BREAK(16); /*error: tried to read disallowed huffman symbol*/
    }
    //���¿ռ䲻����
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


//--------------------------------���뺯��-------------------------------------
//ԭinflateHuffmanBlock
signed char DeflateNano_Decoder(const unsigned char *data,//����������
                                 brsize_t insize,           //���ݸ���
                                 winWriter_t *out)     //��׼���õĽ������ݻ���
{
  if(bReader_SizeIsInvalid(insize)) return -1;//�������弫��
  bReader_t reader;
  bReader_Init(&reader, data, insize);
  
  signed char error = 0;
  unsigned BFINAL = 0; 
  while(!BFINAL) {
    unsigned BTYPE;
    if(reader.bitsize - reader.bp < 3) return 52; /*error, bit pointer will jump past memory*/
    ensureBits9(&reader, 3); //��������������3bit��ʵ��˫��
    BFINAL = readBits(&reader, 1);//�ӻ���ȡ��1bit,ͬʱ������λ1bit��BFINAL��λ��λ��ʾ����
    BTYPE = readBits(&reader, 2);//�����ӻ���ȡ��2bit

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



