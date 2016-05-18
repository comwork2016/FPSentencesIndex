#ifndef READCORPUS_H
#define READCORPUS_H

#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "StringUtil.h"
#include "DataStrcture.h"

class ReadCorpus
{
    public:
        static std::map<std::string,double> map_CorpusTF;

        ReadCorpus();
        virtual ~ReadCorpus();
        static void ReadCorpusTF(const std::string& str_CorpusPath);
    protected:
    private:
};

#endif // READCORPUS_H
