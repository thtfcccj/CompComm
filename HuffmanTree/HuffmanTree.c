/*******************************************************************************

//		               ��������ģ��
��ģ����Ҫ���ڵĹ���������(����֧�ֱ���)���Խ�ѹѹ����ʽ�ļ���ͼƬ
��ģ���޸ĳ���"lodepng->HuffmanTree", ��л���ߣ���������
��ģ��Ϊ����������ѧϰ����������Ϊ����ӡ�񣬲������ֱ�������Է���ʹ��
��ģ�����Ƕ��ʽϵͳ�����Ż�,�̶���������Ϊ��������ά������̬�ṹ�Ҳ�������ڴ�
*******************************************************************************/

#include "HuffmanTree.h"
#include <string.h>

extern struct _HuffmanTree HuffmanTree[2]; //��̬�������ṹ
//---------------------------------�ڲ��궨��-----------------------------------
/* amount of bits for first huffman table lookup (aka root bits), see HuffmanTree_makeTable and HuffmanTree_DecodeSymbol.*/
/* values 8u and 9u work the fastest */
#define FIRSTBITS     9u

#define INVALIDSYMBOL 65535u

#define FIRST_LENGTH_CODE_INDEX   257
#define LAST_LENGTH_CODE_INDEX    285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define NUM_DEFLATE_CODE_SYMBOLS   288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define NUM_DISTANCE_SYMBOLS       32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define NUM_CODE_LENGTH_CODES      19

//-----------------------------HuffmanTree--------------------------------
//HuffmanTree_UpdateDync�в���,makeTable()��Ҫ
static unsigned long bitlen_ll[NUM_DEFLATE_CODE_SYMBOLS];//tree_ll��
static unsigned long bitlen_d[NUM_DISTANCE_SYMBOLS]; //tree_d��
//_makeFromLengths2()��makeTable()�д���
static unsigned long _codes[NUM_DEFLATE_CODE_SYMBOLS];//tree_ll,tree_d����

//-----------------------------�ڲ���ʱ��������--------------------------------
union {
  struct{//_makeFromLengths2()��ʹ��, ��������_codes
    unsigned long blcount[16];
    unsigned long nextcode[16];
  }makecodes;
  struct{//makeTable()��ʹ�ã�
    unsigned long maxlens[1u << FIRSTBITS];
  }makeTables;
}u;

//�����Ҫ�ģ���̬�������ṹ���Ȳ��ұ�, _makeTable()�в���
static unsigned char table_len[2][1u << FIRSTBITS]; 
//�����Ҫ�ģ���̬�������ṹֵ���ұ�, _makeTable()�в���
static unsigned short table_value[2][1u << FIRSTBITS]; 

#define maxlens  u.makeTables.maxlens
#define _table_len(id)  table_len[id]
#define _table_value(id)  table_value[id]
#define _blcount  u.makecodes.blcount
#define _nextcode  u.makecodes.nextcode

//�ڲ����ã�
#define HAFFMAN_TREE_CL      0  //0
#define tree_cl  HuffmanTree[0] //���ã����治����
#define bitlen_cl  bitlen_ll    //���ã����治����
//---------------------------------��������-----------------------------
/*the order in which "code length alphabet code lengths" are stored as specified by deflate, out of this the huffman
tree of the dynamic huffman tree lengths is generated*/
static const unsigned char CLCL_ORDER[NUM_CODE_LENGTH_CODES]= {
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

/*******************************************************************************
                             ��غ���
*******************************************************************************/

//---------------------------------�ڲ��꺯��-----------------------------
#define _ERROR_BREAK(num)  return (num)
#define _gtofl(bp, needbit, count)  (((bp) + (needbit)) > (count))

#ifndef _MAX
  #define _MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif 

#ifndef _MIN
  #define _MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _ABS
  #define _ABS(x) ((x) < 0 ? -(x) : (x))
#endif

//---------------------------------�ṹ��ʼ��-----------------------------
static void _init(HuffmanTree_t *tree)
{
  tree->codes = 0;
  tree->lengths = 0;
  tree->table_len = 0;
  tree->table_value = 0;
}

static unsigned reverseBits(unsigned bits, unsigned num) {
  /*TODO: implement faster lookup table based version when needed*/
  unsigned i, result = 0;
  for(i = 0; i < num; i++) result |= ((bits >> (num - i - 1u)) & 1u) << i;
  return result;
}

//----------------------------���ɲ��ұ�-----------------------------
//����table_len��table_value���ڽ���ʱ����HuffmanTree_makeTable
//�õ�������Ҫ�Ĳ��ұ�
static signed char _makeTable(unsigned char id)
{
  HuffmanTree_t* tree = &HuffmanTree[id];
  
  static const unsigned headsize = 1u << FIRSTBITS; /*size of the first table*/
  static const unsigned mask = (1u << FIRSTBITS) /*headsize*/ - 1u;
  size_t i, numpresent, pointer, size; /*total table size*/
  
  /* compute maxlens: max total bit length of symbols sharing prefix in the first table*/
  memset(maxlens, 0, headsize * sizeof(*maxlens));
  for(i = 0; i < tree->numcodes; i++) {
    unsigned symbol = tree->codes[i];
    unsigned l = tree->lengths[i];
    unsigned index;
    if(l <= FIRSTBITS) continue; /*symbols that fit in first table don't increase secondary table size*/
    /*get the FIRSTBITS MSBs, the MSBs of the symbol are encoded first. See later comment about the reversing*/
    index = reverseBits(symbol >> (l - FIRSTBITS), FIRSTBITS);
    maxlens[index] = _MAX(maxlens[index], l);
  }
  /* compute total table size: size of first table plus all secondary tables for symbols longer than FIRSTBITS */
  size = headsize;
  for(i = 0; i < headsize; ++i) {
    unsigned l = maxlens[i];
    if(l > FIRSTBITS) size += (1u << (l - FIRSTBITS));
  }
  tree->table_len = _table_len(id);
  tree->table_value = _table_value(id);   
  
  /*initialize with an invalid length to indicate unused entries*/
  for(i = 0; i < size; ++i) tree->table_len[i] = 16;

  /*fill in the first table for long symbols: max prefix size and pointer to secondary tables*/
  pointer = headsize;
  for(i = 0; i < headsize; ++i) {
    unsigned l = maxlens[i];
    if(l <= FIRSTBITS) continue;
    tree->table_len[i] = l;
    tree->table_value[i] = pointer;
    pointer += (1u << (l - FIRSTBITS));
  }

  /*fill in the first table for short symbols, or secondary table for long symbols*/
  numpresent = 0;
  for(i = 0; i < tree->numcodes; ++i) {
    unsigned l = tree->lengths[i];
    unsigned symbol = tree->codes[i]; /*the huffman bit pattern. i itself is the value.*/
    /*reverse bits, because the huffman bits are given in MSB first order but the bit reader reads LSB first*/
    unsigned reverse = reverseBits(symbol, l);
    if(l == 0) continue;
    numpresent++;

    if(l <= FIRSTBITS){
      /*short symbol, fully in first table, replicated num times if l < FIRSTBITS*/
      unsigned num = 1u << (FIRSTBITS - l);
      unsigned j;
      for(j = 0; j < num; ++j) {
        /*bit reader will read the l bits of symbol first, the remaining FIRSTBITS - l bits go to the MSB's*/
        unsigned index = reverse | (j << l);
        if(tree->table_len[index] != 16) return 55; /*invalid tree: long symbol shares prefix with short symbol*/
        tree->table_len[index] = l;
        tree->table_value[index] = i;
      }
    } 
    else {
      /*long symbol, shares prefix with other long symbols in first lookup table, needs second lookup*/
      /*the FIRSTBITS MSBs of the symbol are the first table index*/
      unsigned index = reverse & mask;
      unsigned maxlen = tree->table_len[index];
      /*log2 of secondary table length, should be >= l - FIRSTBITS*/
      unsigned tablelen = maxlen - FIRSTBITS;
      unsigned start = tree->table_value[index]; /*starting index in secondary table*/
      unsigned num = 1u << (tablelen - (l - FIRSTBITS)); /*amount of entries of this symbol in secondary table*/
      unsigned j;
      if(maxlen < l) return 55; /*invalid tree: long symbol shares prefix with short symbol*/
      for(j = 0; j < num; ++j) {
        unsigned reverse2 = reverse >> FIRSTBITS; /* l - FIRSTBITS bits */
        unsigned index2 = start + (reverse2 | (j << (l - FIRSTBITS)));
        tree->table_len[index2] = l;
        tree->table_value[index2] = i;
      }
    }
  }

  if(numpresent < 2) {
    /* In case of exactly 1 symbol, in theory the huffman symbol needs 0 bits,
    but deflate uses 1 bit instead. In case of 0 symbols, no symbols can
    appear at all, but such huffman tree could still exist (e.g. if distance
    codes are never used). In both cases, not all symbols of the table will be
    filled in. Fill them in with an invalid symbol value so returning them from
    HuffmanTree_DecodeSymbol will cause error. */
    for(i = 0; i < size; ++i) {
      if(tree->table_len[i] == 16) {
        /* As length, use a value smaller than FIRSTBITS for the head table,
        and a value larger than FIRSTBITS for the secondary table, to ensure
        valid behavior for advanceBits when reading this symbol. */
        tree->table_len[i] = (i < headsize) ? 1 : (FIRSTBITS + 1);
        tree->table_value[i] = INVALIDSYMBOL;
      }
    }
  } else {
    /* A good huffman tree has N * 2 - 1 nodes, of which N - 1 are internal nodes.
    If that is not the case (due to too long length codes), the table will not
    have been fully used, and this is an error (not all bit combinations can be
    decoded): an oversubscribed huffman tree, indicated by error 55. */
    for(i = 0; i < size; ++i) {
      if(tree->table_len[i] == 16) return 55;
    }
  }

  return 0;
}


//----------------------------�ӳ���������2-----------------------------
//�������tree->codes��ԭHuffmanTree_makeFromLengths2
static void _makeFromLengths2(HuffmanTree_t* tree)
{
  unsigned bits, n;

  tree->codes = _codes;

  for(n = 0; n != tree->maxbitlen + 1; n++) _blcount[n] = _nextcode[n] = 0;//��ʼ��Ϊ0
  /*step 1: count number of instances of each code length*/
  for(bits = 0; bits != tree->numcodes; ++bits) ++_blcount[tree->lengths[bits]];//blcount�ۼ��ϵ�ǰ���ֵĳ���
  /*step 2: generate the nextcode values*/
  for(bits = 1; bits <= tree->maxbitlen; ++bits) {
    _nextcode[bits] = (_nextcode[bits - 1] + _blcount[bits - 1]) << 1u;
  }
  /*step 3: generate all the codes*/
  for(n = 0; n != tree->numcodes; ++n) {
    if(tree->lengths[n] != 0) {
      tree->codes[n] = _nextcode[tree->lengths[n]]++;
      /*remove superfluous bits from the code*/
      tree->codes[n] &= ((1u << tree->lengths[n]) - 1u);
    }
  }
}

//----------------------------�ӳ���������-------------------------------
static signed char _makeFromLengths(unsigned char id,
                                    unsigned long * bitlen,
                                  brsize_t numcodes, 
                                  unsigned long maxbitlen)
{
  HuffmanTree_t* tree = &HuffmanTree[id];
  tree->lengths = bitlen;
  tree->numcodes = (unsigned)numcodes; /*number of symbols*/
  tree->maxbitlen = maxbitlen;
  _makeFromLengths2(tree);  //����tree->codes
  //����table_len��table_value���ڽ���ʱ����HuffmanTree_makeTable
  //�õ�������Ҫ�Ĳ��ұ�
  return _makeTable(id);
}
//ԭ����
#define HuffmanTree_makeFromLengths(t,b,n,m) _makeFromLengths(t,b,n,m)
//----------------------------���¶�̬�������ṹ-------------------------------
//ԭgetTreeInflateDynamic, ����ǰ����,���ط�0����
//�˺�������λ�����¶�̬�ṹ
signed short HuffmanTree_UpdateDync(bReader_t *reader)
{
 /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated*/
  unsigned error = 0;
  unsigned n, HLIT, HDIST, HCLEN, i;

  if(reader->bitsize - reader->bp < 14) return 49; /*error: the bit pointer is or will go past the memory*/ //
  ensureBits17(reader, 14); //ȡ��14b: 5b����+5b����+4b�볤HCLEN

  /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already*/
  HLIT =  readBits(reader, 5) + 257; //ȡ��5bit����
  /*number of distance codes. Unlike the spec, the value 1 is added to it here already*/
  HDIST = readBits(reader, 5) + 1;
  /*number of code length codes. Unlike the spec, the value 4 is added to it here already*/
  HCLEN = readBits(reader, 4) + 4;//HCLEN���=2^4+4=20;

  _init(&tree_cl);

  while(!error) {//����
    /*read the code length codes out of 3 * (amount of code length codes) bits*/
    if(_gtofl(reader->bp, HCLEN * 3, reader->bitsize)) {
      _ERROR_BREAK(50); /*error: the bit pointer is or will go past the memory*/
    }
    for(i = 0; i != HCLEN; ++i) {//ÿ��ȡ3b������bitlen_cl��ֱ�������볤
      ensureBits9(reader, 3); /*out of bounds already checked above */
      bitlen_cl[CLCL_ORDER[i]] = readBits(reader, 3);
    }
    for(i = HCLEN; i != NUM_CODE_LENGTH_CODES; ++i) {//δʹ�ò�����Ϊ0
      bitlen_cl[CLCL_ORDER[i]] = 0;
    }

    error = _makeFromLengths(HAFFMAN_TREE_CL, bitlen_cl, NUM_CODE_LENGTH_CODES, 7); //�õ����ֵ
    if(error) break;

    memset(bitlen_ll, 0, NUM_DEFLATE_CODE_SYMBOLS * sizeof(*bitlen_ll));
    memset(bitlen_d, 0, NUM_DISTANCE_SYMBOLS * sizeof(*bitlen_d));

    /*i is the current symbol we're reading in the part that contains the code lengths of lit/len and dist codes*/
    i = 0;
    while(i < HLIT + HDIST) {
      unsigned code;
      ensureBits25(reader, 22); /* up to 15 bits for huffman code, up to 7 extra bits below*/
      code = HuffmanTree_DecodeSymbol(reader, &tree_cl);
      if(code <= 15) /*a length code*/ {
        if(i < HLIT) bitlen_ll[i] = code;
        else bitlen_d[i - HLIT] = code;
        ++i;
      }
      else if(code == 16){ /*repeat previous*/ 
        unsigned replength = 3; /*read in the 2 bits that indicate repeat length (3-6)*/
        unsigned value; /*set value to the previous code*/

        if(i == 0) _ERROR_BREAK(54); /*can't repeat previous if i is 0*/

        replength += readBits(reader, 2);

        if(i < HLIT + 1) value = bitlen_ll[i - 1];
        else value = bitlen_d[i - HLIT - 1];
        /*repeat this value in the next lengths*/
        for(n = 0; n < replength; ++n) {
          if(i >= HLIT + HDIST) _ERROR_BREAK(13); /*error: i is larger than the amount of codes*/
          if(i < HLIT) bitlen_ll[i] = value;
          else bitlen_d[i - HLIT] = value;
          ++i;
        }
      } 
      else if(code == 17){ /*repeat "0" 3-10 times*/ 
        unsigned replength = 3; /*read in the bits that indicate repeat length*/
        replength += readBits(reader, 3);

        /*repeat this value in the next lengths*/
        for(n = 0; n < replength; ++n) {
          if(i >= HLIT + HDIST) _ERROR_BREAK(14); /*error: i is larger than the amount of codes*/

          if(i < HLIT) bitlen_ll[i] = 0;
          else bitlen_d[i - HLIT] = 0;
          ++i;
        }
      } 
      else if(code == 18){ /*repeat "0" 11-138 times*/ 
        unsigned replength = 11; /*read in the bits that indicate repeat length*/
        replength += readBits(reader, 7);

        /*repeat this value in the next lengths*/
        for(n = 0; n < replength; ++n) {
          if(i >= HLIT + HDIST) _ERROR_BREAK(15); /*error: i is larger than the amount of codes*/

          if(i < HLIT) bitlen_ll[i] = 0;
          else bitlen_d[i - HLIT] = 0;
          ++i;
        }
      }
      else{ /*if(code == INVALIDSYMBOL)*/
        _ERROR_BREAK(16); /*error: tried to read disallowed huffman symbol*/
      }
      /*check if any of the ensureBits above went out of bounds*/
      if(reader->bp > reader->bitsize) {
        /*return error code 10 or 11 depending on the situation that happened in HuffmanTree_DecodeSymbol
        (10=no endcode, 11=wrong jump outside of tree)*/
        /* TODO: revise error codes 10,11,50: the above comment is no longer valid */
        _ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
      }
    }
    if(error) break;

    if(bitlen_ll[256] == 0) _ERROR_BREAK(64); /*the length of the end code 256 must be larger than 0*/

    /*now we've finally got HLIT and HDIST, so generate the code trees, and the function is done*/
    error = HuffmanTree_makeFromLengths(HAFFMAN_TREE_LL, bitlen_ll, NUM_DEFLATE_CODE_SYMBOLS, 15);
    if(error) break;
    error = HuffmanTree_makeFromLengths(HAFFMAN_TREE_D, bitlen_d, NUM_DISTANCE_SYMBOLS, 15);

    break; /*end of error-while*/
  }
  
  return 0;
}

//---------------------------------����������-----------------------------
//ԭHuffmanTree_DecodeSymbol
unsigned short HuffmanTree_DecodeSymbol(bReader_t *reader, 
                                         const HuffmanTree_t* codetree)
{
  unsigned short code = peekBits(reader, FIRSTBITS);//�ӻ�������ȡ��Ӧ��������
  unsigned short l = codetree->table_len[code];//�鵽�ģ���ѹ�����ݵĳ���
  unsigned short value = codetree->table_value[code];//��ѯ����ֵ
  if(l <= FIRSTBITS) {
    advanceBits(reader, l); //λ��ȡ�ˣ���ǰ�ƽ�,��buffer����lλ
    return value;
  }
  else{
    advanceBits(reader, FIRSTBITS);
    value += peekBits(reader, l - FIRSTBITS);
    advanceBits(reader, codetree->table_len[value] - FIRSTBITS);
    return codetree->table_value[value];
  }
}
