#ifndef NET_C_H
#define NET_C_H
/*
* ��ͷ�ļ�������������صĸ�������
* ����
* �����ļ�
* �����ļ�
*/
#include <iostream>
#include <WinSock2.h>//����inet_pton
#include <WS2tcpip.h>//inet_pton��inet_ntop����ȡip��
#include <chrono>
#include <string>

#include "rdt_on_udp.h"
#include "udp_hole_punch.h"
#define buffSize  102400//100KB���������ᱨwarning
//���������Ĭ����16K���ǵ�������ʹ�ö�ջ����ֵ���û���ʹ��Ĭ�϶�ջ�ܴ�СΪ1M��

//�жϽ����ļ��߳��Ƿ����������ļ���ȫ�ֱ���������recvfile����Ϊtrue���κ��˳���Ҫ��Ϊfalse
extern bool isRecvingGf;
extern bool isRecvingSf;

void initSocket();//��ʼ��winsock����������ʹ��һ�μ��ɡ�
SOCKET connect_S(const char* server_ip, int port);//�������Ӿ��
void sendfile(const char* server_ip, const int port, std::string filename);
//0��sendfile��1��getfile
void recvfile(const int port, std::string filename ,int flag);//flag���ڱ�ʾ��getfile����sendfile
#endif