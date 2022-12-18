#ifndef REQTABLE_H
#define REQTABLE_H
//�����
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>

using namespace std;


class ReqTable//���̺߳ͽ����̶߳���Ҫ�������
{
private:
	unordered_map<string, string> reqTable;
	using iterator = unordered_map<string, string>::iterator;
	string filename;
	mutex reqTableMux;
public:
	ReqTable() {}
	~ReqTable() {}
	void insertReq(string sid, string cmd);//�������
	void deleteReq(string sid);//ɾ������
	string checkReq(string sid);//���ң������sendfile�����û�����getName������ȡ
	string getName() { return filename; }

	void print();
};

#endif
