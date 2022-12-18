#include "net_c.h"
const char* filelog = "fileLog.txt";//��¼�շ��ļ���Ϣ
bool isRecvingGf = false;
bool isRecvingSf = false;
const char* NET_SERVER_IP = "101.34.2.129"; 
const int SERVERUDPPORT = 10000;
void initSocket()
{
    //��ʼ��WSA
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;//WSADATA�ṹ������ĵ�ֵַ

    //int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
    //�ɹ�ʱ�᷵��0��ʧ��ʱ���ط���Ĵ������ֵ
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        std::cerr << "WSAStartup() error!" << std::endl;
        exit(1);
    }
}
SOCKET connect_S(const char* server_ip, const int port)//�������Ӿ��
{


    //�����׽���
    SOCKET connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connfd == INVALID_SOCKET)
    {
        //std::cerr << "socket error !" << std::endl;���̰߳�ȫ
        printf("socket error!\n");
        return INVALID_SOCKET;
    }

    //����sockaddr_in
    struct sockaddr_in socketaddr;
    socketaddr.sin_family = AF_INET;//ipv4
    socketaddr.sin_port = htons(port);//�������˿ڣ��Լ����Ӻ�Ķ˿���os����ģ��ɽ���ѡ��һ���˿�ȥ��������
    //socketaddr.sin_addr.s_addr = inet_addr(SERVER_IP);  ///������ip
    //inet_addr��û���inet_aton()������ԩ��0.0.0.0��255.255.255.255
    struct in_addr inaddr;
    inet_pton(AF_INET, server_ip, &inaddr);//windows���൱��inet_aton�ĺ��������˵�һ������������ipv4����ipv6
    socketaddr.sin_addr = inaddr;

    ///���ӷ��������ɹ�����0�����󷵻�-1�����ص�������connfd����socket�����˷�����ip��port���Լ�ip��port�������ڷ��ͺͽ�������
    if (connect(connfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == SOCKET_ERROR)
    {
        //std::cout << "connect to target fail! " << std::endl;���̰߳�ȫ
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


void recvfile(const int port, std::string filename ,int flag)//���filename������·��
{

    if (flag)//1��getfile
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

    if (flag)//1��getfile
        isRecvingGf = false;
    else
        isRecvingSf = false;
}
