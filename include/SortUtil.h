#ifndef SORTUTIL_H
#define SORTUTIL_H

#include <vector>
#include <map>
#include <algorithm>
#include "DataStrcture.h"

class SortUtil
{
    public:
        SortUtil();
        virtual ~SortUtil();

        static std::vector<SNPair> SortSNMap(std::map<std::wstring, int>& map_SN);

    protected:
        static bool SNMapComp(const SNPair& x, const SNPair& y);
    private:
};

#endif // SORTUTIL_H
