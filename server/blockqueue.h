#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <cassert>
#include <chrono>
template<class T>
class blockqueue
{
private:
    std::queue<T> que;
    std::mutex mux;
    std::condition_variable condprod;
    std::condition_variable condcons;
    size_t size;
    bool isclose;
public:
    blockqueue(int maxsize = 1024) :size(maxsize), isclose(false)
    {
        assert(maxsize > 0);//初始化检查
    }

    ~blockqueue()//上层调用close即可
    {
    }

    void close();
    void clear();
    bool empty();
    bool full();

    void push(const T& task);
    bool pop(T& task);
    bool pop(T& task, int timeout);


};


template<class T>
bool blockqueue<T>::empty()
{
    //这是日志系统调用的函数，阻塞队列的pop不能调用，否则会死锁
    std::lock_guard<std::mutex> locker(mux);
    return que.empty();
}

template<class T>
bool blockqueue<T>::full()
{
    //这是日志系统调用的函数，阻塞队列的push不能调用，否则会死锁
    std::lock_guard<std::mutex> locker(mux);
    return que.size() >= size;
}

template<class T>
void blockqueue<T>::push(const T& task)
{
    //插入元素，首先抢占互斥锁，但即使抢占了互斥锁也可能不能插入，队列可能是满的，这时要释放锁让消费者线程获得锁
    std::unique_lock<std::mutex> locker(mux);//要用条件变量，用unique锁
    while (que.size() >= size)//避免虚假唤醒，notify_one一般不会导致虚假唤醒，但要随时最好准备。并且当要关闭时会notify_all
        condprod.wait(locker);//等待唤醒
    if (isclose)
        return;//不能插入元素
    que.push(task);//插入元素
    condcons.notify_one();//唤醒消费者
}

template<class T>
bool blockqueue<T>::pop(T& task)
{
    std::unique_lock<std::mutex> locker(mux);
    while (que.empty() and !isclose)
        condcons.wait(locker);

    if (isclose)//关闭的信号
        return false;

    task = que.front();
    que.pop();
    condprod.notify_one();
    return true;
}

template<class T>
bool blockqueue<T>::pop(T& task, int timeout)
{
    std::unique_lock<std::mutex> locker(mux);
    while (que.empty() and !isclose)//为空就等待，等待过程中如果超时就返回
        if (condcons.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)//超时
            return false;

    if (isclose)//关闭的信号
        return false;
    task = que.front();
    que.pop();
    condprod.notify_one();
    return true;
}

template<class T>
void blockqueue<T>::close()
{
    /*
    * 当关闭时，我们的目标是要让所有的线程都退出，也就是不能被阻塞到push和pop里
    * 调用此函数的时机是，上层日志系统已经把任务都做完，然后关闭队列
    * 则调用这个函数后，所有想再次尝试push和pop的线程都不允许
    */
    std::lock_guard<std::mutex> locker(mux);//要锁住，然后clear队列
    clear();
    isclose = true;//修改信号
    //唤醒所有线程
    condprod.notify_all();
    condcons.notify_all();
}

template<class T>
void blockqueue<T>::clear()
{
    //close已经锁住了，不用锁了
    //高效的方式，swap一个空队列
    std::queue<T> empty;
    std::swap(empty, que);
}

#endif
