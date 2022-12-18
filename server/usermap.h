#ifndef USERMAP_H
#define USERMAP_H

#include <string>
#include <unordered_map>
#include <mutex>
#include "state_c.h"
using namespace std;

class UserMap
{
private:
	//映射表
	unordered_map<int, string> conn1_ip;//id0，套接字1->ip，以下都用这样的命名方式
	unordered_map<string, int> ip_conn2;//id1
	unordered_map<int, int> conn1_2;//id2
	unordered_map<int, string> conn1_sid;//id3	
	unordered_map<int, string> conn1_sname;//id4
	unordered_map<string, int> sid_conn2;//id5
	unordered_map<string, string> sid_sname;//id6
	unordered_map<string, clientState> sid_state;//id7
	unordered_map<int, clientState> conn1_state;//id8
	unordered_map<int, int> conn1_peer2;//id9
	unordered_map<int, string> conn2_user;//id10
	unordered_map<int, string> conn1_user;//id11
	unordered_map<int, int> conn1_req_peer2;//id12
	//每个表的互斥锁
	mutex mux_conn1_ip;
	mutex mux_ip_conn2;
	mutex mux_conn1_2;
	mutex mux_conn1_sid;
	mutex mux_conn1_sname;
	mutex mux_sid_conn2;
	mutex mux_sid_sname;
	mutex mux_sid_state;
	mutex mux_conn1_state;
	mutex mux_conn1_peer2;
	mutex mux_conn2_user;
	mutex mux_conn1_user;
	mutex mux_conn1_req_peer2;
public:
	//-----------------------------------------------------0
	void ins_conn1_ip(int key, string value);
	void del_conn1_ip(int key);
	string fvalue_conn1_ip(int key);
	//不会通过ip找conn1

	//-----------------------------------------------------1
	void ins_ip_conn2(string key, int value);
	void del_ip_conn2(string key);
	int fvalue_ip_conn2(string key);
	//不会通过conn2找ip

	//-----------------------------------------------------2
	void ins_conn1_2(int key, int value);
	void del_conn1_2(int key);
	int fvalue_conn1_2(int key);
	int fkey_conn1_2(int value);//通过2找1

	//-----------------------------------------------------3
	void ins_conn1_sid(int key, string value);
	void del_conn1_sid(int key);
	string fvalue_conn1_sid(int key);
	int fkey_conn1_sid(string value);

	//-----------------------------------------------------4
	void ins_conn1_sname(int key, string value);
	void del_conn1_sname(int key);
	string fvalue_conn1_sname(int key);

	//-----------------------------------------------------5
	void ins_sid_conn2(string key, int value);
	void del_sid_conn2(string key);
	int fvalue_sid_conn2(string key);
	string fkey_sid_conn2(int value);

	//-----------------------------------------------------6
	void ins_sid_sname(string key, string value);
	void del_sid_sname(string key);
	string fvalue_sid_sname(string key);

	//-----------------------------------------------------7
	void ins_sid_state(string key, clientState value);
	void del_sid_state(string key);
	clientState fvalue_sid_state(string key);
	//不可能通过state找key

	//-----------------------------------------------------8
	void ins_conn1_state(int key, clientState value);
	void del_conn1_state(int key);
	clientState fvalue_conn1_state(int key);
	//不可能通过state找key
	
	//-----------------------------------------------------9
	void ins_conn1_peer2(int key, int value);
	void del_conn1_peer2(int key);
	int fvalue_conn1_peer2(int key);

	//-----------------------------------------------------10
	void ins_conn2_user(int key, string value);
	void del_conn2_user(int key);
	string fvalue_conn2_user(int key);

	//-----------------------------------------------------11
	void ins_conn1_user(int key, string value);
	void del_conn1_user(int key);
	string fvalue_conn1_user(int key);

	//-----------------------------------------------------12
	void ins_conn1_req_peer2(int key, int value);
	void del_conn1_req_peer2(int key);
	int fvalue_conn1_req_peer2(int key);
};
#endif
