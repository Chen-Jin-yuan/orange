#ifndef REQTABLE_H
#define REQTABLE_H
//请求表
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>

using namespace std;


class ReqTable//主线程和接收线程都需要互斥操作
{
private:
	unordered_map<string, string> reqTable;
	using iterator = unordered_map<string, string>::iterator;
	string filename;
	mutex reqTableMux;
public:
	ReqTable() {}
	~ReqTable() {}
	void insertReq(string sid, string cmd);//插入表项
	void deleteReq(string sid);//删除表项
	string checkReq(string sid);//查找，如果是sendfile，让用户调用getName函数获取
	string getName() { return filename; }

	void print();
};

#endif
