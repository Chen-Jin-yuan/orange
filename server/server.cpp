#include "server.h"

void Server::init_all_Socket()//初始化所有端口，复用单端口代码
{
	init_one_Socket(listenfd1, port1);
	init_one_Socket(listenfd2, port2);//代码复用
                UDP_HP::init_udp_Socket(listenfd3, port3);
}
void Server::init_one_Socket(int& listenfd, const int port)//初始化一个端口
{
    //定义socketfd，它要绑定监听的网卡地址和端口
    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//第三个参数写0也可以，这里表示创建tcp套接字
    if (listenfd < 0)
    {
        LOG_ERROR("create listen socket error, port-%d", port);
        exit(1);
    }
    //定义sockaddr_in
    struct sockaddr_in socketaddr;
    socketaddr.sin_family = AF_INET;//ipv4
    socketaddr.sin_port = htons(port);//字节序转换
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY表示监听所有网卡地址，0.0.0.0；

    //端口复用，在bind前设置，否则bind时出错就晚了
    int optval = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if (ret == -1)
    {
        LOG_ERROR("set socket setsockopt error !");
        close(listenfd);
        exit(1);
    }

    //绑定套接字和地址端口信息，sockaddr_in转成sockaddr
    if (bind(listenfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1)
    {
        LOG_ERROR("bind port-%d error !", port);
        close(listenfd);
        exit(1);
    }
    //开始监听，SOMAXCONN是系统给出的请求队列最大长度
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        LOG_ERROR("listen port-%d error!", port);
        close(listenfd);
        exit(1);
    }
    uint32_t listenEvent = LISTENEVENT;
    if (!epoller->addFd(listenfd, listenEvent))
    {
        LOG_ERROR("add listen to epoll error!");
        close(listenfd);
        exit(1);
    }
    LOG_INFO("server listenning to port-%d", port);
}

void Server::listen_1()//处理端口1连接
{
    while (true)
    {
        struct sockaddr_in client_addr;//获取客户的地址和端口号，连接后的不分配新端口
        socklen_t len = sizeof(client_addr);//socklen_t 相当于 int，但使用int必须强制转型告知编译器
        int conn1 = accept(listenfd1, (struct sockaddr*)&client_addr, &len);
        //成功返回非负数

        if (conn1 == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;//读完了就返回
            else if (errno == ECONNABORTED || errno == EINTR)
                continue;//被中断了或客户端断开了就继续
            else
            {
                LOG_DEBUG("server accept error");
                continue;
            }
        }

        //向内核注册conn1
        uint32_t connEvent = CONNEVENT;
        epoller->addFd(conn1, connEvent);

        string ip = string(inet_ntoa(client_addr.sin_addr));
        usermap.ins_conn1_ip(conn1, ip);//添加映射
        LOG_DEBUG("server accept [1] ip-%s port-%d", ip.c_str(),ntohs(client_addr.sin_port));

    }
}
void Server::listen_2()//处理端口2连接
{
    while (true)
    {
        struct sockaddr_in client_addr;//获取客户的地址和端口号，连接后的不分配新端口
        socklen_t len = sizeof(client_addr);//socklen_t 相当于 int，但使用int必须强制转型告知编译器
        int conn2 = accept(listenfd2, (struct sockaddr*)&client_addr, &len);
        //成功返回非负数
        if (conn2 == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;//读完了就返回
            else if (errno == ECONNABORTED || errno == EINTR)
                continue;//被中断了或客户端断开了就继续
            else
            {
                LOG_DEBUG("server accept error");
                continue;
            }
        }

        //向内核注册conn2，注册是因为可能会有异常事件要处理
        uint32_t connEvent = CONNEVENT;
        epoller->addFd(conn2, connEvent);

        string ip = string(inet_ntoa(client_addr.sin_addr));
        usermap.ins_ip_conn2(ip, conn2);//添加映射
        LOG_DEBUG("server accept [2] ip-%s port-%d", ip.c_str(),ntohs(client_addr.sin_port));

    }
}
void Server::deal_close()//接收关闭信号
{
    char buf[64];
    const string exitstr = "exit -p" + exitpwd + "\n";
    const string quitstr = "quit -p" + exitpwd + "\n";
    while (fgets(buf, sizeof(buf), stdin) != NULL)//gets已不被编译器支持，不太安全
    {
        if (strcmp(buf, exitstr.c_str()) == 0)
            break;
        else if (strcmp(buf, quitstr.c_str()) == 0)
            break;
        else
            cout << "error" << endl;

        bzero(buf, sizeof(buf));
    }
    //close all
    closeServer();
}

//---------------------------------命令处理----------------------------------
//===========================================================================
void Server::task(int conn1)//主要的工作函数
{
    string recvstr = recv_str(conn1);
    if (recvstr == "")//接收出错了
        return;//不给用户发错误信息，可能对方断开了，发过去有危险
    if (recvstr[0] == '~')//聊天内容
    {
        chatmsg(conn1, recvstr);
    }
    else//命令
    {
        LOG_DEBUG("%s",recvstr.c_str());
        vector<string> cmdvec = parse(recvstr);
        //查找命令
        auto iter = cmdmap.find(cmdvec[0]);
        int cmdvalue = iter->second;//客户端处理后命令一定存在

        switch (cmdvalue)
        {
        case 0:
            verify(conn1, 0, cmdvec[1], cmdvec[2]);
            break;
        case 1:
            verify(conn1, 1, cmdvec[1], cmdvec[2]);
            break;
        case 2:
            search(conn1, cmdvec[1]);
            break;
        case 3:
            chat(conn1, cmdvec[1]);
            break;
        case 4:
            accept_(conn1, cmdvec[1]);
            break;
        case 5:
            reject(conn1, cmdvec[1]);
            break;
        case 6:
            break_(conn1);
            break;
        case 7:
            send_(conn1, cmdvec[1], cmdvec[2]);
            break;
        case 8:
            sendfile(conn1, cmdvec[1], cmdvec[2]);
            break;
        case 9:
            acceptfile(conn1, cmdvec[1]);
            break;
        case 10:
            rejectfile(conn1, cmdvec[1]);
            break;
        case 11:
            getfile(conn1, cmdvec[1]);
            break;
        case 12:
            acceptget(conn1, cmdvec[1], cmdvec[2]);
            break;
        case 13:
            rejectget(conn1, cmdvec[1]);
            break;
        case 14:
            setsid(conn1, cmdvec[1]);
            break;
        case 15:
            setsname(conn1, cmdvec[1]);
            break;
        case 16:
            choosefile(conn1, cmdvec[1]);
            break;
        case 17:
            //exit_(conn1);
            break;
        case 18:
            hisir(conn1, cmdvec[1]);
            break;
        case 20:
            oyasumi(conn1);
            break;
        }
    }
    uint32_t connEvent = CONNEVENT;//因为是oneshot，每次重新注册
    epoller->modFd(conn1, connEvent);
}
vector<string> Server::parse(string cmdstr)//解析命令函数
{
    if (cmdstr[0] == '@')//首先看第一个字符是不是@，是的话去掉就好了
        cmdstr = cmdstr.substr(1, cmdstr.size() - 1);
    //对于一个关键字的命令，无法用空格分割，考虑到最后一定有个\n是没用的，因此把\n改为空格，一举两得
    cmdstr[cmdstr.size() - 1] = ' ';
    vector<string> res;
    size_t pos = 0;
    size_t pos1;
    while ((pos1 = cmdstr.find(' ', pos)) != string::npos)
    {
        res.push_back(cmdstr.substr(pos, pos1 - pos));
        while (cmdstr[pos1] == ' ')//过滤空格
            pos1++;
        pos = pos1;
    }
    return res;//返回值是右值，外部vector会接收右值，调用移动构造
}
string Server::recv_str(int conn1)//接收客户端信息函数
{
    char recvbuf[256];
    string recvstr;
    while (true)
    {
        memset(recvbuf, 0, sizeof(recvbuf));//把接收缓冲清零
        int len = recv(conn1, recvbuf, sizeof(recvbuf), 0);
        if (len == 0)//copy出错或断开
        {
            string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
            LOG_ERROR("%s [recv] error!", myuser.c_str());
            uint32_t connEvent = CONNEVENT;//因为是oneshot，每次重新注册
            epoller->modFd(conn1, connEvent);//重新设置，等下一次
            return "";//这次就不做了
        }
        else if (len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)//说明读完了，两个errno是一样的意思
            {
                uint32_t connEvent = CONNEVENT;//因为是oneshot，每次重新注册
                epoller->modFd(conn1, connEvent);
                break;//正常结束
            }
            else if (errno == EINTR)//被中断了，重新读
                continue;
            else //其他错误
            {
                string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
                LOG_ERROR("%s [recv] error!", myuser.c_str());
                uint32_t connEvent = CONNEVENT;//因为是oneshot，每次重新注册
                epoller->modFd(conn1, connEvent);//重新设置，等下一次
                return "";//这次就不做了
            }
        }
        else//接收到了信息
        {
            recvstr += string(recvbuf);
        }
    }
    return recvstr;
}
bool Server::msg_log(string userid, string& msglog)//记录聊天日志
{
    string filename = "user/"+userid + ".txt";
    FILE* fp = fopen(filename.c_str(), "a");//不用互斥，同时间只有一个线程操作
    if (fp == nullptr)
        return false;
    //初始化时间
    struct tm* nowtime;
    time_t t;
    t = time(NULL);
    nowtime = localtime(&t);
    char timebuffer[64];//时间头
    //添加时间信息
    snprintf(timebuffer, 64, "%d-%02d-%02d_%02d:%02d:%02d:\t",
        nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday,
        nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec);//只精确到秒，更具体的信息交给内容体现
    string allmsg = string(timebuffer) + msglog + "\n";
    fputs(allmsg.c_str(), fp);
    fclose(fp);
    return true;
}
void Server::chatmsg(int conn1, string& recvstr)//处理聊天信息
{
    int conn2 = usermap.fvalue_conn1_peer2(conn1);//获取对方接收套接字
    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    if (conn2 == -1)
    {
        LOG_ERROR("%s chatmsg error, can't find peer!", myuser.c_str());
        return;
    }
    string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid

    string mysname = usermap.fvalue_conn1_sname(conn1);//获取发送方名字

    LOG_INFO("%s send [msg] to %s", myuser.c_str(), peeruser.c_str());
    //还要记录myuser的聊天日志
    recvstr = recvstr.substr(1, recvstr.size() - 2);//去~和去\n
    recvstr = mysname + ": " + recvstr;
    send(conn2, recvstr.c_str(), recvstr.size(), 0);

    //记录日志，在下一篇博客中已完成该函数
    //前面的recvstr已经记录了发送方名字和发送内容，然后文件名包含了发送者id，因此再补上接收者id即可
    recvstr = recvstr + " [" + peeruser + "]";
    if (!msg_log(myuser, recvstr))
        LOG_ERROR("%s open log falied!", myuser.c_str());
}
void Server::verify(int conn1, int isLogin, string userid, string password)//登录注册管理
{
    MYSQL* sql;
    SqlRAII myconn(&sql, sqlpool, 0);//获取一个sql连接句柄，0表示超时时间
    string sendstr = "Server busy!";//默认

    if (sql == nullptr)//如果没拿到，即超时了
    {
        send(conn1, sendstr.c_str(), int(sendstr.size()), 0);
        return;
    }
    else
    {
        MYSQL_RES* res = nullptr;
        string order = "SELECT userid, password FROM user WHERE userid='" + userid + "'";//命令不用加分号

        //要么没有，要么只能查出一个
        if (mysql_query(sql, order.c_str())) //执行语句，成功返回0，错误返回非0
        {
            LOG_DEBUG("Select error: %s", order.c_str());
            mysql_free_result(res);//错误的话释放结果集（无论成功与否，查询后总是释放）
            send(conn1, sendstr.c_str(), int(sendstr.size()), 0);//发送server busy
            return;
        }
        res = mysql_store_result(sql);//完整的结果集

        while (MYSQL_ROW row = mysql_fetch_row(res)) //遍历行，没有对应的表项就进不来，进来就只有一行
        {
            string passwd(row[1]);
            // 能select到说明又对应的username，看是登录还是注册
            if (isLogin)//登录
            {
                if (password == passwd) //登录成功
                {
                    sendstr = "success";
                    send(conn1, sendstr.c_str(), int(sendstr.size()), 0);

                    string ip = usermap.fvalue_conn1_ip(conn1);
                    LOG_INFO("%s login successfully with password: %s ip: %s", userid.c_str(), password.c_str(), ip.c_str());
                    mysql_free_result(res);//获取行完毕，释放结果集使用的内存

                    //添加映射
                    login_addmap(conn1, userid);

                    return;
                }
                else //登录失败，这种情况是密码错误
                {
                    sendstr = "password error!";
                    send(conn1, sendstr.c_str(), int(sendstr.size()), 0);

                    string ip = usermap.fvalue_conn1_ip(conn1);
                    LOG_INFO("%s login unsuccessfully with password: %s ip: %s", userid.c_str(), password.c_str(), ip.c_str());
                    mysql_free_result(res);//获取行完毕，释放结果集使用的内存
                    return;
                }
            }

            else//注册，说明userid被使用了
            {
                sendstr = "the user id [" + userid + "] is already in use";
                send(conn1, sendstr.c_str(), int(sendstr.size()), 0);

                LOG_INFO("register unsuccessfully, [%s] is already in use", userid.c_str());
                mysql_free_result(res);//获取行完毕，释放结果集使用的内存
                return;
            }
        }

        mysql_free_result(res);//释放结果集使用的内存

        //现在处理没有查找到的情况
        if (isLogin)//登录
        {
            sendstr = "user id [" + userid + "] not found";
            send(conn1, sendstr.c_str(), int(sendstr.size()), 0);

            LOG_INFO("%s login but id not found", userid.c_str());
            return;
        }
        else//注册，这种情况可以注册
        {
            string order_insert = "INSERT INTO user(userid, password) VALUES('" + userid + "','" + password + "')";
                if (mysql_query(sql, order_insert.c_str()))
                {
                    LOG_DEBUG("Insert error: %s", order_insert.c_str());
                    send(conn1, sendstr.c_str(), int(sendstr.size()), 0);//发送server busy
                    return;
                }
                else
                {
                    sendstr = "success";
                    send(conn1, sendstr.c_str(), int(sendstr.size()), 0);

                    string ip = usermap.fvalue_conn1_ip(conn1);
                    LOG_INFO("register successfully with id: [%s] password: [%s] ip:%s", userid.c_str(), password.c_str(), ip.c_str());
                    return;
                }
        }
    }
}
void Server::login_addmap(int conn1, string userid)//登录时添加一系列映射
{
    string ip = usermap.fvalue_conn1_ip(conn1);
    int conn2 = usermap.fvalue_ip_conn2(ip);

    usermap.ins_conn1_2(conn1, conn2);
    usermap.ins_conn1_sid(conn1, userid);
    usermap.ins_conn1_sname(conn1, userid);

    usermap.ins_sid_conn2(userid, conn2);
    usermap.ins_sid_sname(userid, userid);
    usermap.ins_sid_state(userid, clientState::cmdLine);

    usermap.ins_conn1_state(conn1, clientState::cmdLine);
    usermap.ins_conn2_user(conn2, userid);
    usermap.ins_conn1_user(conn1, userid);
}
void Server::search(int conn1, string sid)
{
    string sendstr;
    string sname = usermap.fvalue_sid_sname(sid);
    string myuser = usermap.fvalue_conn1_user(conn1);//获取userid
    LOG_INFO("%s [search] sid-%s", myuser.c_str(), sid.c_str());
    if (sname == "")
    {
        sendstr = "The peer does not exist!";
        send(conn1, sendstr.c_str(), sendstr.size(), 0);
        return;
    }
    clientState state = usermap.fvalue_sid_state(sid);
    if (state == clientState::isChatting)
        sendstr = "sid-[" + sid + "] sname-[" + sname + "] state--is chatting";
    else
        sendstr = "sid-[" + sid + "] sname-[" + sname + "] state--isn't chatting";
    send(conn1, sendstr.c_str(), sendstr.size(), 0);
}
void Server::chat(int conn1, string sid)
{
    string sendstr;
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字
    int myconn2 = usermap.fvalue_conn1_2(conn1);//获取自己的接收套接字
    //获取自己信息
    string mysid = usermap.fvalue_conn1_sid(conn1);
    string mysname = usermap.fvalue_conn1_sname(conn1);
    usermap.ins_conn1_state(conn1, clientState::isWaiting);//修改自己状态为waiting
    usermap.ins_sid_state(mysid, clientState::isWaiting);//改两次

    if (conn2 == -1)
    {
        sendstr = "The peer does not exist, please break!";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);
        return;
    }
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    if (state == clientState::isChatting)
    {
        sendstr = "The peer is chatting, you may wait for a long time...";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);
    }

    sendstr = "@#chatfrom " + mysid + " " + mysname;
    send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方
    usermap.ins_conn1_req_peer2(conn1, conn2);//添加一个请求映射

    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
    LOG_INFO("%s send a [chat] requestion to %s", myuser.c_str(), peeruser.c_str());
}
void Server::accept_(int conn1, string sid)
{
    string sendstr;
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字
    int myconn2 = usermap.fvalue_conn1_2(conn1);
    string mysname = usermap.fvalue_conn1_sname(conn1);
    if (state == clientState::isWaiting)
    {
        sendstr = "@#chat accept "+ mysname;
        send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方

        string mysid = usermap.fvalue_conn1_sid(conn1);//获取自己的sid，为了修改状态
        usermap.ins_conn1_state(conn1, clientState::isChatting);//修改自己状态为chatting
        usermap.ins_sid_state(mysid, clientState::isChatting);//改两次

        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        usermap.ins_conn1_state(peerconn1, clientState::isChatting);//修改对方状态为chatting
        usermap.ins_sid_state(sid, clientState::isChatting);//改两次

        usermap.ins_conn1_peer2(conn1, conn2);
        usermap.ins_conn1_peer2(peerconn1, myconn2);

        //accept不删，为了在break到达前accept后还能找到对方，reject可以删，因为不用找对方
        //usermap.del_conn1_req_peer2(peerconn1);//把服务器保存的对方的请求删掉

        string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
        string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
        LOG_INFO("%s [accept] the chat requestion for %s", myuser.c_str(), peeruser.c_str());
    }
    else//对方已经退出了，或在accept到服务器前break了，或其他情况，让accept方退出
    {
        sendstr = "@#break now";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);//发给自己
    }
}
void Server::reject(int conn1, string sid)
{
    string sendstr;
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字

    if (state == clientState::isWaiting)
    {
        sendstr = "@#chat reject";
        send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方

        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        usermap.ins_conn1_state(peerconn1, clientState::cmdLine);//修改对方状态为cmdLine
        usermap.ins_sid_state(sid, clientState::cmdLine);//改两次

        usermap.del_conn1_req_peer2(peerconn1);//把服务器保存的对方的请求删掉

        string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
        string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
        LOG_INFO("%s [reject] the chat requestion for %s", myuser.c_str(), peeruser.c_str());
    }
    //其他情况下对于reject不用管
}
void Server::break_(int conn1)//加下划线为了避免和break关键字重合，这里不close，在epoll里close
{
    string sendstr;
    clientState state = usermap.fvalue_conn1_state(conn1);//获取自己状态
    string mysid = usermap.fvalue_conn1_sid(conn1);//获取自己的sid，为了修改状态

    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    int conn2 = usermap.fvalue_conn1_req_peer2(conn1);
    LOG_DEBUG("%s is ready to [break]", myuser.c_str());
    if (state == clientState::isWaiting)
    {
        usermap.ins_conn1_state(conn1, clientState::cmdLine);//修改自己状态为cmdLine
        usermap.ins_sid_state(mysid, clientState::cmdLine);//改两次

        if (conn2 == -1)//没有请求就不用管，否则要删除对方的请求表
            return;
        usermap.del_conn1_req_peer2(conn1);//删请求
        sendstr = "@#break " + mysid;
        send(conn2, sendstr.c_str(), sendstr.size(), 0);
        LOG_INFO("%s [break] from waiting", myuser.c_str());
    }
    else if (state == clientState::cmdLine)//在break到达前acceptfile了，让对方退出
    {
        if (conn2 == -1)//没有请求就不用管，reject会把请求删掉，这与accept区分开来
            return;
        usermap.del_conn1_req_peer2(conn1);//删请求
        sendstr = "@#break now";
        send(conn2, sendstr.c_str(), sendstr.size(), 0);
        LOG_INFO("%s [break] from cmdLine", myuser.c_str());
    }
    else if (state == clientState::isChatting)//正在通信，或在break到达前accept了
    {
        conn2 = usermap.fvalue_conn1_peer2(conn1);//找到正在通信的对方

        string mysid = usermap.fvalue_conn1_sid(conn1);//获取自己的sid，为了修改状态
        usermap.ins_conn1_state(conn1, clientState::cmdLine);//修改自己状态为cmdLine
        usermap.ins_sid_state(mysid, clientState::cmdLine);//改两次
        LOG_INFO("%s [break] from chatting", myuser.c_str());

        if (conn2 == -1)//一般不会，可能有没考虑到的情况吧
            return;
        //让对方也退出
        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        string sid = usermap.fvalue_conn1_sid(peerconn1);//获取对方sid，修改状态
        usermap.ins_conn1_state(peerconn1, clientState::cmdLine);//修改对方状态为cmdLine
        usermap.ins_sid_state(sid, clientState::cmdLine);//改两次

        //删除通信的映射
        usermap.del_conn1_peer2(conn1);
        usermap.del_conn1_peer2(peerconn1);

        sendstr = "@#break now";
        send(conn2, sendstr.c_str(), sendstr.size(), 0);
    }
}
void Server::send_(int conn1, string sid, string& msg)
{
    string sendstr;
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字
    if (conn2 == -1)
        return;
    //获取自己信息
    string mysid = usermap.fvalue_conn1_sid(conn1);
    string mysname = usermap.fvalue_conn1_sname(conn1);

    sendstr = "sid-[" + mysid + "] sname-[" + mysname + "] send: " + msg;
    send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方

    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
    LOG_INFO("%s [send] message to %s: %s", myuser.c_str(), peeruser.c_str(), msg.c_str());
}
void Server::sendfile(int conn1, string sid, string filename)
{
    size_t pos = filename.find_last_of("/\\");//发过来可能是完整路径，要把路径删掉
    if (pos != string::npos)
        filename = filename.substr(pos + 1);
    string sendstr;
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字
    int myconn2 = usermap.fvalue_conn1_2(conn1);//获取自己的接收套接字
    //获取自己信息
    string mysid = usermap.fvalue_conn1_sid(conn1);
    string mysname = usermap.fvalue_conn1_sname(conn1);
    usermap.ins_conn1_state(conn1, clientState::isWaiting);//修改自己状态为waiting
    usermap.ins_sid_state(mysid, clientState::isWaiting);//改两次

    if (conn2 == -1)
    {
        sendstr = "The peer does not exist, please break!";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);
        return;
    }
    if(filename == "")
    {
        sendstr = "filename error, please break and check!";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);
        return;
    }
    sendstr = "@#sendfilefrom " + mysid + " " + mysname + " " + filename;
    send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方
    usermap.ins_conn1_req_peer2(conn1, conn2);//添加一个请求映射

    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
    LOG_INFO("%s send a [sendfile] requestion to %s, filename-%s", myuser.c_str(), peeruser.c_str(),filename.c_str());
}
void Server::acceptfile(int conn1, string sid)
{
    string sendstr;
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字
    int myconn2 = usermap.fvalue_conn1_2(conn1);
    if (state == clientState::isWaiting)
    {
        string myip = usermap.fvalue_conn1_ip(conn1);
        sendstr = "@#sendfile accept " + myip;

        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        usermap.ins_conn1_state(peerconn1, clientState::cmdLine);//修改对方状态为cmdLine
        usermap.ins_sid_state(sid, clientState::cmdLine);//改两次


        //accept不删，为了在break到达前accept后还能找到对方，reject可以删，因为不用找对方
        //usermap.del_conn1_req_peer2(peerconn1);//把服务器保存的对方的请求删掉

        string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
        string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
        LOG_INFO("%s [acceptfile] from %s", myuser.c_str(), peeruser.c_str());

        send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方
        UDP_HP::work(listenfd3);
    }
    else//对方已经退出了，或在accept到服务器前break了，或其他情况，让accept方退出
    {
        sendstr = "@#break now";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);//发给自己
    }
}
void Server::rejectfile(int conn1, string sid)
{
    string sendstr;
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字

    if (state == clientState::isWaiting)
    {
        sendstr = "@#sendfile reject";
        send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方

        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        usermap.ins_conn1_state(peerconn1, clientState::cmdLine);//修改对方状态为cmdLine
        usermap.ins_sid_state(sid, clientState::cmdLine);//改两次

        usermap.del_conn1_req_peer2(peerconn1);//把服务器保存的对方的请求删掉

        string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
        string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
        LOG_INFO("%s  [rejectfile] from %s", myuser.c_str(), peeruser.c_str());
    }
    //其他情况下对于reject不用管
}
void Server::getfile(int conn1, string sid)
{
    string sendstr;
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字
    int myconn2 = usermap.fvalue_conn1_2(conn1);//获取自己的接收套接字
    //获取自己信息
    string mysid = usermap.fvalue_conn1_sid(conn1);
    string mysname = usermap.fvalue_conn1_sname(conn1);
    usermap.ins_conn1_state(conn1, clientState::isWaiting);//修改自己状态为waiting
    usermap.ins_sid_state(mysid, clientState::isWaiting);//改两次

    if (conn2 == -1)
    {
        sendstr = "The peer does not exist, please break!";
        send(myconn2, sendstr.c_str(), sendstr.size(), 0);
        return;
    }
    sendstr = "@#getfilefrom " + mysid + " " + mysname;
    send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方
    usermap.ins_conn1_req_peer2(conn1, conn2);//添加一个请求映射

    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
    LOG_INFO("%s send a [getfile] requestion to %s", myuser.c_str(), peeruser.c_str());
}
void Server::acceptget(int conn1, string sid, string& src)
{
    string sendstr;
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字

    if (state == clientState::isWaiting)
    {
        sendstr = "@#getfile accept " + src;
        send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方


        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        usermap.ins_conn1_state(peerconn1, clientState::cmdLine);//修改对方状态为cmdLine
        usermap.ins_sid_state(sid, clientState::cmdLine);//改两次


        //accept不删，为了在break到达前accept后还能找到对方，reject可以删，因为不用找对方
        //usermap.del_conn1_req_peer2(peerconn1);//把服务器保存的对方的请求删掉

        string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
        string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
        LOG_INFO("%s [acceptget] from %s", myuser.c_str(), peeruser.c_str());
    }
    //其他情况下对于getfile不用管
}
void Server::rejectget(int conn1, string sid)
{
    string sendstr;
    clientState state = usermap.fvalue_sid_state(sid);//获取对方状态
    int conn2 = usermap.fvalue_sid_conn2(sid);//获取对方接收套接字

    if (state == clientState::isWaiting)
    {
        sendstr = "@#getfile reject";
        send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方

        int peerconn1 = usermap.fkey_conn1_2(conn2);//获取对方套接字1，为了修改状态
        usermap.ins_conn1_state(peerconn1, clientState::cmdLine);//修改对方状态为cmdLine
        usermap.ins_sid_state(sid, clientState::cmdLine);//改两次

        usermap.del_conn1_req_peer2(peerconn1);//把服务器保存的对方的请求删掉

        string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
        string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
        LOG_INFO("%s [rejectget] from %s", myuser.c_str(), peeruser.c_str());
    }
    //其他情况下对于reject不用管


}
void Server::choosefile(int conn1, string number)//服务器没有re命令，但是多了一个choosefile，通告选择的文件号码
{
    string sendstr;
    int conn2 = usermap.fvalue_conn1_req_peer2(conn1);//从getfile请求里获取对方的套接字
    if (conn2 == -1)//意外情况...
    {
        LOG_ERROR("[choosefile] can't find the file-sender")
            return;
    }

    string myip = usermap.fvalue_conn1_ip(conn1);//告知对方（发送方）本地ip地址
    sendstr = "@#choosefile " + number + " " + myip;
    

    string myuser = usermap.fvalue_conn1_user(conn1);//获取发送方userid
    string peeruser = usermap.fvalue_conn2_user(conn2);//获取对方userid
    LOG_INFO("%s [choosefile] from %s", myuser.c_str(), peeruser.c_str());

    send(conn2, sendstr.c_str(), sendstr.size(), 0);//发给对方
    UDP_HP::work(listenfd3);
}
void Server::setsid(int conn1, string& newsid)
{
    string oldsid = usermap.fvalue_conn1_sid(conn1);//先获取旧的sid
    string sendstr;
    if (oldsid == newsid)
    {
        sendstr = "Your old sid and new sid are the same";
        send(conn1, sendstr.c_str(), sendstr.size(), 0);//发给自己
        return;
    }

    int check = usermap.fvalue_sid_conn2(newsid);//查看是否已有该sid
    if (check != -1)//该sid已注册
    {
        sendstr = "The newsid [" + newsid + "] has been used, setsid failed";
        send(conn1, sendstr.c_str(), sendstr.size(), 0);//发给自己
        return;
    }

    //有些表sid是当key的，这里的修改原则是删除->重新插入
    string sname = usermap.fvalue_conn1_sname(conn1);//获取sname，为了改sid_sname表
    clientState state = usermap.fvalue_sid_state(oldsid);//获取state，为了改sid_state表
    int conn2 = usermap.fvalue_sid_conn2(oldsid);//获取conn2，为了改sid_conn2表

    usermap.ins_conn1_sid(conn1, newsid);//修改

    usermap.del_sid_sname(oldsid);//删
    usermap.ins_sid_sname(newsid, sname);//插入
    usermap.del_sid_state(oldsid);//删
    usermap.ins_sid_state(newsid, state);//插入
    usermap.del_sid_conn2(oldsid);//删
    usermap.ins_sid_conn2(newsid, conn2);//插入

    sendstr = "setsid successfullly, new sid is [" + newsid + "]";
    send(conn1, sendstr.c_str(), sendstr.size(), 0);//发给自己

    string myuser = usermap.fvalue_conn1_user(conn1);//获取userid
    LOG_INFO("%s [setsid] from %s to [%s]", myuser.c_str(), oldsid.c_str(), newsid.c_str());
}
void Server::setsname(int conn1, string& newsname)
{
    string oldsname = usermap.fvalue_conn1_sname(conn1);//先获取旧的sname
    string sendstr;
    if (oldsname == newsname)
    {
        sendstr = "Your old sname and new sname are the same";
        send(conn1, sendstr.c_str(), sendstr.size(), 0);//发给自己
        return;
    }
    string sid = usermap.fvalue_conn1_sid(conn1);

    //修改
    usermap.ins_conn1_sname(conn1, newsname);
    usermap.ins_sid_sname(sid, newsname);

    sendstr = "setsname successfullly, new sname is [" + newsname + "]";
    send(conn1, sendstr.c_str(), sendstr.size(), 0);//发给自己

    string myuser = usermap.fvalue_conn1_user(conn1);//获取userid
    LOG_INFO("%s [setsname] from %s to [%s]", myuser.c_str(), oldsname.c_str(), newsname.c_str());
}
void Server::exit_(int conn1)//退出相当于break+clean map+close
{
    break_(conn1);//调用break，让会影响到的对方退出
    //break是没有关系的，因为如果普通情况下exit，请求表映射是空，那么break不会做事情
    string myuser = usermap.fvalue_conn1_user(conn1);//获取userid
    if(myuser == "")
        myuser = "[noLogin]";
    LOG_INFO("%s [exit]", myuser.c_str());//删之前记录一下
    
    cleanmap(conn1);//删表
    epoller->delFd(conn1);//从epoll内删除事件
    close(conn1);//关套接字
    LOG_DEBUG("close connfd [1] %d",conn1);
}
void Server::cleanmap(int conn1)//删表，oyasumi和exit都可以使用
{
    string myip = usermap.fvalue_conn1_ip(conn1);
    string mysid = usermap.fvalue_conn1_sid(conn1);
    int conn2 = usermap.fvalue_conn1_2(conn1);

    //13个表都删了
    usermap.del_conn1_ip(conn1);
    usermap.del_ip_conn2(myip);
    usermap.del_conn1_2(conn1);
    usermap.del_conn1_sid(conn1);
    usermap.del_conn1_sname(conn1);
    usermap.del_sid_conn2(mysid);
    usermap.del_sid_sname(mysid);
    usermap.del_sid_state(mysid);
    usermap.del_conn1_state(conn1);
    usermap.del_conn1_peer2(conn1);
    usermap.del_conn2_user(conn2);
    usermap.del_conn1_user(conn1);
    usermap.del_conn1_req_peer2(conn1);
}
void Server::hisir(int conn1, string& msg)
{
    string sendstr = hisir_sentence[rand_of_hisir()];
    send(conn1, sendstr.c_str(), sendstr.size(), 0);

    string myuser = usermap.fvalue_conn1_user(conn1);//获取userid
    LOG_INFO("%s [hisir]: %s", myuser.c_str(), msg.c_str());
}
void Server::oyasumi(int conn1)
{
    string sendstr = oyasumi_sentence[rand_of_oyasumi()];
    send(conn1, sendstr.c_str(), sendstr.size(), 0);

    break_(conn1);//调用break，让会影响到的对方退出

    string myuser = usermap.fvalue_conn1_user(conn1);//获取userid
    LOG_INFO("%s [oyasumi]~", myuser.c_str());//删之前记录一下

    cleanmap(conn1);//删表
    close(conn1);//关套接字
}

Server::Server(int sqlPort, const char* sqlUser, const  char* sqlPwd, const char* dbName, int connPoolNum,
    int logLevel ,const char* logPath, int logQueSize, int logThreadNum,
    int taskThreadNum, string exitpwd_, int timeout_, int max_events):
    timeout(timeout_), isopen(true), exitpwd(exitpwd_),
    rand_of_hisir(0,hisir_sentence.size()-1),rand_of_oyasumi(0,oyasumi_sentence.size()-1),
    threadp(new threadpool(taskThreadNum)),epoller(new Epoller(max_events))
{
    sqlpool = Sqlconnpool::instance();
    sqlpool->init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    Log::instance()->init(logLevel, logPath, logQueSize ,logThreadNum);
}

void Server::start()
{
    LOG_INFO("=====================Server start=====================");
    init_all_Socket();
    threadp->addTask(std::bind(&Server::deal_close, this));
    
    while (isopen)//如果wait是阻塞的就无法退出
    {
        int eventCnt = epoller->wait(timeout);
        for (int i = 0; i < eventCnt; i++)
        {
            int fd = epoller->getEventFd(i);
            uint32_t events = epoller->getEvents(i);
            //处理epoll出错和对端关闭情况
            if (fd == listenfd1)//注意有两个listen端口
            {
                listen_1();//把所有connect的都accept，ET模式要循环到底
            }
            else if (fd == listenfd2)
            {
                listen_2();
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))//异常事件
            {//一般来讲，某个异常是客户端出问题，这样两个connfd都应该会异常，然后都关闭，
                    //所以不用根据一个把另一个同时关了，关这次的事件即可
                /*
                * 帮这个用户调exit_，删除表和让影响的对方退出，exit内会close套接字并删除事件
                * 这样用户直接exit_后，因为exit_内close了，就不会进入这里再exit_重复调用了
                * 而如果用户没有exit_直接关闭或崩溃，就帮忙exit_，总之就是只调用一次
                */
                
                if (usermap.fvalue_conn1_ip(fd) != "")//如果是conn1
                {
                    LOG_DEBUG("close connfd [1] %d",fd);
                    exit_(fd);//nologin的话break_直接不会做什么事情
                }
                    
                else//conn2
                {
                    epoller->delFd(fd);//从epoll内删除事件
                    close(fd);//conn2直接close
                    LOG_DEBUG("close connfd [2] %d",fd);
                }

                //其他处理
            }
            else if (events & EPOLLIN)//处理读事件
            {
                /*
                * std::bind()绑定类成员函数时，需要在类名前面添加取地址符，让其拥有callable的属性。
                * 这和作用域有关，只有加了类名才是“quilified-id”，即受作用域限制，这样才能正确寻找
                * 类成员函数不像普通函数一样可以隐式转换成函数指针，所以需要手动加&，普通函数可以隐式转换
                * 别忘了传入this指针
                */
                threadp->addTask(std::bind(&Server::task, this,fd));//添加任务，该任务处理读与写
            }
        }
        //其他处理
    }
}
void Server::closeServer()
{
    LOG_INFO("=====================Server close=====================");
    
    //关闭端口
    close(listenfd1);
    close(listenfd2);
    close(listenfd3);

    //关线程池，通知任务完成后退出
    threadp->close();//只通知，不阻塞
    cout << "threadpool close" << endl;
    sqlpool->close();//等待所有连接放回，因为要逐个关闭连接，会阻塞
    cout << "sql pool close" << endl;
    Log::instance()->close();//日志一般最后关闭，会阻塞等阻塞队列的内容写完
    cout << "log system close" << endl;
    isopen = false;
}
