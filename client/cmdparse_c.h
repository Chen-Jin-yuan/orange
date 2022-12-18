#ifndef CMDPARSE_C_H
#define CMDPARSE_C_H
/*
* ��ͷ�ļ�������������صĸ�������
*/
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "state_c.h"
using namespace std;


//constȫ�ֱ���
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


//ȥ��string��β�ո�
void trim(string& s);

//�������cmdstr��λ�ո�ȥ����
vector<string> parse(string cmdstr);

bool isNumber(const char* buf);//�ж��ַ����Ƿ�Ϊ����

bool checkCmd(int cmdvalue,string chatSname);
#endif