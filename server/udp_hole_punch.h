#ifndef UDP_HOLE_PUNCHING_H
#define UDP_HOLE_PUNCHING_H
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>//sockaddr_in
#include <arpa/inet.h>//in_addr
#include <cstring>
#include <unistd.h>//close
#include <vector>
#include "log.h"
using namespace std;
namespace UDP_HP
{    
    void init_udp_Socket(int& listenfd, const int port);
    string udp_hole_punching(int listenfd);
    vector<string> parse(string str);
    void work(int& listenudp);
}
#endif