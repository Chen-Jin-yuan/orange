#include "sqlconnpool.h"
#include <chrono>
#include <cassert>
#include "log.h"
using namespace std;
Sqlconnpool::Sqlconnpool()
{
    maxconn = 0;
    freecount = 0;
}

Sqlconnpool* Sqlconnpool::instance()
{
    static Sqlconnpool instance;
    return &instance;
}

MYSQL* Sqlconnpool::getconn(int timeout = 0)
{
    assert(timeout >= 0);//
    unique_lock<mutex> locker(mux);
    while (connque.empty())
        if (cond.wait_for(locker, chrono::seconds(timeout)) == std::cv_status::timeout)//超时
        {
            LOG_WARN("Sqlconnpool busy")
                return nullptr;
        }

    MYSQL* sql = connque.front();
    connque.pop();
    freecount--;
    return sql;
}

void Sqlconnpool::freeconn(MYSQL* conn)
{
    assert(conn);//防止放入nullptr
    lock_guard<mutex> locker(mux);
    connque.push(conn);
    freecount++;
    cond.notify_one();//唤醒一个get线程
}
int Sqlconnpool::conncount()
{
    lock_guard<mutex> locker(mux);
    return maxconn - freecount;
}

void Sqlconnpool::init(const char* host, int port, const char* user, const char* pwd, const char* dbname, int connsize)
{
    assert(connsize > 0);
    maxconn = connsize;
    freecount = connsize;
    for (int i = 0; i < maxconn; i++)
    {
        //三步初始化
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("sql number %d init error", i);
            assert(sql);//终止报错
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbname, port, nullptr, 0);
        if (!sql)
        {
            LOG_ERROR("sql number %d connect error", i);
            assert(sql);//终止报错
        }
        connque.push(sql);//放入的一定不是nullptr
    }
}

void Sqlconnpool::close()
{
    unique_lock<mutex> locker(mux);
    while (freecount != maxconn)//必须要等待所有连接都放回来，直接close再执行查询程序会崩溃
        cond.wait(locker);//每放回一个连接唤醒一次，然后判断

    while (!connque.empty())//逐个关闭连接
    {
        mysql_close(connque.front());
        connque.pop();
    }
    mysql_library_end();//释放库的资源
}
