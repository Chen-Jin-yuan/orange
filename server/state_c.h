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


#endif
