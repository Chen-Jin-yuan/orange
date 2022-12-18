#include "usermap.h"
//---------------------------------------0
void UserMap::ins_conn1_ip(int key, string value)
{
	lock_guard<mutex> locker(mux_conn1_ip);
	conn1_ip[key] = value;//插入或修改
}
void UserMap::del_conn1_ip(int key)
{
	lock_guard<mutex> locker(mux_conn1_ip);
	typename unordered_map<int, string>::iterator iter = conn1_ip.find(key);
	if (iter == conn1_ip.end())//不存在则直接不管
		return;
	else
		conn1_ip.erase(iter);
}
string UserMap::fvalue_conn1_ip(int key)
{
	lock_guard<mutex> locker(mux_conn1_ip);
	typename unordered_map<int, string>::iterator iter = conn1_ip.find(key);

	if (iter == conn1_ip.end())
		return "";
	else
		return iter->second;
}
//---------------------------------------1
void UserMap::ins_ip_conn2(string key, int value)
{
	lock_guard<mutex> locker(mux_ip_conn2);
	ip_conn2[key] = value;//插入或修改
}
void UserMap::del_ip_conn2(string key)
{
	lock_guard<mutex> locker(mux_ip_conn2);
	typename unordered_map<string, int>::iterator iter = ip_conn2.find(key);
	if (iter == ip_conn2.end())//不存在则直接不管
		return;
	else
		ip_conn2.erase(iter);
}
int UserMap::fvalue_ip_conn2(string key)
{
	lock_guard<mutex> locker(mux_ip_conn2);
	typename unordered_map<string, int>::iterator iter = ip_conn2.find(key);

	if (iter == ip_conn2.end())
		return -1;
	else
		return iter->second;
}
//-----------------------------------------------------2
void UserMap::ins_conn1_2(int key, int value)
{
	lock_guard<mutex> locker(mux_conn1_2);
	conn1_2[key] = value;//插入或修改
}
void UserMap::del_conn1_2(int key)
{
	lock_guard<mutex> locker(mux_conn1_2);
	typename unordered_map<int, int>::iterator iter = conn1_2.find(key);
	if (iter == conn1_2.end())//不存在则直接不管
		return;
	else
		conn1_2.erase(iter);
}
int UserMap::fvalue_conn1_2(int key)
{
	lock_guard<mutex> locker(mux_conn1_2);
	typename unordered_map<int, int>::iterator iter = conn1_2.find(key);

	if (iter == conn1_2.end())
		return -1;
	else
		return iter->second;
}
int UserMap::fkey_conn1_2(int value)//通过2找1
{
	lock_guard<mutex> locker(mux_conn1_2);
	typename unordered_map<int, int>::iterator iter = conn1_2.begin();
	for (; iter != conn1_2.end(); iter++)
		if (iter->second == value)
			return iter->first;

	return -1;
}

//-----------------------------------------------------3
void UserMap::ins_conn1_sid(int key, string value)
{
	lock_guard<mutex> locker(mux_conn1_sid);
	conn1_sid[key] = value;//插入或修改
}
void UserMap::del_conn1_sid(int key)
{
	lock_guard<mutex> locker(mux_conn1_sid);
	typename unordered_map<int, string>::iterator iter = conn1_sid.find(key);
	if (iter == conn1_sid.end())//不存在则直接不管
		return;
	else
		conn1_sid.erase(iter);
}
string UserMap::fvalue_conn1_sid(int key)
{
	lock_guard<mutex> locker(mux_conn1_sid);
	typename unordered_map<int, string>::iterator iter = conn1_sid.find(key);

	if (iter == conn1_sid.end())
		return "";
	else
		return iter->second;
}
int UserMap::fkey_conn1_sid(string value)
{
	lock_guard<mutex> locker(mux_conn1_sid);
	typename unordered_map<int, string>::iterator iter = conn1_sid.begin();
	for (; iter != conn1_sid.end(); iter++)
		if (iter->second == value)
			return iter->first;

	return -1;
}

//-----------------------------------------------------4
void UserMap::ins_conn1_sname(int key, string value)
{
	lock_guard<mutex> locker(mux_conn1_sname);
	conn1_sname[key] = value;//插入或修改
}
void UserMap::del_conn1_sname(int key)
{
	lock_guard<mutex> locker(mux_conn1_sname);
	typename unordered_map<int, string>::iterator iter = conn1_sname.find(key);
	if (iter == conn1_sname.end())//不存在则直接不管
		return;
	else
		conn1_sname.erase(iter);
}
string UserMap::fvalue_conn1_sname(int key)
{
	lock_guard<mutex> locker(mux_conn1_sname);
	typename unordered_map<int, string>::iterator iter = conn1_sname.find(key);

	if (iter == conn1_sname.end())
		return "";
	else
		return iter->second;
}

//-----------------------------------------------------5
void UserMap::ins_sid_conn2(string key, int value)
{
	lock_guard<mutex> locker(mux_sid_conn2);
	sid_conn2[key] = value;//插入或修改
}
void UserMap::del_sid_conn2(string key)
{
	lock_guard<mutex> locker(mux_sid_conn2);
	typename unordered_map<string, int>::iterator iter = sid_conn2.find(key);
	if (iter == sid_conn2.end())//不存在则直接不管
		return;
	else
		sid_conn2.erase(iter);
}
int UserMap::fvalue_sid_conn2(string key)
{
	lock_guard<mutex> locker(mux_sid_conn2);
	typename unordered_map<string, int>::iterator iter = sid_conn2.find(key);

	if (iter == sid_conn2.end())
		return -1;
	else
		return iter->second;
}
string UserMap::fkey_sid_conn2(int value)
{
	lock_guard<mutex> locker(mux_sid_conn2);
	typename unordered_map<string, int>::iterator iter = sid_conn2.begin();
	for (; iter != sid_conn2.end(); iter++)
		if (iter->second == value)
			return iter->first;

	return "";
}

//-----------------------------------------------------6
void UserMap::ins_sid_sname(string key, string value)
{
	lock_guard<mutex> locker(mux_sid_sname);
	sid_sname[key] = value;//插入或修改
}
void UserMap::del_sid_sname(string key)
{
	lock_guard<mutex> locker(mux_sid_sname);
	typename unordered_map<string, string>::iterator iter = sid_sname.find(key);
	if (iter == sid_sname.end())//不存在则直接不管
		return;
	else
		sid_sname.erase(iter);
}
string UserMap::fvalue_sid_sname(string key)
{
	lock_guard<mutex> locker(mux_sid_sname);
	typename unordered_map<string, string>::iterator iter = sid_sname.find(key);

	if (iter == sid_sname.end())
		return "";
	else
		return iter->second;
}

//-----------------------------------------------------7
void UserMap::ins_sid_state(string key, clientState value)
{
	lock_guard<mutex> locker(mux_sid_state);
	sid_state[key] = value;//插入或修改
}
void UserMap::del_sid_state(string key)
{
	lock_guard<mutex> locker(mux_sid_state);
	typename unordered_map<string, clientState>::iterator iter = sid_state.find(key);
	if (iter == sid_state.end())//不存在则直接不管
		return;
	else
		sid_state.erase(iter);
}
clientState UserMap::fvalue_sid_state(string key)
{
	lock_guard<mutex> locker(mux_sid_state);
	typename unordered_map<string, clientState>::iterator iter = sid_state.find(key);

	if (iter == sid_state.end())
		return clientState::noLogin;
	else
		return iter->second;
}
//不可能通过state找key

//-----------------------------------------------------8
void UserMap::ins_conn1_state(int key, clientState value)
{
	lock_guard<mutex> locker(mux_conn1_state);
	conn1_state[key] = value;//插入或修改
}
void UserMap::del_conn1_state(int key)
{
	lock_guard<mutex> locker(mux_conn1_state);
	typename unordered_map<int, clientState>::iterator iter = conn1_state.find(key);
	if (iter == conn1_state.end())//不存在则直接不管
		return;
	else
		conn1_state.erase(iter);
}
clientState UserMap::fvalue_conn1_state(int key)
{
	lock_guard<mutex> locker(mux_conn1_state);
	typename unordered_map<int, clientState>::iterator iter = conn1_state.find(key);

	if (iter == conn1_state.end())
		return clientState::noLogin;
	else
		return iter->second;
}
//不可能通过state找key

//-----------------------------------------------------9
void UserMap::ins_conn1_peer2(int key, int value)
{
	lock_guard<mutex> locker(mux_conn1_peer2);
	conn1_peer2[key] = value;//插入或修改
}
void UserMap::del_conn1_peer2(int key)
{
	lock_guard<mutex> locker(mux_conn1_peer2);
	typename unordered_map<int, int>::iterator iter = conn1_peer2.find(key);
	if (iter == conn1_peer2.end())//不存在则直接不管
		return;
	else
		conn1_peer2.erase(iter);
}
int UserMap::fvalue_conn1_peer2(int key)
{
	lock_guard<mutex> locker(mux_conn1_peer2);
	typename unordered_map<int, int>::iterator iter = conn1_peer2.find(key);

	if (iter == conn1_peer2.end())
		return -1;
	else
		return iter->second;
}

//-----------------------------------------------------10
void UserMap::ins_conn2_user(int key, string value)
{
	lock_guard<mutex> locker(mux_conn2_user);
	conn2_user[key] = value;//插入或修改
}
void UserMap::del_conn2_user(int key)
{
	lock_guard<mutex> locker(mux_conn2_user);
	typename unordered_map<int, string>::iterator iter = conn2_user.find(key);
	if (iter == conn2_user.end())//不存在则直接不管
		return;
	else
		conn2_user.erase(iter);
}
string UserMap::fvalue_conn2_user(int key)
{
	lock_guard<mutex> locker(mux_conn2_user);
	typename unordered_map<int, string>::iterator iter = conn2_user.find(key);

	if (iter == conn2_user.end())
		return "";
	else
		return iter->second;
}

//-----------------------------------------------------11
void UserMap::ins_conn1_user(int key, string value)
{
	lock_guard<mutex> locker(mux_conn1_user);
	conn1_user[key] = value;//插入或修改
}
void UserMap::del_conn1_user(int key)
{
	lock_guard<mutex> locker(mux_conn1_user);
	typename unordered_map<int, string>::iterator iter = conn1_user.find(key);
	if (iter == conn1_user.end())//不存在则直接不管
		return;
	else
		conn1_user.erase(iter);
}
string UserMap::fvalue_conn1_user(int key)
{
	lock_guard<mutex> locker(mux_conn1_user);
	typename unordered_map<int, string>::iterator iter = conn1_user.find(key);

	if (iter == conn1_user.end())
		return "";
	else
		return iter->second;
}

//-----------------------------------------------------12
void UserMap::ins_conn1_req_peer2(int key, int value)
{
	lock_guard<mutex> locker(mux_conn1_req_peer2);
	conn1_req_peer2[key] = value;//插入或修改
}
void UserMap::del_conn1_req_peer2(int key)
{
	lock_guard<mutex> locker(mux_conn1_req_peer2);
	typename unordered_map<int, int>::iterator iter = conn1_req_peer2.find(key);
	if (iter == conn1_req_peer2.end())//不存在则直接不管
		return;
	else
		conn1_req_peer2.erase(iter);
}
int UserMap::fvalue_conn1_req_peer2(int key)
{
	lock_guard<mutex> locker(mux_conn1_req_peer2);
	typename unordered_map<int, int>::iterator iter = conn1_req_peer2.find(key);

	if (iter == conn1_req_peer2.end())
		return -1;
	else
		return iter->second;
}
