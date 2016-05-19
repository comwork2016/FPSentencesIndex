#ifndef HASHUTIL_H
#define HASHUTIL_H

#include <string>
#include <vector>

#include "Constants.h"
#include "StringUtil.h"
#include "DataStrcture.h"
#include "SplitUtil.h"

// to delete
#include <iostream>

class HashUtil
{
    public:
        HashUtil();
        virtual ~HashUtil();

        static bool IsSimHashSimilar(const SIMHASH_TYPE& l_num1, const SIMHASH_TYPE& l_num2);
        static SIMHASH_TYPE CalcStringHash(const std::string& str);
        static SIMHASH_TYPE CalcDocSimHash(const std::vector<KGramHash>& vec_SimHash);

        static std::vector<KGramHash> GetKGramAndCalcKRHash(const Sentence& sen);
    protected:
        template <typename T>
        static SIMHASH_TYPE CalcSimHash(const std::vector<T>& vec_SimHash);
    private:
};

#endif // HASHUTIL_H
