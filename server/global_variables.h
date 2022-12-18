#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/epoll.h>
//这里是不应该改的，包括端口
using namespace std;
extern const unordered_map<string, int> cmdmap;
extern const vector<string> hisir_sentence;
extern const vector<string> oyasumi_sentence;
//事件类型
extern const uint32_t CONNEVENT;
extern const uint32_t LISTENEVENT;
//端口
extern const int port1;
extern const int port2;
extern const int port3; //udp

#endif

