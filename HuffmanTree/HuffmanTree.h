/*******************************************************************************

//		               哈夫曼树模块
此模块主要用于的哈夫曼解码(不能支持编码)，以解压压缩格式文件或图片
此模块修改出自"lodepng->HuffmanTree", 感谢作者！！！！！
此模块为重新整理以学习分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统做了优化,固定哈夫曼改为常量，并维护两动态结构且不需分配内存

此模块支持多线程调用！
*******************************************************************************/
#ifndef _HUFFMAN_TREE_H
#define _HUFFMAN_TREE_H


/*******************************************************************************
                             宏定义
*******************************************************************************/
/* amount of bits for first huffman table lookup (aka root bits), 
   see HuffmanTree_makeTable and HuffmanTree_DecodeSymbol.*/
/* values 8u and 9u work the fastest */
#define HUFFMAN_TREE_FIRSTBITS     9u

#define HUFFMAN_TREE_INVALIDSYMBOL 65535u

#define HUFFMAN_TREE_FIRST_LENGTH_CODE_INDEX   257
#define HUFFMAN_TREE_LAST_LENGTH_CODE_INDEX    285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define HUFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS   288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define HUFFMAN_TREE_NUM_DISTANCE_SYMBOLS       32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define HUFFMAN_TREE_NUM_CODE_LENGTH_CODES      19


//动态距离哈夫曼结构
#define HUFFMAN_TREE_LL  0  
//动态值与长度哈夫曼结构
#define HUFFMAN_TREE_D   1  

/*******************************************************************************
                             相关结构
*******************************************************************************/


//--------------------------哈夫曼树主结构-----------------------------------
typedef struct _HuffmanTree{
  //计算查找表需要的数据(不支持编码)
  unsigned long *codes; //huffman codes, 大小：numcodes
  unsigned long *lengths; //huffman codes lengths,大小：numcodes
  unsigned short maxbitlen; //single code maximum number
  unsigned short numcodes; //number of codes
  
  //由上述生成的查找表，解码时直接查表即可
  unsigned char *table_len;//值的长度查找表,大小512
  unsigned short *table_value; //值查找表,大小512
}HuffmanTree_t;

extern const struct _HuffmanTree HuffmanTree_FixD;//计算好的固定距离哈夫曼结构
extern const struct _HuffmanTree HuffmanTree_FixLL;//计算好的固定值与长度哈夫曼结构

//--------------------------动态哈夫曼管理器-----------------------------------
//在HuffmanTree_UpdateDync()中使用，哈夫曼译码结束可释放
struct _HuffmanTreeMng{
  //缓冲的动态哈夫曼结构
  struct _HuffmanTree HuffmanTree[2]; 
  //最后需要的：动态哈夫曼结构长度查找表, _makeTable()中产生
  unsigned char table_len[2][1u << HUFFMAN_TREE_FIRSTBITS]; 
  //最后需要的：动态哈夫曼结构值查找表, _makeTable()中产生
  unsigned short table_value[2][1u << HUFFMAN_TREE_FIRSTBITS]; 
};

//---------------------------动态哈夫曼内部内存缓冲器---------------------------
//在HuffmanTree_UpdateDync()中使用，退出即可释放
struct _HuffmanTreeBuf{
  struct{//HuffmanTree_UpdateDync()中使用的缓冲，需保持至退出
    //HuffmanTree_UpdateDync中产生,makeTable()需要
    unsigned long bitlen_ll[HUFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS];
    unsigned long bitlen_d[HUFFMAN_TREE_NUM_DISTANCE_SYMBOLS];
    //_makeFromLengths2()或makeTable()中传递
    unsigned long codes[HUFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS];
  }m; 
  
  union {//二级函数使用的缓冲
    struct{//_makeFromLengths2()中使用, 用于生成_codes
      unsigned long blcount[16];
      unsigned long nextcode[16];
    }makecodes;
    struct{//makeTable()中使用：
      unsigned long maxlens[1u << HUFFMAN_TREE_FIRSTBITS];
    }makeTables;
  }u;
};

/*******************************************************************************
                             相关函数
*******************************************************************************/

#include "bReader.h"

//----------------------------得到哈夫曼结构-------------------------------
//仅供解码使用
//得到固定距离哈夫曼结构
#define HuffmanTree_pGetFixD()  &HuffmanTree_FixD;
//得到固定值与长度哈夫曼结构
#define HuffmanTree_pGetFixLL()  &HuffmanTree_FixLL;

//----------------------------更新动态哈夫曼结构-------------------------------
//原getTreeInflateDynamic, 更新前调用,返回非0有误
//此函数根据位流更新动态结构
signed short HuffmanTree_UpdateDync(struct _HuffmanTreeMng *pMng,//分配好内存再传入
                                     struct _HuffmanTreeBuf *pBuf,//分配好内存再传入
                                     bReader_t *reader);

//------------------------------------哈夫曼译码------------------------------
//原huffmanDecodeSymbol
unsigned short HuffmanTree_DecodeSymbol(struct _HuffmanTreeMng *pMng,//已UpdateDync()
                                         bReader_t *reader, 
                                         const HuffmanTree_t* codetree);

#endif //_HUFFMAN_TREE_H


