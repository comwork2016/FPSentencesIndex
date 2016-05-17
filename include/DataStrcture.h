#ifndef DATASTRCTURE_H_INCLUDED
#define DATASTRCTURE_H_INCLUDED

#include <string>
#include <vector>
#include "Constants.h"

typedef std::pair<std::wstring,int> SNPair;//存储词频信息

//定义结构体，用来存储分词之后的片段信息
struct SplitedHits
{
    std::wstring words;//宽字节存储词语
    int offset;//词语在文档中的偏移量
    int length;//词语所占的字节数
    SIMHASH_TYPE hashValue;
};

//定义结构体，文档某一段之间的内容
struct TextRange
{
    int offset_begin;
    int offset_end;
};

//定义结构体，用来存储k-gram组合信息
struct KGramHash
{
    SIMHASH_TYPE hashValue; //组合的hash值
    TextRange textRange; //KGram的组合范围
    std::vector<SplitedHits> vec_splitedHits; //组合包含的分词信息
};

//定义结构体，存储句子信息
struct Sentence
{
    TextRange textRange; //段落范围
    SIMHASH_TYPE hashValue;//句子的simhash值
    std::vector<SplitedHits> vec_splitedHits; //段落包含的分词信息
    std::vector<KGramHash> vec_KGramHash; //句子指纹信息
};

//定义结构体，存储段落信息
struct Paragraph
{
    TextRange textRange; //段落范围
    SIMHASH_TYPE hashValue;//段落simhash值
    std::vector<Sentence> vec_Sentences; //段落包含的分词信息
};

//文档之间的相似度和相同文本
struct FingerPrintsSimilarDocument
{
    std::string str_SearchDoc;//查询的文档
    std::string str_DBDoc;//数据库中的文档
    float f_similarity; //文档相似度
    std::vector<TextRange> vec_SearchDocSimilarTextRange;//待比对的文档中相同指纹向量
    std::vector<TextRange> vec_DBDocSimilarTextRange;//数据库的文档中相同指纹向量
};

#endif // DATASTRCTURE_H_INCLUDED
