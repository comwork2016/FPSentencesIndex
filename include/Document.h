#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

#include "SplitUtil.h"

#include "StringUtil.h"
#include "HashUtil.h"
#include "ReadCorpus.h"

// to delete
#include <string.h>

class Document
{
    public:
        Document(const std::string& str_DocPath,bool b_Split = false);
        virtual ~Document();
        std::string GetstrDocPath() const { return m_strDocPath; }
        std::string GetstrDocName() const { return m_strDocName; }
        std::string GetstrContents() const { return m_strContents; }
        std::vector<Paragraph> GetvecParagraph() const { return m_vecParagraph; }
        std::vector<KGramHash> GetKGramFingerPrints() const { return m_KGramFingerPrints; }
        SIMHASH_TYPE GetlSimHash() const { return m_lSimHash; }

        int ReadDocumentAndSplit();
        int ReadDocumentContent();
        void TFNormalization();
        void CalcDocSimHash();
        void PickStopTerm();
        void PickFingerPrints();
        void Dispaly();
    protected:
    private:
        std::string m_strDocPath;
        std::string m_strDocName;
        std::string m_strContents;
        std::vector<Paragraph> m_vecParagraph;
        std::vector<std::string> m_vecTitleTerm;
        int m_nWordCount; //文章中的词的总数
        std::map<std::string, double> m_MapTF;//文档词频信息
        SIMHASH_TYPE m_lSimHash;
        std::vector<KGramHash> m_KGramFingerPrints;
        std::set<std::string> m_setStopTerm;
};

#endif // DOCUMENT_H
