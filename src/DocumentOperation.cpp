#include "DocumentOperation.h"

DocumentOperation::DocumentOperation()
{
    //ctor
}

/**
    向数据库中添加一个文档信息
*/
int DocumentOperation::AddDocument(const std::string& str_DocPath)
{
    //通过文件路径读取文件内容，并进行分词处理，计算simhash值。
    Document* doc = new Document(str_DocPath,true,true);
    DocumentDao* docDao = new DocumentDao();
    //与数据库中的文件SimHash比较,如果不相同,计算文档指纹并存入数据库中
    const std::string str_SimilarDoc = docDao->QuerySIMSimilarity(doc);
    if(str_SimilarDoc=="")
    {
        //挑选指纹信息并存入
        doc->PickFingerPrints();
        docDao->InsertDocument(doc);
        std::cout<<doc->GetstrDocName() <<" inserted"<<std::endl;
    }
    else
    {
        std::cout<<"DUPLICATE DOC TO INSERT: "<<doc->GetstrDocName()<<" is similar to "<<str_SimilarDoc<<std::endl;
    }
    delete docDao;
    delete doc;
    return 0;
}

/**
    插入一个文档集合到数据库中
*/
int DocumentOperation::InsertDocuments(const std::vector<Document*> vec_Doc)
{
    std::ofstream ofs_Log;
    ofs_Log.open("./test/log.txt",std::ios_base::out);

    DocumentDao* docDao = new DocumentDao();
    for(int i=0; i<vec_Doc.size(); i++)
    {
        Document* doc = vec_Doc[i];
        const std::string str_SimilarDoc = docDao->QuerySIMSimilarity(doc);
        if(str_SimilarDoc=="")
        {
            //挑选指纹信息并存入
            doc->PickFingerPrints();
            docDao->InsertDocument(doc);
            std::cout<<doc->GetstrDocName() <<" inserted"<<std::endl;
            ofs_Log<<doc->GetstrDocName() <<" inserted"<<std::endl;
        }
        else
        {
            std::cout<<"DUPLICATE DOC TO INSERT: "<<doc->GetstrDocName()<<" is similar to "<<str_SimilarDoc<<std::endl;
            ofs_Log<<"DUPLICATE DOC TO INSERT: "<<doc->GetstrDocName()<<" is similar to "<<str_SimilarDoc<<std::endl;
        }
    }
    ofs_Log.close();
    delete docDao;
}

//将整个目录中的文件添加到数据库中
int DocumentOperation::AddDirectoryDocuments(const std::string& str_InputDir)
{
    DocumentDao* docDao = new DocumentDao();
    docDao->DeleteAll();
    delete docDao;

    std::vector<Document*> vec_Documents;
    //读取目录下所有的文件
    struct dirent *ptr;
    DIR *dir;
    dir=opendir((char *)str_InputDir.c_str());
    if(!dir)
    {
        std::cout<<"read input dir error"<<std::endl;
        return 1;
    }
    while((ptr=readdir(dir))!=NULL)
    {
        //跳过'.'和'..'两个目录
        if(ptr->d_name[0] == '.' || ptr->d_name[strlen(ptr->d_name)-1] == '~')
        {
            continue;
        }
        std::string str_DocPath = str_InputDir + ptr->d_name;
        Document* doc = new Document(str_DocPath,true);//分句但不分词
        vec_Documents.push_back(doc);
    }
    closedir(dir);
    //对所有文档进行分词处理，并计算simhash，挑选指纹
    SplitUtil* splitUtil = new SplitUtil();
    for(int i=0; i<vec_Documents.size(); i++)
    {
        Document* doc = vec_Documents[i];
        std::vector<Paragraph> vec_Paragraph;
        for(int i=0; i<doc->GetvecParagraph().size(); i++)
        {
            Paragraph para = doc->GetvecParagraph()[i];
            //对句子进行分词处理
            for(int j = 0; j<para.vec_Sentences.size(); j++)
            {
                Sentence& sen = para.vec_Sentences[j];
                int n_SenLen = sen.textRange.offset_end-sen.textRange.offset_begin;
                std::string str_sentence = doc->GetstrContents().substr(sen.textRange.offset_begin,n_SenLen);
                int n_WordCount = 0; //文章中的词的总数
                std::map<std::string, double> map_TF = doc->GetMapTF();//文档词频信息
                splitUtil->SplitTermAndCalcTF(sen,str_sentence,map_TF, n_WordCount);
                doc->SetMapTF(map_TF);
                doc->SetnWordCount(doc->GetnWordCount() + n_WordCount);
                //std::cout<<doc->GetstrDocName()<<","<<doc->GetnWordCount()<<","<<n_WordCount<<std::endl;
                //std::cin.get();
                //提取文章标题中的词语
                if(i==0 && j==0) //如果是第一段的第一句话，则为文章的标题
                {
                    std::vector<std::string> vec_TitleTerm;
                    for(int k =0; k<sen.vec_splitedHits.size(); k++)
                    {
                        vec_TitleTerm.push_back(sen.vec_splitedHits[k].words);
                    }
                    doc->SetvecTitleTerm(vec_TitleTerm);
                }
            }
            vec_Paragraph.push_back(para);
        }
        doc->SetvecParagraph(vec_Paragraph);
        doc->CalcDocSimHash();
    }
    delete splitUtil;
    //将文档集合添加到数据库中
    InsertDocuments(vec_Documents);
    //释放所有文档资源
    for(int i=0; i<vec_Documents.size(); i++)
    {
        delete vec_Documents[i];
    }
    return 0;
}

int DocumentOperation::SearchLeak(const std::string& str_DocPath)
{
    Document* doc = new Document(str_DocPath,true,true);
    DocumentDao* docDao = new DocumentDao();
    //与数据库中的文件SimHash比较,如果不相同,再通过文档指纹查询泄露信息
    std::string str_SimilarDoc = docDao->QuerySIMSimilarity(doc);
    if(str_SimilarDoc=="")
    {
        //查询相同的指纹
        std::vector<FingerPrintsSimilarDocument> vec_SimilarDocument = docDao->GetFingerPrintsSimilarDocument(doc);
        /*遍历输出相同指纹*/
        for(int i=0; i<vec_SimilarDocument.size(); i++)
        {
            FingerPrintsSimilarDocument similarDoc = vec_SimilarDocument[i];
            const char* pch_DocPath = doc->GetstrDocPath().c_str();
            const char* pch_SimDocPath = similarDoc.str_DBDoc.c_str();
            std::cout<<std::endl<<std::endl<<"*************************************************************************************************"<<std::endl;
            std::cout<<"similarity between "<<pch_DocPath<<" and "<<pch_SimDocPath<<" is "<<100*similarDoc.f_similarity<<"%"<<std::endl;
            std::cout<<"*************************************************************************************************"<<std::endl;
            int n_SameSize = similarDoc.vec_SearchDocSimilarTextRange.size();
            for(int j=0; j<n_SameSize; j++)
            {
                std::cout<<"==============================="<<std::endl;
                TextRange textrange_SearchDoc = similarDoc.vec_SearchDocSimilarTextRange[j]; //待比对文档中相同的hash
                TextRange textrange_DBDoc = similarDoc.vec_DBDocSimilarTextRange[j]; //数据库中文档的相同的hash;
                // 搜索文档的内容和位置
                int n_OriginLength = textrange_SearchDoc.offset_end - textrange_SearchDoc.offset_begin;
                std::string str_OriginWord = doc->GetstrContents().substr(textrange_SearchDoc.offset_begin, n_OriginLength);
                std::cout<<"["<<textrange_SearchDoc.offset_begin<<","<<textrange_SearchDoc.offset_end<<","<<n_OriginLength<<"]"<<std::endl;
                std::cout<<str_OriginWord<<std::endl<<std::endl;
                //数据库中文档的内容和位置
                Document* docDB = new Document(similarDoc.str_DBDoc);// 数据库中的文档信息
                int n_DBLength = textrange_DBDoc.offset_end - textrange_DBDoc.offset_begin;
                std::string str_DBWord = docDB->GetstrContents().substr(textrange_DBDoc.offset_begin, n_DBLength);
                std::cout<<"["<<textrange_DBDoc.offset_begin<<","<<textrange_DBDoc.offset_end<<","<<n_DBLength<<"]"<<std::endl;
                std::cout<<str_DBWord<<std::endl;
            }
        }
    }
    else
    {
        const char* pch_DocName =doc->GetstrDocName().c_str();
        const char* pch_SimilarDocName =str_SimilarDoc.c_str();
        std::cout<<"LEAKAGE DOC FOUND: "<<pch_DocName <<" is similar to "<<pch_SimilarDocName<<std::endl;
    }
    delete docDao;
    delete doc;
    return 0;
}

DocumentOperation::~DocumentOperation()
{
    //dtor
}
