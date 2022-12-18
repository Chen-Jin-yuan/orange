#ifndef LOG_H
#define LOG_H

#include<mutex>
#include "time.h"
#include<stdarg.h>
#include<string>
#include<stdio.h>
#include<cassert>
#include<thread>
#include <dirent.h>		//opendir
#include <sys/stat.h>         //mkdir
#include "blockqueue.h"
using namespace std;
class Log
{
private:
    Log();//单例模式构造函数私有，成员函数才能调用构造函数
public:
    Log(Log const&) = delete;
    Log& operator=(Log const&) = delete;
    ~Log();//关闭...//析构函数实际上和构造函数一样，可以private，因为本质上是成员函数调用
    static Log* instance();//单例

    //函数声明和定义，只能有一个使用默认参数，如果函数的声明和定义是分开的，那缺省函数不能在函数声明和定义中同时出现
    //默认参数在函数声明中提供，当又有声明又有定义时，定义中不允许默认参数（定义中的默认参数是无用的，必须传入参数才能找到匹配的函数）
    void init(int level = 1, const char* fpath = "./log", int maxqueue_size = 1024, int threadnum = 1);//不能用构造函数传参，使用一个init传参初始化

    void setlevel(int level) { loglevel = level; }//修改level的接口，只允许主线程修改，因此不用互斥
    int getlevel() { return loglevel; }
    bool isopen() { return logisopen; }//看是否打开日志的接口

    void createthread(int threadnum);
    static void logthread();//异步线程的回调函数，需要是staic，没有this隐藏参数
    void write(int level, const char* format, ...);//同步写，解耦
    void close();
private:
    void asyncwrite();//互斥写，不用lambda表达式，因为要用到log类的变量，并修改它们
    void changefile(struct tm* nowtime);//write函数的解耦
    struct tm* gettime();
private:
    static const int maxlines = 52000;
    FILE* logfp;
    int linecounts;
    int filenum;
    const char* path;
    int logday;
    bool isasync;
    bool logisopen;
    int loglevel;
    unique_ptr<blockqueue<string>> blockque;//不用lambda表达式可以用uniqueptr，因为一个指针一起用。用指针是因为要根据队列长度动态构造
    mutex mux;
};

//我们想用一个函数封装write函数，比如logoinfo调用level1的write，并且还要能判断loglevel支不支持
//但函数封装变参函数，为了传递可变参数，实际上还要修改write的实现，不如用宏来实现，使用##__VA_ARGS__传递可变参数，让编译器把宏替换为真实的函数
//##__VA_ARGS__的优点是，对于宏调用，如果format是一个字符串也即后面没有可变参数，## 操作将使预处理器（preprocessor）去除掉它前面的那个逗号。
//宏与类无关了，这里必须isopen了才能使用
#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::instance();\
        if (log->isopen() && log->getlevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif
