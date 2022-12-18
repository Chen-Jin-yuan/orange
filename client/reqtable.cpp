#include "reqtable.h"


void ReqTable::insertReq(string sid, string cmd)
{
	lock_guard<mutex> locker(reqTableMux);
	reqTable[sid] = cmd;
}

void ReqTable::deleteReq(string sid)
{
	lock_guard<mutex> locker(reqTableMux);
	iterator iter = reqTable.find(sid);

	if (iter == reqTable.end())//��������ֱ�Ӳ���
		return;
	else
		reqTable.erase(iter);

}

string ReqTable::checkReq(string sid)
{
	lock_guard<mutex> locker(reqTableMux);
	iterator iter = reqTable.find(sid);

	if (iter == reqTable.end())//�������򷵻ؿգ������ϲ㴦��
		return "";
	else
	{
		string cmd = iter->second;
		if (cmd.size() >= 8)//���������sendfile������filename
		{
			filename = cmd.substr(8);
			cmd = "sendfile";
		}
		//�����겻��ɾ����������ɾ������Ϊ����accept����Ӧ��Ҫ����
		//reqTable.erase(iter);
		return cmd;
	}
}

void ReqTable::print()//��ӡ�������������Լ�һ����ӡ���������������������
{
	for (auto ite = reqTable.begin(); ite != reqTable.end(); ite++)
		cout << ite->first << "\t" << ite->second << endl;
}