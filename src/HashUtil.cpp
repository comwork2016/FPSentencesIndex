#include "HashUtil.h"

HashUtil::HashUtil()
{
    //ctor
}

/**
    计算文本的hash值
*/
SIMHASH_TYPE HashUtil::CalcStringHash(const std::string& str)
{
    SIMHASH_TYPE l_Hash = 0;
    //判断是否有近义词
    std::string str_Delim = "|";
    if(str.find(str_Delim)!=std::string::npos) // 没有近义词
    {
        std::vector<std::string> vec_strSplited = SplitUtil::SplitString(str,str_Delim);
        for(int i=0; i<vec_strSplited.size(); i++)
        {
            std::string stri = vec_strSplited[i];
            SIMHASH_TYPE l_HashI = CalcStringHash(stri);
            l_Hash += l_HashI;
        }
    }
    else
    {
        std::wstring wstr = StringUtil::ConvertCharArraytoWString(str);
        for(int i=0; i<wstr.length(); i--)
        {
            wchar_t c = wstr.at(i);
            l_Hash = ( BASE * l_Hash +  int(c)*BASE) % MODNUM;
        }
    }
    return l_Hash;
}

/**
    通过分词块计算KGram的KRHash
*/
void HashUtil::CalcKRHash(Sentence& sen)
{
    if(sen.vec_splitedHits.size()<KGRAM)//如果分词数少于阈值，则不作为一个特征项
    {
        return;
    }
    //初始化kgramhash
    int n_kcount = 0;
    KGramHash kgram_Now;
    kgram_Now.hashValue = 0;
    kgram_Now.vec_splitedHits.clear();
    KGramHash kgram_Last;
    kgram_Last.hashValue = 0;
    kgram_Last.vec_splitedHits.clear();
    SIMHASH_TYPE l_SimHash = 0;
    //遍历分词列表
    int i=0;
    for(i=0; i<sen.vec_splitedHits.size(); i++)
    {
        SplitedHits hits = sen.vec_splitedHits[i];//词语单位
        //H(c 1 . . . ck ) = c1 ∗ b^k + c2 ∗ b^k−1 ∗ . . . + ck−1 ∗ b^2 + ck*b
        if(n_kcount<KGRAM)
        {
            kgram_Now.vec_splitedHits.push_back(hits);
            kgram_Last.vec_splitedHits.push_back(hits);
            //(a + b)%M = (a%M + b%M)%M    (ab)%M = [(a%M)(b%M)]%M
            kgram_Now.hashValue = ( kgram_Now.hashValue * BASE + hits.hashValue * BASE) % MODNUM;
            n_kcount++;
        }
        else
        {
            // 计算偏移信息并保存kgram的信息
            kgram_Now.textRange.offset_begin = kgram_Now.vec_splitedHits[0].offset;
            kgram_Now.textRange.offset_end = kgram_Now.vec_splitedHits[KGRAM-1].offset + kgram_Now.vec_splitedHits[KGRAM-1].length;
            sen.vec_KGramHash.push_back(kgram_Now);
            // 更新kgram_now and kgram_now，即删除kgram_now中的第一个元素，添加下一个元素，同时，第一个分词索引下移
            std::vector<SplitedHits>::iterator it_first = kgram_Now.vec_splitedHits.begin();
            kgram_Now.vec_splitedHits.erase(it_first);
            kgram_Now.vec_splitedHits.push_back(hits);
            //为防止数据溢出，需要对运算进行等价处理
            SIMHASH_TYPE l_LastCharWeight=kgram_Last.vec_splitedHits[0].hashValue;
            //the value of l_LastCharWeight would be "kgram_Last.vec_splitedHits[0].hashValue * pow(BASE, K) % MODNUM"
            for(int i = 0; i < KGRAM; i++)
            {
                l_LastCharWeight = ( l_LastCharWeight * BASE ) % MODNUM;
            }
            kgram_Now.hashValue = ((kgram_Now.hashValue - l_LastCharWeight + hits.hashValue)*BASE)%MODNUM;
            if(kgram_Now.hashValue<0)
            {
                kgram_Now.hashValue+=MODNUM;
            }
            kgram_Last = kgram_Now;
        }
    }
    sen.vec_KGramHash.push_back(kgram_Now);
}

/**
    计算海明距离
*/
bool HashUtil::IsSimHashSimilar(const SIMHASH_TYPE& l_num1, const SIMHASH_TYPE& l_num2)
{
    int hd = 0;
    SIMHASH_TYPE x = l_num1^l_num2;
    while (x && hd<=HAMMINGDIST)
    {
        hd += 1;
        x = x&(x-1);//减一之后二进制的数字里面会减少一个1
    }
    if(hd<=HAMMINGDIST)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
    计算文档分词后的simhash值
*/
template <typename T>
SIMHASH_TYPE HashUtil::CalcSimHash(const std::vector<T>& vec_SimHash)
{
    //初始化表示simhash每一位的权重数组
    short v[SIMHASHBITS-1];
    for(int i=0; i<SIMHASHBITS-1; i++)
    {
        v[i] = 0;
    }
    //遍历hash值列表
    for(int i=0;i<vec_SimHash.size();i++)
    {
        SIMHASH_TYPE l_Hash = vec_SimHash[i].hashValue;
        //计算对hash值的每一位，如果为1，则权重数组的相应位+1，为0则-1
        for (int j = 0; j < SIMHASHBITS - 1; j++)
        {
            SIMHASH_TYPE bitmask = 1 << j; //位的掩码:向左移j位
            SIMHASH_TYPE bit = l_Hash&bitmask;
            if (bit != 0)
            {
                v[j] += 1;
            }
            else
            {
                v[j] -= 1;
            }
        }
    }
    //根据权重数组计算文档的simhash值
    SIMHASH_TYPE l_SimHash = 0;
    for (int i = 0; i < SIMHASHBITS-1; i++)
    {
        if (v[i] > 0)
        {
            SIMHASH_TYPE n_IBit = (SIMHASH_TYPE)1<<(i);
            l_SimHash += n_IBit;
        }
    }
    return l_SimHash;
}

SIMHASH_TYPE HashUtil::CalcSenSimHash(const std::vector<KGramHash>& vec_SimHash)
{
    return CalcSimHash(vec_SimHash);
}

SIMHASH_TYPE HashUtil::CalcParaSimHash(const std::vector<Sentence>& vec_SimHash)
{
    return CalcSimHash(vec_SimHash);
}

SIMHASH_TYPE HashUtil::CalcDocSimHash(const std::vector<Paragraph>& vec_SimHash)
{
    return CalcSimHash(vec_SimHash);
}

HashUtil::~HashUtil()
{
    //dtor
}
