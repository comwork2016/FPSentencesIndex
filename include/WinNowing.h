#ifndef WINNOWING_H
#define WINNOWING_H

#include <vector>
#include <stdlib.h>
#include "DataStrcture.h"


//to delete
#include <iostream>

class WinNowing
{
    public:
        WinNowing();
        virtual ~WinNowing();
        static std::vector<KGramHash> PickFingerPrints(const std::vector<SplitedHits>& vec_SplitedHits);
        static std::vector<KGramHash> CalcRabinHash(const std::vector<SplitedHits>& vec_SplitedHits);
    protected:
    private:
};

#endif // WINNOWING_H
