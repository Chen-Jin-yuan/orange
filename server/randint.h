#ifndef RANDINT_H
#define RANDINT_H
#include <iostream>
#include <random>
#include <mutex>
using namespace std;

class Randint
{
private:
    mutex mux;//因为引擎有序的关系，要互斥
    random_device seed;//硬件生成随机数种子
    ranlux48 engine;//利用种子生成随机数引擎
    uniform_int_distribution<int> distrib;//设置随机数范围，并为均匀分布
public:
    //hisir语录和oyasumi语录大小可能不一样，所以对应不一样的随即器
    Randint(int min, int max) :engine(seed()), distrib(min, max) {}//初始列表构造

    int operator()()
    {
        lock_guard<mutex> locker(mux);
        return distrib(engine);
    }
};
#endif
