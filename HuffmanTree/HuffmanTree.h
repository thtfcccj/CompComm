/*******************************************************************************

//		               哈夫曼树模块
此模块主要用于的哈夫曼解码(不能支持编码)，以解压压缩格式文件或图片
此模块修改出自"lodepng->HuffmanTree", 感谢作者！！！！！
此模块为重新整理以学习分析代码以为加深印像，并符合现编码规则，以方便使用
此模块针对嵌入式系统做了优化,固定哈夫曼改为常量，并维护两动态结构且不需分配内存

此模块不支持多线程调用！
*******************************************************************************/
#ifndef _HAFFMAN_TREE_H
#define _HAFFMAN_TREE_H

/*******************************************************************************
                             相关结构
*******************************************************************************/

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


#define HAFFMAN_TREE_LL  0  //动态距离哈夫曼结构
#define HAFFMAN_TREE_D   1  //动态值与长度哈夫曼结构
extern struct _HuffmanTree HuffmanTree[2]; //动态哈夫曼结构

/*******************************************************************************
                             相关函数
*******************************************************************************/

#include "bReader.h"

//----------------------------更新动态哈夫曼结构-------------------------------
//原getTreeInflateDynamic, 更新前调用,返回非0有误
//此函数根据位流更新动态结构
signed short HuffmanTree_UpdateDync(bReader_t *reader);

//---------------------------------哈夫曼译码-----------------------------
//原huffmanDecodeSymbol
unsigned short HuffmanTree_DecodeSymbol(bReader_t *reader, 
                                         const HuffmanTree_t* codetree);

#endif //_HAFFMAN_TREE_H


