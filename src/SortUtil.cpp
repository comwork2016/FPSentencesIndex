#include "SortUtil.h"

SortUtil::SortUtil()
{
    //ctor
}

/**
    对map<string, double>按值排序
*/
std::vector<TFPair> SortUtil::SortTFMap(std::map<std::string, double>& map_TF)
{
    std::vector<TFPair> vec_TFPair;
    //遍历map
    for(std::map<std::string,double>::iterator it = map_TF.begin(); it != map_TF.end(); it++)
    {
        vec_TFPair.push_back(TFPair(it->first,it->second));
    }
    sort(vec_TFPair.begin(),vec_TFPair.end(),TFMapComp);
    return vec_TFPair;
}

bool SortUtil::TFMapComp(const TFPair& x, const TFPair& y)
{
    return x.second < y.second;
}

SortUtil::~SortUtil()
{
    //dtor
}
