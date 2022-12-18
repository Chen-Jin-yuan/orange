#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <functional>
#include <random>//随机数
#include <sys/socket.h>
#include <netinet/in.h>//sockaddr_in
#include <arpa/inet.h>//in_addr
#include <cstring>
#include <unistd.h>//close

#include "epoller.h"
#include "global_variables.h"
#include "threadpool.h"
#include "sqlconnpool.h"
#include "sqlraii.h"
#include "log.h"
#include "usermap.h"
#include "randint.h"
#include "udp_hole_punch.h"

using namespace std;

class Server
{
public:
	Server(int sqlPort, const char* sqlUser, const  char* sqlPwd, const char* dbName, int connPoolNum,
		int logLevel, const char* logPath, int logQueSize, int logThreadNum,
		int taskThreadNum, string exitpwd_, int timeout, int max_events);
	~Server() {};
	void closeServer();
	void start();

private:
	void init_all_Socket();//初始化所有端口，复用单端口代码
	void init_one_Socket(int& listenfd, const int port);//初始化一个端口
	void listen_1();//处理端口1连接
	void listen_2();//处理端口2连接
	void deal_close();//接收关闭信号
	
	//---------------------------------命令处理----------------------------------
	//===========================================================================
	void task(int conn1);//主要的工作函数
	vector<string> parse(string cmdstr);//解析命令函数
	string recv_str(int conn1);//接收客户端信息函数
	bool msg_log(string userid, string& msglog);//记录聊天日志
	void chatmsg(int conn1, string& recvstr);//处理聊天信息
	void verify(int conn1, int isLogin, string userid, string password);//登录注册管理
	void login_addmap(int conn1, string userid);//登录时添加一系列映射
	void search(int conn1, string sid);
	void chat(int conn1, string sid);
	void accept_(int conn1, string sid);
	void reject(int conn1, string sid);
	void break_(int conn1);//加下划线为了避免和break关键字重合，这里不close，在epoll里close
	void send_(int conn1, string sid, string& msg);
	void sendfile(int conn1, string sid, string filename);
	void acceptfile(int conn1, string sid);
	void rejectfile(int conn1, string sid);
	void getfile(int conn1, string sid);
	void acceptget(int conn1, string sid, string& src);
	void rejectget(int conn1, string sid);
	void choosefile(int conn1, string number);//服务器没有re命令，但是多了一个choosefile，通告选择的文件号码
	void setsid(int conn1, string& newsid);
	void setsname(int conn1, string& newsname);
	void exit_(int conn1);//退出相当于break+clean map+close
	void cleanmap(int conn1);//删表，oyasumi和exit都可以使用
	void hisir(int conn1, string& msg);
	void oyasumi(int conn1);

//私有变量
	int timeout;//延时
	bool isopen;
	int listenfd1;
	int listenfd2;
                int listenfd3; //内网穿透端口
	//退出密码
	string exitpwd;
	//随机数生成器
	Randint rand_of_hisir;
	Randint rand_of_oyasumi;

	UserMap usermap;
	Sqlconnpool* sqlpool;
	//动态创建的使用智能指针管理
	unique_ptr<threadpool> threadp;//动态指定线程个数
	unique_ptr<Epoller> epoller;//动态指定线程个数

};


#endif
