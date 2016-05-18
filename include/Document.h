#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "SplitUtil.h"

#include "StringUtil.h"
#include "SortUtil.h"
#include "HashUtil.h"

// to delete
#include <string.h>

class Document
{
    public:
        Document(const std::string& str_DocPath);
        virtual ~Document();
        std::string GetstrDocPath() const { return m_strDocPath; }
        std::string GetstrDocName() const { return m_strDocName; }
        std::string GetstrContents() const { return m_strContents; }
        std::vector<Paragraph> GetvecParagraph() const { return m_vecParagraph; }
        SIMHASH_TYPE GetlSimHash() const { return m_lSimHash; }

        int ReadDocumentAndSplit();
        int ReadDocumentContent();
        void CalcDocSimHash();
        void Dispaly();
    protected:
    private:
        std::string m_strDocPath;
        std::string m_strDocName;
        std::string m_strContents;
        std::vector<Paragraph> m_vecParagraph;
        std::map<std::string, double> m_MapTF;//文档词频信息
        SIMHASH_TYPE m_lSimHash;
};

#endif // DOCUMENT_H
