#include "client.h"
const char* SERVER_IP = "101.34.2.129";
const int cmdPORT1 = 9000;
const int sendPORT2 = 8000;
const int udpPORT3 = 10000;

int main()
{
	Client client;
	client.start(SERVER_IP, cmdPORT1, sendPORT2);
	
	//system("pause "); 这东西要os介入
	cerr << endl << "Enter to exit the program..." << endl;
	getchar();
	return 0;
}