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

	if (iter == reqTable.end())//不存在则直接不管
		return;
	else
		reqTable.erase(iter);

}

string ReqTable::checkReq(string sid)
{
	lock_guard<mutex> locker(reqTableMux);
	iterator iter = reqTable.find(sid);

	if (iter == reqTable.end())//不存在则返回空，交由上层处理
		return "";
	else
	{
		string cmd = iter->second;
		if (cmd.size() >= 8)//这种情况是sendfile，设置filename
		{
			filename = cmd.substr(8);
			cmd = "sendfile";
		}
		//查找完不能删除，还不能删除，因为可能accept不对应，要保留
		//reqTable.erase(iter);
		return cmd;
	}
}

void ReqTable::print()//打印表项，后续或许可以加一个打印请求表的命令，现在用作测试
{
	for (auto ite = reqTable.begin(); ite != reqTable.end(); ite++)
		cout << ite->first << "\t" << ite->second << endl;
}