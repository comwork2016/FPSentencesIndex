#include "DocumentDao.h"
#include "DocumentOperation.h"
#include "ReadCorpus.h"
#include <time.h>

//静态变量的初始化
std::map<std::string,double> ReadCorpus::map_CorpusTF;

int main()
{
    //设置全局环境本地化
    std::locale::global(std::locale(""));
    clock_t start,finish;
    double duration;
    start = clock();
    //读取语料库中的词频信息
    ReadCorpus::ReadCorpusTF("./Corpus/Corpus.csv");
    //输出语料库信息
    //将一个目录中的文件加入到数据库中
    //std::string str_InputDir = "./in/";
    //DocumentOperation::AddDirectoryDocuments(str_InputDir);
    //DocumentOperation::AddDocument("./in/Winnowing.txt");
    //DocumentOperation::AddDocument("./in/utf_104.txt");
    //查询数据泄露
    //std::string str_LeakDoc = "./test/leak.txt";
    //DocumentOperation::SearchLeak(str_LeakDoc);
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    std::cout<<std::endl<<std::endl<<"cost "<<duration<<" secs"<<std::endl<<std::endl;
    return 0;
}
