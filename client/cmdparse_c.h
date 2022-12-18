#ifndef CMDPARSE_C_H
#define CMDPARSE_C_H
/*
* 该头文件管理与命令相关的辅助函数
*/
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "state_c.h"
using namespace std;


//const全局变量
const unordered_map<string, int> cmdmap =
{
    {"register",0},
    {"login",1},
    {"search",2},
    {"chat",3},
    {"accept",4},
    {"reject",5},
    {"break",6},
    {"send",7},
    {"sendfile",8},
    {"acceptfile",9},
    {"rejectfile",10},
    {"getfile",11},
    {"acceptget",12},
    {"rejectget",13},
    {"setsid",14},
    {"setsname",15},
    {"re",16},
    {"exit",17},
    {"hisir",18},
    {"myresource",19},
    {"oyasumi",20}
};


//去掉string首尾空格
void trim(string& s);

//解析命令，cmdstr首位空格都去掉了
vector<string> parse(string cmdstr);

bool isNumber(const char* buf);//判断字符传是否为数字

bool checkCmd(int cmdvalue,string chatSname);
#endif