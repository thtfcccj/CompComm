/*******************************************************************************

//		               哈夫曼树模块
此模块主要用于的哈夫曼解码(不能支持编码)，以解压压缩格式文件或图片
此模块修改出自"lodepng->HuffmanTree", 感谢作者！！！！！
此模块为重新整理以学习分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统做了优化,固定哈夫曼改为常量，并维护两动态结构且不需分配内存
*******************************************************************************/

#include "HuffmanTree.h"
#include <string.h>

//struct _HuffmanTreeMng _HuffmanTreeMng;//容量测试
//struct _HuffmanTreeBuf _HuffmanTreeBuf;//容量测试

/*******************************************************************************
                             相关变量
*******************************************************************************/

//------------------------pHuffmanTreeMng内相关操作宏---------------------------
#define _HuffmanTree(id)  pMng->HuffmanTree[id]
#define _table_len(id)    pMng->table_len[id]
#define _table_value(id)  pMng->table_value[id]

//------------------------HuffmanTreeBuf内相关操作宏-----------------------------
#define _codes      pBuf->m.codes
#define _bitlen_ll  pBuf->m.bitlen_ll
#define _bitlen_d   pBuf->m.bitlen_d

#define _blcount   pBuf->u.makecodes.blcount
#define _nextcode  pBuf->u.makecodes.nextcode
#define _maxlens   pBuf->u.makeTables.maxlens

//内部复用：
#define _HUFFMAN_TREE_CL  0  //0
#define _tree_cl          _HuffmanTree(0) //先用，后面不用了
#define _bitlen_cl        _bitlen_ll      //先用，后面不用了

//---------------------------------常量表定义-----------------------------
/*the order in which "code length alphabet code lengths" are stored as specified by deflate, out of this the huffman
tree of the dynamic huffman tree lengths is generated*/
static const unsigned char CLCL_ORDER[HUFFMAN_TREE_NUM_CODE_LENGTH_CODES]= {
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

/*******************************************************************************
                             相关函数
*******************************************************************************/

//---------------------------------内部宏函数-----------------------------
#define _ERROR_BREAK(num)  return (num)
#define _gtofl(bp, needbit, count)  (((bp) + (needbit)) > (count))

#ifndef _MAX0
  #define _MAX0(a, b) (((a) > (b)) ? (a) : (b))
#endif 

static unsigned reverseBits(unsigned bits, unsigned num) {
  /*TODO: implement faster lookup table based version when needed*/
  unsigned i, result = 0;
  for(i = 0; i < num; i++) result |= ((bits >> (num - i - 1u)) & 1u) << i;
  return result;
}

//---------------------------------结构初始化-----------------------------
static void _init(HuffmanTree_t *tree)
{
  tree->codes = 0;
  tree->lengths = 0;
  tree->table_len = 0;
  tree->table_value = 0;
}

//----------------------------生成查找表-----------------------------
//生成table_len，table_value以在解码时调用HuffmanTree_makeTable
//得到解码需要的查找表
static signed char _makeTable(struct _HuffmanTreeMng *pMng,
                               struct _HuffmanTreeBuf *pBuf,
                               unsigned char id)
{
  HuffmanTree_t* tree = &_HuffmanTree(id);
  
  static const unsigned headsize = 1u << HUFFMAN_TREE_FIRSTBITS; /*size of the first table*/
  static const unsigned mask = (1u << HUFFMAN_TREE_FIRSTBITS) /*headsize*/ - 1u;
  size_t i, numpresent, pointer, size; /*total table size*/
  
  unsigned long *maxlens = _maxlens;
  /* compute maxlens: max total bit length of symbols sharing prefix in the first table*/
  memset(maxlens, 0, headsize * sizeof(*maxlens));
  for(i = 0; i < tree->numcodes; i++) {
    unsigned symbol = tree->codes[i];
    unsigned l = tree->lengths[i];
    unsigned index;
    if(l <= HUFFMAN_TREE_FIRSTBITS) continue; /*symbols that fit in first table don't increase secondary table size*/
    /*get the HUFFMAN_TREE_FIRSTBITS MSBs, the MSBs of the symbol are encoded first. See later comment about the reversing*/
    index = reverseBits(symbol >> (l - HUFFMAN_TREE_FIRSTBITS), HUFFMAN_TREE_FIRSTBITS);
    maxlens[index] = _MAX0(maxlens[index], l);
  }
  /* compute total table size: size of first table plus all secondary tables for symbols longer than HUFFMAN_TREE_FIRSTBITS */
  size = headsize;
  for(i = 0; i < headsize; ++i) {
    unsigned l = maxlens[i];
    if(l > HUFFMAN_TREE_FIRSTBITS) size += (1u << (l - HUFFMAN_TREE_FIRSTBITS));
  }
  tree->table_len = _table_len(id);
  tree->table_value = _table_value(id);   
  
  /*initialize with an invalid length to indicate unused entries*/
  for(i = 0; i < size; ++i) tree->table_len[i] = 16;

  /*fill in the first table for long symbols: max prefix size and pointer to secondary tables*/
  pointer = headsize;
  for(i = 0; i < headsize; ++i) {
    unsigned l = maxlens[i];
    if(l <= HUFFMAN_TREE_FIRSTBITS) continue;
    tree->table_len[i] = l;
    tree->table_value[i] = pointer;
    pointer += (1u << (l - HUFFMAN_TREE_FIRSTBITS));
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

    if(l <= HUFFMAN_TREE_FIRSTBITS){
      /*short symbol, fully in first table, replicated num times if l < HUFFMAN_TREE_FIRSTBITS*/
      unsigned num = 1u << (HUFFMAN_TREE_FIRSTBITS - l);
      unsigned j;
      for(j = 0; j < num; ++j) {
        /*bit reader will read the l bits of symbol first, the remaining HUFFMAN_TREE_FIRSTBITS - l bits go to the MSB's*/
        unsigned index = reverse | (j << l);
        if(tree->table_len[index] != 16) return 55; /*invalid tree: long symbol shares prefix with short symbol*/
        tree->table_len[index] = l;
        tree->table_value[index] = i;
      }
    } 
    else {
      /*long symbol, shares prefix with other long symbols in first lookup table, needs second lookup*/
      /*the HUFFMAN_TREE_FIRSTBITS MSBs of the symbol are the first table index*/
      unsigned index = reverse & mask;
      unsigned maxlen = tree->table_len[index];
      /*log2 of secondary table length, should be >= l - HUFFMAN_TREE_FIRSTBITS*/
      unsigned tablelen = maxlen - HUFFMAN_TREE_FIRSTBITS;
      unsigned start = tree->table_value[index]; /*starting index in secondary table*/
      unsigned num = 1u << (tablelen - (l - HUFFMAN_TREE_FIRSTBITS)); /*amount of entries of this symbol in secondary table*/
      unsigned j;
      if(maxlen < l) return 55; /*invalid tree: long symbol shares prefix with short symbol*/
      for(j = 0; j < num; ++j) {
        unsigned reverse2 = reverse >> HUFFMAN_TREE_FIRSTBITS; /* l - HUFFMAN_TREE_FIRSTBITS bits */
        unsigned index2 = start + (reverse2 | (j << (l - HUFFMAN_TREE_FIRSTBITS)));
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
        /* As length, use a value smaller than HUFFMAN_TREE_FIRSTBITS for the head table,
        and a value larger than HUFFMAN_TREE_FIRSTBITS for the secondary table, to ensure
        valid behavior for advanceBits when reading this symbol. */
        tree->table_len[i] = (i < headsize) ? 1 : (HUFFMAN_TREE_FIRSTBITS + 1);
        tree->table_value[i] = HUFFMAN_TREE_INVALIDSYMBOL;
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


//----------------------------从长度生成树2-----------------------------
//建立码表tree->codes，原_makeFromLengths2
static void _makeFromLengths2(struct _HuffmanTreeMng *pMng,
                               struct _HuffmanTreeBuf *pBuf,
                              HuffmanTree_t* tree)
{
  unsigned bits, n;
  unsigned long *codes = _codes;
  tree->codes = codes;

  unsigned long *nextcode = _nextcode;  
  unsigned long *blcount = _blcount;
  for(n = 0; n != tree->maxbitlen + 1; n++) blcount[n] = nextcode[n] = 0;//初始化为0
  /*step 1: count number of instances of each code length*/
  for(bits = 0; bits != tree->numcodes; ++bits) ++blcount[tree->lengths[bits]];//blcount累加上当前出现的长度
  /*step 2: generate the nextcode values*/
  for(bits = 1; bits <= tree->maxbitlen; ++bits) {
    nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1u;
  }
  /*step 3: generate all the codes*/
  for(n = 0; n != tree->numcodes; ++n) {
    if(tree->lengths[n] != 0) {
      codes[n] = nextcode[tree->lengths[n]]++;
      /*remove superfluous bits from the code*/
      codes[n] &= ((1u << tree->lengths[n]) - 1u);
    }
  }
}

//----------------------------从长度生成树-------------------------------
static signed char _makeFromLengths(struct _HuffmanTreeMng *pMng,
                                     struct _HuffmanTreeBuf *pBuf,
                                     unsigned char id,
                                     unsigned long * bitlen,
                                     brsize_t numcodes, 
                                     unsigned long maxbitlen)
{
  HuffmanTree_t* tree = &_HuffmanTree(id);
  tree->lengths = bitlen;
  tree->numcodes = (unsigned)numcodes; /*number of symbols*/
  tree->maxbitlen = maxbitlen;
  _makeFromLengths2(pMng, pBuf, tree);  //生成tree->codes
  //生成table_len，table_value以在解码时调用HuffmanTree_makeTable
  //得到解码需要的查找表
  return _makeTable(pMng, pBuf,id);
}
//----------------------------更新动态哈夫曼结构-------------------------------
//原getTreeInflateDynamic, 更新前调用,返回非0有误
//此函数根据位流更新动态结构
signed short HuffmanTree_UpdateDync(struct _HuffmanTreeMng *pMng,//分配好内存再传入
                                     struct _HuffmanTreeBuf *pBuf,//分配好内存再传入
                                     bReader_t *reader)

{
 /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated*/
  unsigned error = 0;
  unsigned n, HLIT, HDIST, HCLEN, i;

  if(reader->bitsize - reader->bp < 14) return 49; /*error: the bit pointer is or will go past the memory*/ //
  ensureBits17(reader, 14); //取出14b: 5b长度+5b距离+4b码长HCLEN

  /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already*/
  HLIT =  readBits(reader, 5) + 257; //取出5bit长度
  /*number of distance codes. Unlike the spec, the value 1 is added to it here already*/
  HDIST = readBits(reader, 5) + 1;
  /*number of code length codes. Unlike the spec, the value 4 is added to it here already*/
  HCLEN = readBits(reader, 4) + 4;//HCLEN最大=2^4+4=20;

  _init(&_tree_cl);

  while(!error) {//建表
    /*read the code length codes out of 3 * (amount of code length codes) bits*/
    if(_gtofl(reader->bp, HCLEN * 3, reader->bitsize)) {
      _ERROR_BREAK(50); /*error: the bit pointer is or will go past the memory*/
    }
    unsigned long  *bitlen_cl = _bitlen_cl;
    for(i = 0; i != HCLEN; ++i) {//每次取3b缓冲至bitlen_cl，直到给定码长
      ensureBits9(reader, 3); /*out of bounds already checked above */
      bitlen_cl[CLCL_ORDER[i]] = readBits(reader, 3);
    }
    for(i = HCLEN; i != HUFFMAN_TREE_NUM_CODE_LENGTH_CODES; ++i) {//未使用部分清为0
      bitlen_cl[CLCL_ORDER[i]] = 0;
    }

    error = _makeFromLengths(pMng, pBuf,
                             _HUFFMAN_TREE_CL, bitlen_cl, 
                             HUFFMAN_TREE_NUM_CODE_LENGTH_CODES, 7); //得到查表值
    if(error) break;

    unsigned long *bitlen_ll = _bitlen_ll;
    unsigned long *bitlen_d = _bitlen_d;    
    memset(bitlen_ll, 0, HUFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS * sizeof(*bitlen_ll));
    memset(bitlen_d, 0, HUFFMAN_TREE_NUM_DISTANCE_SYMBOLS * sizeof(*bitlen_d));

    /*i is the current symbol we're reading in the part that contains the code lengths of lit/len and dist codes*/
    i = 0;
    while(i < HLIT + HDIST) {
      unsigned code;
      ensureBits25(reader, 22); /* up to 15 bits for huffman code, up to 7 extra bits below*/
      code = HuffmanTree_DecodeSymbol(pMng, reader, &_tree_cl);
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
      else{ /*if(code == HUFFMAN_TREE_INVALIDSYMBOL)*/
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
    error = _makeFromLengths(pMng, pBuf,HUFFMAN_TREE_LL, bitlen_ll, 
                             HUFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS, 15);
    if(error) break;
    error = _makeFromLengths(pMng, pBuf,HUFFMAN_TREE_D, bitlen_d, 
                             HUFFMAN_TREE_NUM_DISTANCE_SYMBOLS, 15);

    break; /*end of error-while*/
  }
  
  return 0;
}

//---------------------------------哈夫曼译码-----------------------------
//原HuffmanTree_DecodeSymbol
unsigned short HuffmanTree_DecodeSymbol(struct _HuffmanTreeMng *pMng,//已UpdateDync()
                                         bReader_t *reader, 
                                         const HuffmanTree_t* codetree)
{
  unsigned short code = peekBits(reader, HUFFMAN_TREE_FIRSTBITS);//从缓冲区读取对应掩码数据
  unsigned short l = codetree->table_len[code];//查到的，被压缩数据的长度
  unsigned short value = codetree->table_value[code];//查询到的值
  if(l <= HUFFMAN_TREE_FIRSTBITS) {
    advanceBits(reader, l); //位读取了，往前推进,即buffer右移l位
    return value;
  }
  else{
    advanceBits(reader, HUFFMAN_TREE_FIRSTBITS);
    value += peekBits(reader, l - HUFFMAN_TREE_FIRSTBITS);
    advanceBits(reader, codetree->table_len[value] - HUFFMAN_TREE_FIRSTBITS);
    return codetree->table_value[value];
  }
}
