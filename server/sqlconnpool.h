#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <mutex>
#include <queue>
#include <condition_variable>
using namespace std;
class Sqlconnpool
{
private:
    mutex mux;
    condition_variable cond;
    queue<MYSQL*> connque;
    int maxconn;
    int freecount;
    Sqlconnpool();
    ~Sqlconnpool() {}//析构函数实际上和构造函数一样，可以private，因为本质上是成员函数调用
public:
    Sqlconnpool(const Sqlconnpool&) = delete;
    Sqlconnpool& operator=(const Sqlconnpool&) = delete;

    void close();
    static Sqlconnpool* instance();
    MYSQL* getconn(int timeout);
    void freeconn(MYSQL* conn);
    //无法使用构造函数传参，用init，默认参数写声明中
    void init(const char* host, int port, const char* user, const char* pwd, const char* dbname, int connsize = 20);
    int conncount();

};

#endif
