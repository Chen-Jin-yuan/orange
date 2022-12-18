#ifndef NET_C_H
#define NET_C_H
/*
* 该头文件管理与网络相关的辅助函数
* 连接
* 接收文件
* 发送文件
*/
#include <iostream>
#include <WinSock2.h>//除了inet_pton
#include <WS2tcpip.h>//inet_pton，inet_ntop（获取ip）
#include <chrono>
#include <string>

#include "rdt_on_udp.h"
#include "udp_hole_punch.h"
#define buffSize  102400//100KB缓冲区，会报warning
//这个常数（默认是16K）是单个函数使用堆栈的阈值；用户可使用默认堆栈总大小为1M。

//判断接收文件线程是否正在下载文件的全局变量，进入recvfile就设为true，任何退出都要设为false
extern bool isRecvingGf;
extern bool isRecvingSf;

void initSocket();//初始化winsock，整个进程使用一次即可。
SOCKET connect_S(const char* server_ip, int port);//返回连接句柄
void sendfile(const char* server_ip, const int port, std::string filename);
//0是sendfile，1是getfile
void recvfile(const int port, std::string filename ,int flag);//flag用于表示是getfile还是sendfile
#endif