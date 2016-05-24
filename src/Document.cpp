#include "Document.h"

/**
   文档构造函数
   处理文件路径，文件名读取文档内容。
*/
Document::Document(const std::string& str_DocPath,bool b_SplitToSentence,bool b_SplitToWords)
{
    this->m_strDocPath = str_DocPath;
    int n_SeparatorIndex = str_DocPath.find_last_of("/");
    this->m_strDocName = str_DocPath.substr(n_SeparatorIndex+1);
    this->m_strContents = "";
    this->m_lSimHash = 0;
    this->m_nWordCount = 0;
    if(b_SplitToSentence)
    {
        //读取文档内容
        int n_ReadStats = ReadDocumentAndSplitToSentence();
        if(n_ReadStats == ERROR_OPENFILE)
        {
            std::cout<<"open file "<<this->m_strDocPath<<" error"<<std::endl;
            return;
        }
        if(b_SplitToWords)
        {
            SplitSentenceToWords();
            CalcDocSimHash();
        }
    }
    else
    {
        ReadDocumentContent();
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
int Document::ReadDocumentAndSplitToSentence()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    int offset=0;
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
            this->m_vecParagraph.push_back(para);
        }
    }
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    将句子分词处理
*/
void Document::SplitSentenceToWords()
{
    SplitUtil* splitUtil = new SplitUtil();
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = this->m_vecParagraph[i];
        //对句子进行分词处理
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            int n_SenLen = sen.textRange.offset_end-sen.textRange.offset_begin;
            std::string str_sentence = this->m_strContents.substr(sen.textRange.offset_begin,n_SenLen);
            splitUtil->SplitTermAndCalcTF(sen,str_sentence,this->m_MapTF,this->m_nWordCount);
            //提取文章标题中的词语
            if(i==0 && j==0) //如果是第一段的第一句话，则为文章的标题
            {
                for(int k =0; k<sen.vec_splitedHits.size(); k++)
                {
                    this->m_vecTitleTerm.push_back(sen.vec_splitedHits[k].words);
                }
            }
        }
    }
    delete splitUtil;
}

/**
    计算全文的simhash
*/
void Document::CalcDocSimHash()
{
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = this->m_vecParagraph[i];
        //将句子组合成一个特征项，并添加到文档指纹集合中
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            std::vector<KGramHash> vec_KGramHash =  HashUtil::GetKGramAndCalcKRHash(sen);
            this->m_KGramFingerPrints.insert(this->m_KGramFingerPrints.end(),vec_KGramHash.begin(),vec_KGramHash.end());
        }
    }
    this->m_lSimHash = HashUtil::CalcDocSimHash(this->m_KGramFingerPrints);
}

/**
    词频标准化
*/
void Document::TFNormalization()
{
    //对在文章标题中出现的词语，加大词语的权重，作为5个平常词语出现
    for(int i=0; i<this->m_vecTitleTerm.size(); i++)
    {
        std::string str_term = this->m_vecTitleTerm[i];
        this->m_MapTF[str_term] += 4;
    }
    for(std::map<std::string,double>::iterator it = this->m_MapTF.begin(); it != this->m_MapTF.end(); it++)
    {
        std::string str_term = it->first;
        double d_TF = it->second / this->m_nWordCount;
        this->m_MapTF[str_term] = d_TF;
    }
}

/**
    挑选停用词
*/
void Document::PickStopTerm()
{
    //对文档中出现的每个词语，计算逆文档频率，当频率小于阈值时，加入到停用词集合中
    for(std::map<std::string,double>::iterator it = this->m_MapTF.begin(); it != this->m_MapTF.end(); it++)
    {
        std::string str_term = it->first;
        double d_tf = it->second;
        double d_idf = ReadCorpus::map_CorpusTF[str_term];
        if(d_idf !=0)//语料库中存在该词条
        {
            double d_tf_idf = d_tf / d_idf;
            //std::cout<<str_term<<":"<<d_tf_idf<<std::endl;
            if(d_tf_idf > 1)//当词语的逆文档频率小于阈值时，加入停用词集合
            {
                this->m_setStopTerm.insert(str_term);
            }
        }
    }
}

/**
    挑选文档指纹，如果特征项的前缀逆文档频率没有超过阈值时，删除该特征项
*/
void Document::PickFingerPrints()
{
    TFNormalization();
    PickStopTerm();//挑选停用词
    for(std::vector<KGramHash>::iterator it = this->m_KGramFingerPrints.begin(); it != this->m_KGramFingerPrints.end(); it++)
    {
        KGramHash kgram = *it;
        //删除前缀为停用词语的指纹，并且删除相似度较高的指纹
        if(!kgram.b_Last)
        {
            std::string str_prefix = kgram.vec_splitedHits[0].words;
            const int n_wcharBit = sizeof(wchar_t) - 1; //一个宽字节字符所占的bit数
            //前缀为一个字（最后一个指纹除外）或者为停用词，则删除
            if( kgram.vec_splitedHits[0].length == n_wcharBit || this->m_setStopTerm.find(str_prefix) != this->m_setStopTerm.end())
            {
                this->m_KGramFingerPrints.erase(it);
                it--;
                continue;
            }
            else if(it != this->m_KGramFingerPrints.begin())//对比上一个指纹，如果vsm相似度高达（KGRAME-2）/KGRAME，则删除
            {
                std::vector<KGramHash>::iterator it_Last = it-1;
                std::string str_prefix_last1 = (*it_Last).vec_splitedHits[1].words;
                if(str_prefix == str_prefix_last1)
                {
                    this->m_KGramFingerPrints.erase(it);
                    it--;
                    continue;
                }
            }
        }
        //对保留下来的指纹，查看KGRAM中停用词的比例
        int n_StopTerm = 0;
        bool b_kept = true;
        for(int ik = 0; ik <KGRAM; ik++)
        {
            std::string str_term = kgram.vec_splitedHits[ik].words;
            if( this->m_setStopTerm.find(str_term) != this->m_setStopTerm.end())
            {
                n_StopTerm++;
                if(n_StopTerm >= STFGATE)//停用词超出比例范围时，删除该指纹
                {
                    this->m_KGramFingerPrints.erase(it);
                    it--;
                    b_kept = false;
                    break;
                }
                if(ik - n_StopTerm > KGRAM - STFGATE)//没有超过比例时，不再比较
                {
                    break;
                }
            }
        }
        if(b_kept && kgram.b_Last) //如果最后一个指纹保留了，则删除上一个vsm相似度高的指纹
        {
            if(it != this->m_KGramFingerPrints.begin())
            {
                std::vector<KGramHash>::iterator it_Last = it-1;
                std::string str_prefix = kgram.vec_splitedHits[0].words;
                std::string str_prefix_last = (*it_Last).vec_splitedHits[1].words;
                if(str_prefix == str_prefix_last)
                {
                    this->m_KGramFingerPrints.erase(it_Last);
                    it--;
                    continue;
                }
            }
        }
    }
}

void Document::Dispaly()
{
    //输出文件的信息
    std::cout<<this->m_strDocName<<std::endl;
    /*
    //输出文章内容
    std::cout<<this->m_strContents<<std::endl;
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
            std::cout<<"Para "<<i<<" Sentence "<<j<<":["<<sen.textRange.offset_begin<<","<<sen.textRange.offset_end<<"]"<<std::endl;
            std::cout<<str_sentence<<std::endl<<std::endl;
        }
    }*/

    /*//遍历k-gram词组的hash值和文本范围
    for(int i=0; i<this->m_KGramFingerPrints.size(); i++)
    {
        std::cout<<this->m_KGramFingerPrints[i].hashValue<<"\t";
        for(int j=0; j<this->m_KGramFingerPrints[i].vec_splitedHits.size(); j++)
        {
            SplitedHits hits = this->m_KGramFingerPrints[i].vec_splitedHits[j];
            std::cout<<hits.words<<"\t";
            //std::cout<<"["<<hits.hashValue<<"]"<<hits.words<<"\t";
        }
        //std::cout<<"["<<this->m_KGramFingerPrints[i].textRange.offset_begin<<","<<this->m_KGramFingerPrints[i].textRange.offset_end<<"]"<<std::endl;
        std::cout<<std::endl;
    }*/
    std::cout<<this->m_KGramFingerPrints.size()<<std::endl;
    std::cout<<this->m_lSimHash<<std::endl;
}

Document::~Document()
{
    //dtor
    //释放文档信息使用的内存
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph& para = m_vecParagraph[i];
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            sen.vec_splitedHits.clear();
        }
        para.vec_Sentences.clear();
    }
    this->m_vecParagraph.clear();
    this->m_vecTitleTerm.clear();
    this->m_MapTF.clear();
    this->m_KGramFingerPrints.clear();
    this->m_setStopTerm.clear();
}
