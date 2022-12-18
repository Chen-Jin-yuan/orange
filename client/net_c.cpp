#include "net_c.h"
const char* filelog = "fileLog.txt";//记录收发文件信息
bool isRecvingGf = false;
bool isRecvingSf = false;
const char* NET_SERVER_IP = "101.34.2.129"; 
const int SERVERUDPPORT = 10000;
void initSocket()
{
    //初始化WSA
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;//WSADATA结构体变量的地址值

    //int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
    //成功时会返回0，失败时返回非零的错误代码值
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        std::cerr << "WSAStartup() error!" << std::endl;
        exit(1);
    }
}
SOCKET connect_S(const char* server_ip, const int port)//返回连接句柄
{


    //创建套接字
    SOCKET connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connfd == INVALID_SOCKET)
    {
        //std::cerr << "socket error !" << std::endl;非线程安全
        printf("socket error!\n");
        return INVALID_SOCKET;
    }

    //定义sockaddr_in
    struct sockaddr_in socketaddr;
    socketaddr.sin_family = AF_INET;//ipv4
    socketaddr.sin_port = htons(port);//服务器端口，自己连接后的端口是os分配的，由进程选择一个端口去连服务器
    //socketaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  ///服务器ip
    //inet_addr最好换成inet_aton()，不会冤枉0.0.0.0和255.255.255.255
    struct in_addr inaddr;
    inet_pton(AF_INET, server_ip, &inaddr);//windows下相当于inet_aton的函数，多了第一个参数表明是ipv4还是ipv6
    socketaddr.sin_addr = inaddr;

    ///连接服务器，成功返回0，错误返回-1。返回的描述符connfd，该socket包含了服务器ip、port，自己ip、port，可用于发送和接收数据
    if (connect(connfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == SOCKET_ERROR)
    {
        //std::cout << "connect to target fail! " << std::endl;非线程安全
        printf("connect to target fail!\n");
        return SOCKET_ERROR;
    }
    return connfd;
}




void sendfile(const char* server_ip, const int port, std::string filename)
{
    SOCKET udpfd;
    pair<string, int> gateway = udpPunchedSide(udpfd, NET_SERVER_IP, SERVERUDPPORT, port);
    if (gateway.first == "")
    {
        closesocket(udpfd);
        WSACleanup();
        return;
    }
    const char* gateway_ip = gateway.first.c_str();
    const int gateway_port = gateway.second;

    Sender sender(udpfd,filename.c_str() , gateway_ip, gateway_port);

    sender.start();
        
    sender.sendFile();

    closesocket(udpfd);

}


void recvfile(const int port, std::string filename ,int flag)//这个filename是完整路径
{

    if (flag)//1是getfile
        isRecvingGf = true;
    else
        isRecvingSf = true;

    SOCKET udpfd;
    if (!udpPunchSide(udpfd, NET_SERVER_IP, SERVERUDPPORT, port))
    {
        closesocket(udpfd);
        WSACleanup();
        return;
    }
    Receiver receiver(udpfd, filename.c_str());

    receiver.start();
    receiver.recvFile();
    closesocket(udpfd);

    if (flag)//1是getfile
        isRecvingGf = false;
    else
        isRecvingSf = false;
}
