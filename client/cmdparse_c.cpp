#include "cmdparse_c.h"
extern clientState state;

//ȥ��string��β�ո�
void trim(string& s)
{
    if (!s.empty())
    {
        s.erase(0, s.find_first_not_of(" \t"));//�״γ��ֲ�ƥ��ո��λ��
        s.erase(s.find_last_not_of(" \t") + 1);
    }
}

//�������cmdstr��λ�ո�ȥ����
vector<string> parse(string cmdstr)
{
    //���ȿ���һ���ַ��ǲ���@���ǵĻ�ȥ���ͺ���
    if (cmdstr[0] == '@')
        cmdstr = cmdstr.substr(1, cmdstr.size() - 1);

    //����һ���ؼ��ֵ�����޷��ÿո�ָ���ǵ����һ���и�\n��û�õģ���˰�\n��Ϊ�ո�һ������
    cmdstr[cmdstr.size() - 1] = ' ';
    vector<string> res;
    size_t pos = 0;
    size_t pos1;
    while ((pos1 = cmdstr.find(' ', pos)) != string::npos)
    {
        res.push_back(cmdstr.substr(pos, pos1 - pos));
        while (cmdstr[pos1] == ' ')//���˿ո�
            pos1++;
        pos = pos1;
    }

    return res;//����ֵ����ֵ���ⲿvector�������ֵ�������ƶ�����

}

bool isNumber(const char* buf)//�ж��ַ����Ƿ�Ϊ����
{
    for (int i = 0; buf[i] != '\n'; i++)//��\n��β
        if (buf[i] < '0' || buf[i]>'9')//ÿ���ַ���Ҫ������
            return false;
    return true;
}

bool checkCmd(int cmdvalue,string chatSname)
{
    using namespace std;
    //�ʼֻ�����Ƿ��¼
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
        if (state == clientState::noLogin and cmdvalue != 17)//����exit
        {
            cerr << "error> Not signed in yet!" << endl;
            return false;
        }
    }
    //�������chatting����������chat
    if (state == clientState::isChatting && cmdvalue == 3)
    {
        cerr << "error> You are chatting with " << chatSname << "! You can break to chat with other." << endl;
        return false;
    }

    //������ڵȴ���ֻ����break��exit
    if (state == clientState::isWaiting && !(cmdvalue == 6 || cmdvalue == 17))
    {
        cerr << "error> Only break(exit) or @break(@exit) can input because you are waiting for something!" << endl;
        return false;
    }

    return true;
}