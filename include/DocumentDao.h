#ifndef DOCUMENTDAO_H
#define DOCUMENTDAO_H

#include "mongo/client/dbclient.h"
#include "Document.h"
#include "Constants.h"
#include <vector>

class DocumentDao
{
    public:
        DocumentDao();
        virtual ~DocumentDao();

        int InsertDocument(const Document* doc);
        int DeleteAll();
        std::string QuerySIMSimilarity(const Document* doc);
        std::vector<FingerPrintsSimilarDocument> GetFingerPrintsSimilarDocument(Document* doc);
    protected:
        void ExtendMatch(const Document* doc, const Document *docDB,std::vector<TextRange>& vec_SearchDocSimilarTextRange,std::vector<TextRange>& vec_DBDocSimilarTextRange,int& n_SameContentsBytes);
    private:
        mongo::DBClientConnection m_Conn;
        std::string m_Host;
        std::string m_DBName;
};

#endif //DOCUMENTDAO_H
