#include "server.h"
#include "pthread.h"
int main()
{
	Server server(3306, "root", "jysama@0", "orange", 10,
		0, "./log", 1024, 2, 1024, "jysama", 0, 1024);
	server.start();
	pthread_exit(NULL);
}
