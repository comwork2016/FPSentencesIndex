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

        static std::vector<TFPair> SortTFMap(std::map<std::string, double>& map_SN);

    protected:
        static bool TFMapComp(const TFPair& x, const TFPair& y);
    private:
};

#endif // SORTUTIL_H
