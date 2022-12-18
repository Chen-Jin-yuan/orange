#ifndef CLIENT_H
#define CLIENT_H
#include<iostream>
#include <fstream>//文件流
#include<cstring>//string.h
#include<string>
#include<unordered_map>
#include <thread>
#include <mutex>
#include<functional>
#include<WinSock2.h>//除了inet_pton
#include <WS2tcpip.h>//inet_pton
#include <windows.h>//获取资源信息
#include <direct.h>//创建文件夹 

#include "net_c.h"
#include "cmdparse_c.h"
#include "state_c.h"
#include "reqtable.h"
#include "auxfunc.h"

using namespace std;
extern clientState state;//声明全局变量，在cpp文件里定义
extern bool isRecvingGf;
extern bool isRecvingSf;

class Client
{
private:
    
    string chatSname;//正在chatting的对方的sname
    int fileNumber;//用于对方acceptget后选择文件的最大数量
    unordered_map<int, string> getResourceMap;//获得对方的资源表
    string prompt;//提示符

    string sendfilename;//要发送的完整路径的文件名，从解析sendfile命令获取，当对方接收时根据该文件名发送文件
                        //目前还不支持连续sendfile
    bool chooseflag;
    bool exitflag;//函数退出信号，两个线程共用
    unordered_map<clientState, string> promptMap;//state和提示符的映射
    ReqTable reqTable;//请求表

    //假设有很多配置，这里生成一个配置映射
    unordered_map<string, string> config_map;//映射表
    unordered_map<int, string> myresource_map;
    string myresource_str;//资源列表，用于直接发送

    const int sendfilePort;//sendfile命令对应的接收端口
    const int getfilePort;//getfile命令对应的接收端口

public:
    Client();
    ~Client();
    void run(SOCKET connfd);
    bool parseRecv(string cmdrecv);//解析接收命令，返回解析成不成功，失败则false
    void recvThread(const char* server_ip, const int port);
    void start(const char* server_ip, const int port1, const int port2);//最终调用
    void setConfig();//生成配置
    void genResource(string dirpath);//设置发送的资源列表myresource_str 以及映射表myresoure_map
    void autoLogin(SOCKET connfd);//自动登录
};
#endif
