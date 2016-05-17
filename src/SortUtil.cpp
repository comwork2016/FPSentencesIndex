#include "SortUtil.h"

SortUtil::SortUtil()
{
    //ctor
}

//对map<string, int>按值排序
std::vector<SNPair> SortUtil::SortSNMap(std::map<std::wstring, int>& map_SN)
{
    std::vector<SNPair> vec_SNPair;
    //遍历map
    for(std::map<std::wstring,int>::iterator it = map_SN.begin(); it != map_SN.end(); it++)
    {
    //    if(it->second>1 && it->first.length()>1) //词频大于1且词长大于1的词作为候选关键词
        {
            vec_SNPair.push_back(SNPair(it->first,it->second));
        }
    }
    sort(vec_SNPair.begin(),vec_SNPair.end(),SNMapComp);
    return vec_SNPair;
}

bool SortUtil::SNMapComp(const SNPair& x, const SNPair& y)
{
    return x.second < y.second;
}

SortUtil::~SortUtil()
{
    //dtor
}
