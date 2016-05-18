#include "Document.h"

/**
   文档构造函数
   处理文件路径，文件名读取文档内容。
*/
Document::Document(const std::string& str_DocPath)
{
    this->m_strDocPath = str_DocPath;
    int n_SeparatorIndex = str_DocPath.find_last_of("/");
    this->m_strDocName = str_DocPath.substr(n_SeparatorIndex+1);
    this->m_strContents = "";
    this->m_lSimHash = 0;
    this->m_MapTF.clear();
    this->m_vecParagraph.clear();
    //读取文档内容
    int n_ReadStats = ReadDocumentAndSplit();
    if(n_ReadStats == ERROR_OPENFILE)
    {
        std::cout<<"open file "<<this->m_strDocPath<<" error"<<std::endl;
        return;
    }
}

/**
    一次性读取全部的文档内容
*/
int Document::ReadDocumentContent()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    std::stringstream ss;
    ss<<ifs_Doc.rdbuf();
    this->m_strContents = ss.str();
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    读取文件内容
    程序中，将一行内容作为一个段落
    并用常见符号将段落分割成句子
*/
int Document::ReadDocumentAndSplit()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    int offset=0;
    SplitUtil* splitUtil = new SplitUtil();
    while(!ifs_Doc.eof())
    {
        Paragraph para;
        para.textRange.offset_begin = offset;
        //读入一行数据
        std::string str_Line;
        std::getline(ifs_Doc,str_Line);
        int n_LineLength = str_Line.length();
        offset+=n_LineLength;
        this->m_strContents.append(str_Line);
        para.textRange.offset_end = offset;
        // 不是文章结尾时将原文档的换行符加回去
        if(!ifs_Doc.eof())
        {
            this->m_strContents.append(1,'\n');
            offset++;
        }
        if(n_LineLength != 0 && !StringUtil::isStringBlank(str_Line)) //空白行不计为段落
        {
            //对段落进行句子的拆分
            std::string str = this->m_strContents.substr(para.textRange.offset_begin,para.textRange.offset_end - para.textRange.offset_begin);
            SplitUtil::SplitParaphToSentence(para,str);
            //对句子进行分词处理
            for(int j = 0; j<para.vec_Sentences.size(); j++)
            {
                Sentence& sen = para.vec_Sentences[j];
                int n_SenLen = sen.textRange.offset_end-sen.textRange.offset_begin;
                std::string str_sentence = this->m_strContents.substr(sen.textRange.offset_begin,n_SenLen);
                splitUtil->SplitTermAndCalcTF(sen,str_sentence,this->m_MapTF);
            }
            this->m_vecParagraph.push_back(para);
        }
    }
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    计算词频和全文的simhash
*/
void Document::CalcDocSimHash()
{
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = this->m_vecParagraph[i];
        //对句子计算simhash
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            HashUtil::CalcKRHash(sen);
            /*//遍历k-gram词组的hash值和文本范围
            std::cout<<sen.vec_KGramHash.size()<<std::endl;
            for(int i=0; i<sen.vec_KGramHash.size(); i++)
            {
                for(int j=0; j<sen.vec_KGramHash[i].vec_splitedHits.size(); j++)
                {
                    SplitedHits hits = sen.vec_KGramHash[i].vec_splitedHits[j];
                    std::cout<<hits.words<<" ";
                    //std::cout<<"["<<hits.hashValue<<"]"<<hits.words<<" ";
                }
                std::cout<<"["<<sen.vec_KGramHash[i].textRange.offset_begin<<","<<sen.vec_KGramHash[i].textRange.offset_end<<"]:::"<<sen.vec_KGramHash[i].hashValue<<std::endl;
            }*/
            sen.hashValue = HashUtil::CalcSenSimHash(sen.vec_KGramHash);
            //std::wcout<<sen.hashValue<<std::endl;
        }
        para.hashValue = HashUtil::CalcParaSimHash(para.vec_Sentences);
        //std::wcout<<para.hashValue<<std::endl;
        //std::wcout<<std::endl;
    }
    this->m_lSimHash = HashUtil::CalcDocSimHash(this->m_vecParagraph);
    /*
        //对map按值排序
        std::vector<TFPair> vec_TFPair = SortUtil::SortTFMap(map_TF);
        //遍历map
        for(std::vector<TFPair>::iterator it = vec_TFPair.begin(); it != vec_TFPair.end(); it++)
        {
            std::wcout<<StringUtil::ConvertCharArraytoWString(it->first) <<" : "<<it->second<<std::endl;
        }
    */
}

void Document::Dispaly()
{
    //输出文件的信息
    std::cout<<this->m_strDocName<<std::endl;
    /*
    //输出段落句子的信息
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = m_vecParagraph[i];
        //对句子进行分词并计算simhash
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            int n_SenLen = sen.textRange.offset_end-sen.textRange.offset_begin;
            std::string str_sentence = this->m_strContents.substr(sen.textRange.offset_begin,n_SenLen);
            std::cout<<"Para "<<i<<" Sentence "<<j<<":"<<std::endl;
            std::cout<<str_sentence<<std::endl<<std::endl;
        }
    }*/
    std::cout<<this->m_lSimHash<<std::endl;
}

Document::~Document()
{
    //dtor
}
