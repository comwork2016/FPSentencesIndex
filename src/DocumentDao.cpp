#include "DocumentDao.h"

DocumentDao::DocumentDao()
{
    //ctor
    this->m_Host = "localhost:27017";
    this->m_DBName = "FP.docs";
    this->m_Conn.connect(this->m_Host);
}

/**
    插入一个文档到数据库中
*/
int DocumentDao::InsertDocument(const Document* doc)
{
    mongo::BSONObjBuilder b;
    //保存文档信息
    b.append("filename",doc->GetstrDocName());
    b.append("filepath", doc->GetstrDocPath());
    const char* pch_Contents= doc->GetstrContents().c_str();
    b.append("filelength",static_cast<int>(StringUtil::ConvertCharArraytoWString(pch_Contents).length()));
    b.appendNumber("docsimhash",static_cast<long long>(doc->GetlSimHash()));
    b.appendNumber("docsimhash1",static_cast<long long>(doc->GetlSimHash16_1()));
    b.appendNumber("docsimhash2",static_cast<long long>(doc->GetlSimHash16_2()));
    b.appendNumber("docsimhash3",static_cast<long long>(doc->GetlSimHash16_3()));
    b.appendNumber("docsimhash4",static_cast<long long>(doc->GetlSimHash16_4()));
    b.appendNumber("fingersize",doc->GetKGramFingerPrints().size());
    std::vector<KGramHash> vec_FingerPrints = doc->GetKGramFingerPrints();
    mongo::BSONArrayBuilder bb_FPArray;
    for(std::vector<KGramHash>::iterator it = vec_FingerPrints.begin(); it!=vec_FingerPrints.end(); it++)
    {
        mongo::BSONObjBuilder bb_fp;
        KGramHash kgramHash = *it;
        bb_fp.appendNumber("hash",static_cast<long long>(kgramHash.hashValue));
        mongo::BSONObjBuilder bb_offset;
        bb_offset.append("begin",kgramHash.textRange.offset_begin);
        bb_offset.append("end",kgramHash.textRange.offset_end);
        //bb.append(str_Hash,bb_offset.obj());
        bb_fp.append("pos",bb_offset.obj());
        bb_FPArray.append(bb_fp.obj());
    }
    b.append("fingerprints",bb_FPArray.arr());
    this->m_Conn.insert(this->m_DBName,b.obj());
    return 0;
}

//插入一个文档到数据库中
int DocumentDao::DeleteAll()
{
    //this->m_Conn.dropCollection(this->m_DBName);
    this->m_Conn.remove(this->m_DBName,mongo::Query());
    return 0;
}

//从数据库中查询simhash值相似的文件名称，没有则返回""
std::string DocumentDao::QuerySIMSimilarity(const Document* doc)
{
    std::cout<<"Query similar simhash of document "<<doc->GetstrDocName()<<std::endl;
    std::string str_SimilarDoc = "";
    mongo::BSONObj bson_condition1 = BSON("docsimhash1"<<static_cast<long long>(doc->GetlSimHash16_1()));
    mongo::BSONObj bson_condition2 = BSON("docsimhash2"<<static_cast<long long>(doc->GetlSimHash16_2()));
    mongo::BSONObj bson_condition3 = BSON("docsimhash3"<<static_cast<long long>(doc->GetlSimHash16_3()));
    mongo::BSONObj bson_condition4 = BSON("docsimhash4"<<static_cast<long long>(doc->GetlSimHash16_4()));
    mongo::BSONObj bson_condition = mongo::OR(bson_condition1,bson_condition2,bson_condition3,bson_condition4);
    mongo::BSONObj bo_columns = BSON("docsimhash"<<1<<"filelength"<<1<<"filepath"<<1);
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->m_Conn.query(this->m_DBName,bson_condition,0,0,&bo_columns);
    while (cursor->more())
    {
        mongo::BSONObj p = cursor->next();
        SIMHASH_TYPE l_SimHash = p.getField("docsimhash").numberLong();
        if(HashUtil::IsSimHashSimilar(doc->GetlSimHash(),l_SimHash))
        {
            //两个近似网页的文章长度差距应在20%以内
            int n_DBFileLength = p.getIntField("filelength");
            const char* pch_Contents= doc->GetstrContents().c_str();
            int n_DocFileLength = StringUtil::ConvertCharArraytoWString(pch_Contents).length();
            float f_LengthSim = (float)(n_DBFileLength-n_DocFileLength)/(n_DocFileLength>n_DBFileLength?n_DocFileLength:n_DBFileLength);
            if( f_LengthSim < 0.2)
            {
                str_SimilarDoc = std::string(p.getStringField("filepath"));
                break;
            }
        }
    }
    return str_SimilarDoc;
}

/**
    对相同内容进行扩展匹配
    汉字的第一个字节的最高为为1
    英文最高位为0，且小于等于127
    传入的范围不一定相邻，所以要在整体上合并范围
*/
void DocumentDao::ExtendMatch(const Document* doc, const Document *docDB,std::vector<TextRange>& vec_SearchDocSimilarTextRange,std::vector<TextRange>& vec_DBDocSimilarTextRange,int& n_SameContentsBytes)
{
    //待比对的文档中相同指纹范围 和 数据库的文档中相同指纹范围；
    std::vector<TextRange> vec_ExtendedSearchDocSimilarTextRange;
    std::vector<TextRange> vec_ExtendedDBDocSimilarTextRange;

    //对第一处相同的指纹进行向前扩展匹配
    TextRange textrange_SearchDocFirst = vec_SearchDocSimilarTextRange[0];
    TextRange textrange_DBDocFirst = vec_DBDocSimilarTextRange[0];
    int n_SearchBegin = textrange_SearchDocFirst.offset_begin;
    int n_DBBegin = textrange_DBDocFirst.offset_begin;
    while(n_SearchBegin >=0 && n_DBBegin>=0 && doc->GetstrContents()[n_SearchBegin] == docDB->GetstrContents()[n_DBBegin])
    {
        n_SearchBegin--;
        n_DBBegin--;
    }
    //回到相等的位置
    n_SearchBegin++;
    n_DBBegin++;
    //判断是否是一个字符的边界
    while(doc->GetstrContents()[n_SearchBegin] > 127 && docDB->GetstrContents()[n_DBBegin] > 127)//既不是英文数字，也不是中文的第一个字节
    {
        n_SearchBegin++;
        n_DBBegin++;
    }
    textrange_SearchDocFirst.offset_begin = n_SearchBegin;
    textrange_DBDocFirst.offset_begin = n_DBBegin;
    //添加到扩展范围向量中
    n_SameContentsBytes += textrange_SearchDocFirst.offset_end - textrange_SearchDocFirst.offset_begin;
    vec_ExtendedSearchDocSimilarTextRange.push_back(textrange_SearchDocFirst);
    vec_ExtendedDBDocSimilarTextRange.push_back(textrange_DBDocFirst);

    //对于中间的相同指纹，进行向前向后匹配
    for(int i=1; i<vec_SearchDocSimilarTextRange.size(); i++)
    {
        //当前的相同指纹范围
        TextRange textrange_SearchDoc = vec_SearchDocSimilarTextRange[i];
        TextRange textrange_DBDoc = vec_DBDocSimilarTextRange[i];
        //上一个相同指纹的范围
        TextRange textrange_SearchDocLast = vec_ExtendedSearchDocSimilarTextRange.back(); // 上一个扩展范围
        TextRange textrange_DBDocLast = vec_ExtendedDBDocSimilarTextRange.back(); // 上一个扩展范围
        //两个相同文本范围有重复，则合并处理
        if(textrange_SearchDoc.offset_begin <= textrange_SearchDocLast.offset_end && textrange_DBDoc.offset_begin <= textrange_DBDocLast.offset_end)
        {
            //修改相同文本范围
            textrange_SearchDoc.offset_begin = textrange_SearchDocLast.offset_begin;
            textrange_DBDoc.offset_begin = textrange_DBDocLast.offset_begin;
            //同时删除向量中的上一条记录。
            n_SameContentsBytes -= textrange_SearchDocLast.offset_end - textrange_SearchDocLast.offset_begin;
            vec_ExtendedSearchDocSimilarTextRange.pop_back();
            vec_ExtendedDBDocSimilarTextRange.pop_back();
        }
        else // 如果相邻两个范围没有重复，则对最后一个进行向后扩展匹配，对当前的进行向前扩展匹配
        {
            // 最后一个向后扩展匹配至当前相同文本的开始偏移值
            int n_SearchLastEnd = textrange_SearchDocLast.offset_end;
            int n_DBLastEnd = textrange_DBDocLast.offset_end;
            while(n_SearchLastEnd <= textrange_SearchDoc.offset_begin && n_DBLastEnd <= docDB->GetstrContents().size() && doc->GetstrContents()[n_SearchLastEnd] == docDB->GetstrContents()[n_DBLastEnd])
            {
                n_SearchLastEnd++;
                n_DBLastEnd++;
            }
            //当前一个向前扩展匹配至上一个相同文本的最后偏移值
            int n_SearchBegin = textrange_SearchDoc.offset_begin;
            int n_DBBegin = textrange_DBDoc.offset_begin;
            while(n_SearchBegin >= textrange_SearchDocLast.offset_end && n_DBBegin >= 0 && doc->GetstrContents()[n_SearchBegin] == docDB->GetstrContents()[n_DBBegin])
            {
                n_SearchBegin--;
                n_DBBegin--;
            }
            //如果两个扩展匹配重合，删除上一个相同文本范围并与当前文本范围合并成一个
            if(n_SearchLastEnd >= n_SearchBegin && n_DBLastEnd >= n_DBBegin)
            {
                textrange_SearchDoc.offset_begin = textrange_SearchDocLast.offset_begin;
                textrange_DBDoc.offset_begin = textrange_DBDocLast.offset_begin;
                n_SameContentsBytes -= textrange_SearchDocLast.offset_end - textrange_SearchDocLast.offset_begin;
                vec_ExtendedSearchDocSimilarTextRange.pop_back();
                vec_ExtendedDBDocSimilarTextRange.pop_back();
            }
            else
            {
                //如果两个匹配范围不重合，则修改上一个文本范围，并修改当前文本范围
                n_SameContentsBytes -= textrange_SearchDocLast.offset_end - textrange_SearchDocLast.offset_begin;
                vec_ExtendedSearchDocSimilarTextRange.pop_back();
                vec_ExtendedDBDocSimilarTextRange.pop_back();
                //判断是否是一个字符的边界
                while(doc->GetstrContents()[n_SearchLastEnd] > 127 && docDB->GetstrContents()[n_DBLastEnd] > 127)//既不是英文数字，也不是中文的第一个字节
                {
                    n_SearchLastEnd--;
                    n_DBLastEnd--;
                }
                textrange_SearchDocLast.offset_end = n_SearchLastEnd;
                textrange_DBDocLast.offset_end = n_DBLastEnd;
                n_SameContentsBytes += textrange_SearchDocLast.offset_end - textrange_SearchDocLast.offset_begin;
                vec_ExtendedSearchDocSimilarTextRange.push_back(textrange_SearchDocLast);
                vec_ExtendedDBDocSimilarTextRange.push_back(textrange_DBDocLast);
                //回到相等的位置
                n_SearchBegin++;
                n_DBBegin++;
                //判断是否是一个字符的边界
                while(doc->GetstrContents()[n_SearchBegin] > 127 && docDB->GetstrContents()[n_DBBegin] > 127)//既不是英文数字，也不是中文的第一个字节
                {
                    n_SearchBegin++;
                    n_DBBegin++;
                }
                textrange_SearchDoc.offset_begin = n_SearchBegin;
                textrange_DBDoc.offset_begin = n_DBBegin;
            }
        }
        n_SameContentsBytes += textrange_SearchDoc.offset_end - textrange_SearchDoc.offset_begin;
        vec_ExtendedSearchDocSimilarTextRange.push_back(textrange_SearchDoc);
        vec_ExtendedDBDocSimilarTextRange.push_back(textrange_DBDoc);
    }

    // 对最后一个相同文本进行向后扩展匹配
    TextRange textrange_SearchDocLast = vec_ExtendedSearchDocSimilarTextRange.back(); // 上一个相同文本的范围
    TextRange textrange_DBDocLast = vec_ExtendedDBDocSimilarTextRange.back(); // 上一个相同文本的范围
    n_SameContentsBytes -= textrange_SearchDocLast.offset_end - textrange_SearchDocLast.offset_begin;
    vec_ExtendedSearchDocSimilarTextRange.pop_back();
    vec_ExtendedDBDocSimilarTextRange.pop_back();
    int n_SearchLastEnd = textrange_SearchDocLast.offset_end;
    int n_DBLastEnd = textrange_DBDocLast.offset_end;
    while(n_SearchLastEnd < doc->GetstrContents().size() && n_DBLastEnd < docDB->GetstrContents().size() && doc->GetstrContents()[n_SearchLastEnd] == docDB->GetstrContents()[n_DBLastEnd])
    {
        n_SearchLastEnd++;
        n_DBLastEnd++;
    }
    //判断是否是一个字符的边界
    while(doc->GetstrContents()[n_SearchLastEnd] > 127 && docDB->GetstrContents()[n_DBLastEnd] > 127)//既不是英文数字，也不是中文的第一个字节
    {
        n_SearchLastEnd--;
        n_DBLastEnd--;
    }
    textrange_SearchDocLast.offset_end = n_SearchLastEnd;
    textrange_DBDocLast.offset_end = n_DBLastEnd;
    n_SameContentsBytes += textrange_SearchDocLast.offset_end - textrange_SearchDocLast.offset_begin;
    vec_ExtendedSearchDocSimilarTextRange.push_back(textrange_SearchDocLast);
    vec_ExtendedDBDocSimilarTextRange.push_back(textrange_DBDocLast);
    // 修改传入的引用形参，用于返回
    vec_SearchDocSimilarTextRange = vec_ExtendedSearchDocSimilarTextRange;
    vec_DBDocSimilarTextRange = vec_ExtendedDBDocSimilarTextRange;
}

//从数据库中查询指纹相似的文档
std::vector<FingerPrintsSimilarDocument> DocumentDao::GetFingerPrintsSimilarDocument(Document* doc)
{
    std::vector<FingerPrintsSimilarDocument> vec_SimilarDocument;
    int n_DocFingerSize = doc->GetKGramFingerPrints().size();
    // 查询数据库
    mongo::BSONObj bo_columns = BSON("fingerprints"<<1<<"filepath"<<1);
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->m_Conn.query(this->m_DBName,mongo::Query(),0,0,&bo_columns);
    while (cursor->more())
    {
        mongo::BSONObj p = cursor->next();
        std::string str_DocPathInDB = p.getStringField("filepath");
        mongo::BSONObj bson_FingerPrints = p.getObjectField("fingerprints"); //数据库中一个文档的指纹信息
        std::vector<TextRange> vec_SearchDocSimilarTextRange;//待比对的文档中相同指纹范围
        std::vector<TextRange> vec_DBDocSimilarTextRange;//数据库的文档中相同指纹范围
        //遍历待比对的文档指纹
        std::vector<KGramHash> docFingerPrints = doc->GetKGramFingerPrints();
        for(std::vector<KGramHash>::iterator it = docFingerPrints.begin(); it!= docFingerPrints.end(); it++)
        {
            KGramHash kgramHash_SearchDoc = *it;
            std::stringstream ss;
            ss << kgramHash_SearchDoc.hashValue;
            mongo::BSONObj bson_fingerPos = bson_FingerPrints.getObjectField(ss.str().c_str());
            //数据库文档中存在该指纹
            if(!bson_fingerPos.isEmpty())
            {
                TextRange textrange_SearchDoc;
                TextRange textrange_DBDoc;
                textrange_SearchDoc.offset_begin = kgramHash_SearchDoc.textRange.offset_begin;
                textrange_SearchDoc.offset_end = kgramHash_SearchDoc.textRange.offset_end;
                textrange_DBDoc.offset_begin = bson_fingerPos.getIntField("begin");
                textrange_DBDoc.offset_end = bson_fingerPos.getIntField("end");
                vec_SearchDocSimilarTextRange.push_back(textrange_SearchDoc);
                vec_DBDocSimilarTextRange.push_back(textrange_DBDoc);
            }
        }
        //如果有匹配的指纹，则进行扩展匹配
        int n_SameContentsBytes = 0;
        if(!vec_SearchDocSimilarTextRange.empty())
        {
            Document* docDB = new Document(str_DocPathInDB);
            ExtendMatch(doc,docDB,vec_SearchDocSimilarTextRange,vec_DBDocSimilarTextRange,n_SameContentsBytes);
            delete docDB;
        }
        //遍历完成之后计算相似度
        if(n_SameContentsBytes!=0)
        {
            //保存相似文档信息
//            int n_MinDocSize = doc->GetstrContents().size() < docDB->GetstrContents().size()? doc->GetstrContents().size() : docDB->GetstrContents().size();
//            float f_similarity = float(n_SameContentsBytes)/n_MinDocSize;
            float f_similarity = float(n_SameContentsBytes)/doc->GetstrContents().size();
            FingerPrintsSimilarDocument similarDoc;
            similarDoc.str_SearchDoc = doc->GetstrDocPath();
            similarDoc.str_DBDoc = str_DocPathInDB;
            similarDoc.f_similarity = f_similarity;
            similarDoc.vec_SearchDocSimilarTextRange = vec_SearchDocSimilarTextRange;
            similarDoc.vec_DBDocSimilarTextRange = vec_DBDocSimilarTextRange;
            //将相似文档信息添加到向量中
            vec_SimilarDocument.push_back(similarDoc);
        }
    }
    return vec_SimilarDocument;
}

DocumentDao::~DocumentDao()
{
    //dtor
}
