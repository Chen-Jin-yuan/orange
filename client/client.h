#ifndef CLIENT_H
#define CLIENT_H
#include<iostream>
#include <fstream>//�ļ���
#include<cstring>//string.h
#include<string>
#include<unordered_map>
#include <thread>
#include <mutex>
#include<functional>
#include<WinSock2.h>//����inet_pton
#include <WS2tcpip.h>//inet_pton
#include <windows.h>//��ȡ��Դ��Ϣ
#include <direct.h>//�����ļ��� 

#include "net_c.h"
#include "cmdparse_c.h"
#include "state_c.h"
#include "reqtable.h"
#include "auxfunc.h"

using namespace std;
extern clientState state;//����ȫ�ֱ�������cpp�ļ��ﶨ��
extern bool isRecvingGf;
extern bool isRecvingSf;

class Client
{
private:
    
    string chatSname;//����chatting�ĶԷ���sname
    int fileNumber;//���ڶԷ�acceptget��ѡ���ļ����������
    unordered_map<int, string> getResourceMap;//��öԷ�����Դ��
    string prompt;//��ʾ��

    string sendfilename;//Ҫ���͵�����·�����ļ������ӽ���sendfile�����ȡ�����Է�����ʱ���ݸ��ļ��������ļ�
                        //Ŀǰ����֧������sendfile
    bool chooseflag;
    bool exitflag;//�����˳��źţ������̹߳���
    unordered_map<clientState, string> promptMap;//state����ʾ����ӳ��
    ReqTable reqTable;//�����

    //�����кܶ����ã���������һ������ӳ��
    unordered_map<string, string> config_map;//ӳ���
    unordered_map<int, string> myresource_map;
    string myresource_str;//��Դ�б�����ֱ�ӷ���

    const int sendfilePort;//sendfile�����Ӧ�Ľ��ն˿�
    const int getfilePort;//getfile�����Ӧ�Ľ��ն˿�

public:
    Client();
    ~Client();
    void run(SOCKET connfd);
    bool parseRecv(string cmdrecv);//��������������ؽ����ɲ��ɹ���ʧ����false
    void recvThread(const char* server_ip, const int port);
    void start(const char* server_ip, const int port1, const int port2);//���յ���
    void setConfig();//��������
    void genResource(string dirpath);//���÷��͵���Դ�б�myresource_str �Լ�ӳ���myresoure_map
    void autoLogin(SOCKET connfd);//�Զ���¼
};
#endif
