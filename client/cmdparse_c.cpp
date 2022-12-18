#include "cmdparse_c.h"
extern clientState state;

//去掉string首尾空格
void trim(string& s)
{
    if (!s.empty())
    {
        s.erase(0, s.find_first_not_of(" \t"));//首次出现不匹配空格的位置
        s.erase(s.find_last_not_of(" \t") + 1);
    }
}

//解析命令，cmdstr首位空格都去掉了
vector<string> parse(string cmdstr)
{
    //首先看第一个字符是不是@，是的话去掉就好了
    if (cmdstr[0] == '@')
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

bool isNumber(const char* buf)//判断字符传是否为数字
{
    for (int i = 0; buf[i] != '\n'; i++)//以\n结尾
        if (buf[i] < '0' || buf[i]>'9')//每个字符都要是数字
            return false;
    return true;
}

bool checkCmd(int cmdvalue,string chatSname)
{
    using namespace std;
    //最开始只考虑是否登录
    if (cmdvalue == 0 || cmdvalue == 1)
    {
        if (state != clientState::noLogin)
        {
            cerr << "error> Have signed in yet!" << endl;
            return false;
        }
    }
    else
    {
        if (state == clientState::noLogin and cmdvalue != 17)//可以exit
        {
            cerr << "error> Not signed in yet!" << endl;
            return false;
        }
    }
    //如果正在chatting，不允许再chat
    if (state == clientState::isChatting && cmdvalue == 3)
    {
        cerr << "error> You are chatting with " << chatSname << "! You can break to chat with other." << endl;
        return false;
    }

    //如果正在等待，只允许break或exit
    if (state == clientState::isWaiting && !(cmdvalue == 6 || cmdvalue == 17))
    {
        cerr << "error> Only break(exit) or @break(@exit) can input because you are waiting for something!" << endl;
        return false;
    }

    return true;
}