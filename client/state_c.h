#ifndef STATE_C_H
#define STATE_C_H

//客户端状态机
enum class clientState
{
    noLogin = 0,//未登录状态
    cmdLine,//命令行状态
    isChatting,//chat状态
    isWaiting//等待状态 
};

/*
* 不在该头文件里声明state，因为没必要对应地写一个cpp文件，但是在别的cpp文件定义又很奇怪
* 所以在clien.h声明，在client.cpp定义。

*/

#endif