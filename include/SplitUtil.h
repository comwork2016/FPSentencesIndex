#ifndef SPLITUTIL_H
#define SPLITUTIL_H

#include <iostream>
#include <string>
#include <map>

#include "DataStrcture.h"
#include "StringUtil.h"
#include "HashUtil.h"
extern "C"
{
    #include "friso/friso.h"
    #include "friso/friso_API.h"
}

class SplitUtil
{
    public:
        SplitUtil();
        virtual ~SplitUtil();
        void SplitTermAndCalcTF(Sentence& sen,const std::string& str,std::map<std::string,double>& map_TF, int& n_WordCount);

        static void SplitParaphToSentence(Paragraph& para,const std::string& str);
        static std::vector<std::string> SplitString(const std::string& str_Source,const std::string& str_pattern);
    protected:
    private:
        //friso中文分词配置文件地址
        friso_t friso; // friso实例
        friso_config_t config; //friso配置类
        friso_task_t task; //分词任务类
};

#endif // SPLITUTIL_H
