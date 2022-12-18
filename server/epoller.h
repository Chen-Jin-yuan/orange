//Epoller.h
#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll操作
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <cassert>
#include <vector>
class Epoller
{
private:
    int eventSize;
    const bool nonBlock;
    int epollFd;
    
    std::vector<struct epoll_event> events;
public:
    Epoller(const int eventsize = 1024, const bool ifNonBlock = true):
    eventSize(eventsize),nonBlock(ifNonBlock),epollFd(epoll_create(5)),events(eventsize)
    {
        assert(epollFd>=0);
    }
    ~Epoller()
    {
        close(epollFd);
    }
    
    int setFdNonblock(int fd)
    {
    	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);//出错返回-1
	}
    
    bool addFd(int fd, uint32_t events) 
    {
        if(fd < 0) return false;
        //如果需要设置非阻塞，根据布尔运算就会调用SetFdNonblock函数，如果返回不是-1就成功，是-1就返回false
        if(nonBlock && (setFdNonblock(fd)==-1))
                return false;

        epoll_event ev = {0};
        ev.data.fd = fd;//关联fd
        ev.events = events;//上层设置好类型
        return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);//add，成功返回0
	}

	bool modFd(int fd, uint32_t events) 
    {
        if(fd < 0) return false;
        epoll_event ev = {0};
        ev.data.fd = fd;
        ev.events = events;
        return 0 == epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);//mod
	}

	bool delFd(int fd)
    {
        if(fd < 0) return false;
        return 0 == epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0);
	}
    
    //对于timeout：-1：永远等待；0：不等待直接返回，执行下面的代码；其他：在超时时间内没有事件发生，返回0，如果有事件发生立即返回
    //默认不等待
    int wait(int timeoutMs = 0) //成功返回多少事件就绪，超时返回0，出错返回-1
    {
        return epoll_wait(epollFd, &events[0], eventSize, timeoutMs);
    }
    int getEventFd(size_t i)
    {
        return events[i].data.fd;
    }
    uint32_t getEvents(size_t i)
    {
        return events[i].events;
    }
};

#endif
