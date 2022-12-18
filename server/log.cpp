#include "log.h"
using namespace std;

Log* Log::instance()
{
    static Log instance;//调用构造函数
    return &instance;
}
struct tm* Log::gettime()
{
    struct tm* nowtime;
    time_t t;
    t = time(NULL);
    nowtime = localtime(&t);
    return nowtime;
}

void Log::changefile(struct tm* nowtime)//完成行数增加、判断文件切换
{
    //接下来涉及行数的改写，以及文件的切换，要互斥。因为实际上一个线程切换文件即可，如果有线程在切换其他线程不可动
    //unique_lock<mutex> locker(mux);互斥交给上层，因为fputs也要互斥
    linecounts++;//先++，因为行数是从0开始的，++后刚好判断是不是满了，这个操作要互斥
    if (logday != nowtime->tm_mday || linecounts == maxlines)//如果换了一天或行数满了
    {
        char newname[48];//用snprintf，不能用string了
        if (logday != nowtime->tm_mday)//如果是换了一天
        {
            logday = nowtime->tm_mday;//修改天
            linecounts = 0;//换文件了
            filenum = 0;//文件份数从0开始
            //为了格式化命名，要用format，这里用snprintf写入str
            snprintf(newname, 48, "%s/%d-%02d-%02d_log%05d.txt",//补充一个前缀-文件夹
                path, nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday, filenum);
        }
        else//行数满了
        {
            linecounts = 0;
            filenum++;
            snprintf(newname, 48, "%s/%d-%02d-%02d_log%05d.txt",
                path, nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday, filenum);
        }

        fflush(logfp);//在关闭文件前要把文件缓存区的内容写完
        fclose(logfp);
        logfp = fopen(newname, "w");
        assert(logfp != nullptr);//创建失败报错
    }
    //locker.unlock();
}

void Log::write(int level, const char* format, ...)
{
    //初始化时间
    struct tm* nowtime = gettime();

    //-----------------根据传入的信息整理成一行日志-------------------------
    char infobuffer[128];//一般一行日志没那么长，128足够了
    char timebuffer[64];//时间头
    string allinfo;
    //分级
    switch (level)
    {
    case 0:
        allinfo += "[debug]";
        break;
    case 1:
        allinfo += "[info]";
        break;
    case 2:
        allinfo += "[warning]";
        break;
    case 3:
        allinfo += "[error]";
        break;
    default:
        allinfo += "[info]";
        break;
    }
    //添加时间信息
    snprintf(timebuffer, 64, "%d-%02d-%02d_%02d:%02d:%02d: ",
        nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday,
        nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec);//只精确到秒，更具体的信息交给内容体现

    allinfo += string(timebuffer);

    //写内容
    va_list vaList;
    va_start(vaList, format);
    vsnprintf(infobuffer, 128, format, vaList);
    va_end(vaList);

    allinfo += string(infobuffer) + "\n";//注意换个行
    //----------------------------------------------------------------------------------------------

    //分异步还是同步，要不要切换文件交给异步线程判断 
    if (isasync && !blockque->full())
        blockque->push(allinfo);//异步直接插入
    else
    {
        //在写之前看要不要创建新文件
        //如果当前日期变了，一般来说判断day就可以了；或是行数已满，就换一个文件

        lock_guard<mutex> locker(mux);//互斥
        changefile(nowtime);//直接交给该函数完成
        fputs(allinfo.c_str(), logfp);//操作文件缓冲区，也要互斥
        fflush(logfp);
    }
}

Log::Log()//初始化一部分变量
{
    //初始行数为-1，因为是先++然后判断再写入，初始为0的话，第一份文件会少一行，
    //比如最大行为2，初始为0；则++，写入；++就换文件了，只写了一行，所以初始要是-1。换文件后置为0
    //因为换文件后没++了，写了一行，就是正确的
    linecounts = -1;
    filenum = 0;
    isasync = false;
    blockque = nullptr;
    logday = 0;
    logfp = nullptr;
    logisopen = false;//init才算打开
}
void Log::close()
{

    logisopen = false;
    if (isasync)//异步的话要让线程退出
    {
        while (!blockque->empty());//等待工作完成

        blockque->close();
    }
    if (logfp)//如果打开了文件要关闭
    {
        //由于其他线程可能正在使用，因此要等待互斥锁
        lock_guard<mutex> locker(mux);
        fflush(logfp);//刷新缓冲区
        fclose(logfp);
        logfp = nullptr;
    }
}
Log::~Log()
{
    //close();
}

void Log::logthread()//异步线程回调函数
{
    Log::instance()->asyncwrite();//调用类成员函数
}
//等级越低允许记录的权限越低，fpath是文件夹，文件名内部设置
void Log::init(int level, const char* fpath, int maxqueue_size, int threadnum)
{
    if (logisopen == true)
        return;//只允许init一次
    logisopen = true;
    loglevel = level;
    if (maxqueue_size > 0)//有阻塞队列则异步
    {
        isasync = true;
        //创建阻塞队列
        unique_ptr<blockqueue<string>> que(new blockqueue<string>(maxqueue_size));
        blockque = move(que);//移动赋值
        createthread(threadnum);
    }
    else
        isasync = false;

    //初始化时间
    struct tm* nowtime = gettime();
    logday = nowtime->tm_mday;
    path = fpath;

    char filename[48];//用snprintf，不能用string了
    snprintf(filename, 48, "%s/%d-%02d-%02d_log%05d.txt",//补充一个前缀-文件夹
        path, nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday, filenum);

    //初步打开文件，没有文件夹就创建文件夹
    if (opendir(path) == NULL)//如果文件夹不存在
        mkdir(path, 0777);//0777是最大的访问权

    logfp = fopen(filename, "w");
    assert(logfp != nullptr);
}

void Log::asyncwrite()
{
    string str;
    while (blockque->pop(str))
    {
        struct tm* nowtime = gettime();
        lock_guard<mutex> locker(mux);//互斥
        changefile(nowtime);//每写一行判断要不要换文件
        fputs(str.c_str(), logfp);
        fflush(logfp);
    }
}

void Log::createthread(int threadnum)
{
    for (int i = 0; i < threadnum; i++)
    {
        thread(logthread).detach();//因为内部函数采用单例调用，logthread不用传入this指针
    }
}
