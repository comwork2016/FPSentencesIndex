#include "SplitUtil.h"

SplitUtil::SplitUtil()
{
    //friso初始化
    this->friso = friso_new();
    this->config = friso_new_config();
    static const fstring frisoIniPath = fstring("/home/andin/soft/friso-1.6.0/friso.ini");
    if ( friso_init_from_ifile(this->friso, this->config, frisoIniPath) != 1 )//初始化失败
    {
        std::cout<<"fail to initialize friso and config."<<std::endl;
    }
    this->task = friso_new_task();
}

/**
    将段落分割成句子
*/
void SplitUtil::SplitParaphToSentence(Paragraph& para,const std::string& str)
{
    //将文档用标点符号拆分
    std::vector<std::string> vec_pattern;
    vec_pattern.push_back("。");
    vec_pattern.push_back("？");
    vec_pattern.push_back("！");
    vec_pattern.push_back("；");

    const int strsize=str.size();
    std::string::size_type pos;
    int i = 0;
    while(i<strsize)
    {
        pos = strsize;
        int patternSize=0;
        //查找第一个分隔符的位置
        for(int j=0; j<vec_pattern.size(); j++)
        {
            std::string pattern = vec_pattern[j];
            int index =str.find(pattern,i);
            if(index!=std::string::npos && index < pos)
            {
                pos = index;
                patternSize = pattern.size();
            }
        }
        //std::cout<<i<<","<<pos<<","<<strsize<<std::endl;
        if(pos<=strsize)
        {
            std::string s=str.substr(i,pos-i);
            //std::cout<<s<<std::endl<<std::endl;
            if(!StringUtil::isStringBlank(s))
            {
                Sentence sen;
                sen.textRange.offset_begin = para.textRange.offset_begin + i;
                sen.textRange.offset_end = para.textRange.offset_begin + pos;
                para.vec_Sentences.push_back(sen);
            }
            i=pos+patternSize;
        }
    }
}

/**
    分隔字符串
*/
std::vector<std::string>  SplitUtil::SplitString(const std::string& str_Source,const std::string& str_pattern)
{
    std::vector<std::string> result;
    std::string str = str_Source + str_pattern;
    int strsize=str.size();
    int patternSize = str_pattern.size();
    std::string::size_type pos;
    for(int i=0; i<strsize; i++)
    {
        pos =str.find(str_pattern,i);
        if(pos!=std::string::npos && pos<strsize)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+patternSize-1;
        }
    }
    return result;
}

/**
    对句子分词并统计词频
*/
void SplitUtil::SplitTermAndCalcTF(Sentence& sen,const std::string& str,std::map<std::string,double>& map_TF,int& n_WordCount)
{
    int i;
    // 设置任务信息
    fstring text = fstring(str.c_str());
    friso_set_text( this->task, text);
    //循环读取分词内容
    while ( ( friso_next( this->friso, this->config, this->task ) ) != NULL )
    {
        std::string str_HitsWord = this->task->hits->word;
        int n_offset = task->hits->offset;
        int n_length = task->hits->length;
        //计算分词片段的hash值
        SIMHASH_TYPE hashValue = HashUtil::CalcStringHash(str_HitsWord);
        SplitedHits sh_hits =
        {
            str_HitsWord,
            sen.textRange.offset_begin + n_offset,
            n_length,
            hashValue
        };
        sen.vec_splitedHits.push_back(sh_hits);
        n_WordCount++;
        map_TF[str_HitsWord] += 1;
        //std::cout<<sh_hits.words<<"["<<sh_hits.offset<<","<<sh_hits.length<<","<<sh_hits.hashValue<<"]   ";
    }
    //std::wcout<<std::endl<<std::endl;
}

SplitUtil::~SplitUtil()
{
    //任务结束，销毁friso实例
    friso_free_task( this->task );
    friso_free_config(this->config);
    friso_free(this->friso);
}
