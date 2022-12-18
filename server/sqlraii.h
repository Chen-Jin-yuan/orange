#ifndef SQLRAII_H
#define SQLRAII_H

#include "sqlconnpool.h"
#include <cassert>
using namespace std;
class SqlRAII
{
private:
    MYSQL* conn;//保存连接好的sql
    Sqlconnpool* connpool;//保存连接池
public:
    SqlRAII(MYSQL** sql, Sqlconnpool* sqlpool, int timeout = 0)//传入sql指针的地址，即&sql，获取连接后传出去
    {
        assert(sqlpool);//必须先建好连接池

        *sql = sqlpool->getconn(timeout);//可能会超时
        //为了用户自行getconn和使用sqlraii的统一，这里统一让用户在上层处理sql为nullptr的情况
        conn = *sql;
        connpool = sqlpool;
    }
    ~SqlRAII()
    {
        if (conn)//有连接就释放
            connpool->freeconn(conn);
    }
};

#endif
