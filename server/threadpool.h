//threadpool.h
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<mutex>
#include<thread>
#include<condition_variable>
#include<functional>
#include<queue>
#include <cassert>//使用assert函数
class threadpool
{
private:
    struct pool//封装三个资源
    {
        std::mutex mtx;//互斥锁
        std::queue<std::function<void()>> taskQueue;//任务队列，无参数的function，调用时不用传参
        std::condition_variable cond;//条件变量
        bool isclose = false;//默认值是false
    };
    std::shared_ptr<pool> pool_;//共享指针，pool_是一个指针指向pool结构体，这个指针用于线程池操作资源

public:
    threadpool(int threadnum = 8) :pool_(std::make_shared<pool>())//以make_shared的方式new一个对象给pool_指针
    {
        assert(threadnum > 0);//没有线程就报错
        for (int i = 0; i < threadnum; i++)//创建线程池
            std::thread([pool_t = pool_] {//现在要按值捕获，相当于拷贝构造共享指针，计数+1，且指向相同内容
            std::unique_lock<std::mutex> locker(pool_t->mtx);//定义一个locker对象，现在已经锁住了
            while (true)
            {
                if (!pool_t->taskQueue.empty())//如果有任务
                {
                    auto task = pool_t->taskQueue.front();
                    pool_t->taskQueue.pop();
                    locker.unlock();
                    //解锁后再执行
                    task();
                    //执行完了，进入下一轮循环，注意要锁住
                    locker.lock();//抢占锁
                }
                else if (pool_t->isclose)
                    break;
                else//如果没有任务
                    pool_t->cond.wait(locker);//解锁并等待，唤醒后会抢占互斥锁
            }
                }).detach();//把thread分离，不用手动join，结束自动回收
    }

    void addTask(std::function<void()> task)
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);//定义一个locker对象
        pool_->taskQueue.emplace(task);//这种方式，使用emplace和push没啥区别，task本身就是临时对象
        //如果要真正使用到emplace调用构造函数，还要配合std::forward完美转发，此时无论构造函数是不是explicit（不能隐式转换），都可以正常工作
        pool_->cond.notify_one();//插入一个元素唤醒一个线程
    }
    void close()
    {
        pool_->isclose = true;
        pool_->cond.notify_all();
    }
    ~threadpool()//析构函数
    {
    }
};
#endif
