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
        Document(const std::string& str_DocPath,bool b_SplitToSentence = false, bool b_SplitToWords = false);
        virtual ~Document();
        std::string GetstrDocPath() const { return m_strDocPath; }
        std::string GetstrDocName() const { return m_strDocName; }
        std::string GetstrContents() const { return m_strContents; }

        void SetvecParagraph(std::vector<Paragraph> vec_Paragraph ) { m_vecParagraph = vec_Paragraph; }
        std::vector<Paragraph> GetvecParagraph() const { return m_vecParagraph; }

        std::vector<KGramHash> GetKGramFingerPrints() const { return m_KGramFingerPrints; }

        SIMHASH_TYPE GetlSimHash() const { return m_lSimHash; }

        SIMHASH_TYPE GetlSimHash16_1() const { return m_lSimHash16_1; }
        SIMHASH_TYPE GetlSimHash16_2() const { return m_lSimHash16_2; }
        SIMHASH_TYPE GetlSimHash16_3() const { return m_lSimHash16_3; }
        SIMHASH_TYPE GetlSimHash16_4() const { return m_lSimHash16_4; }

        void SetMapTF(std::map<std::string, double> map_TF) { m_MapTF = map_TF; }
        std::map<std::string, double> GetMapTF() { return m_MapTF; }

        void SetnWordCount(int n_WordCount) { m_nWordCount = n_WordCount; }
        int GetnWordCount() { m_nWordCount; }

        void SetvecTitleTerm(std::vector<std::string> vec_TitleTerm) { m_vecTitleTerm = vec_TitleTerm; }

        int ReadDocumentContent();
        int ReadDocumentAndSplitToSentence();
        void SplitSentenceToWords();
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
        std::set<std::string> m_setStopTerm;
        SIMHASH_TYPE m_lSimHash;
        SIMHASH_TYPE m_lSimHash16_1;//simhash0-15位
        SIMHASH_TYPE m_lSimHash16_2;//simhash16-31位
        SIMHASH_TYPE m_lSimHash16_3;//simhash32-47位
        SIMHASH_TYPE m_lSimHash16_4;//simhash48-63位
        std::vector<KGramHash> m_KGramFingerPrints;
};

#endif // DOCUMENT_H
