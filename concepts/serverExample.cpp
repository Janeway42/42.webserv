#include <sys/socket.h>
#include <iostream>
#include <string>


int errorExit(void)
{
	std::cout << strerror(errno) << std::endl;
	exit(1);
}
int main()
{
	// socket
	int serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD < 0)
		errorExit();

	
	// bind / name a socket
	struct sockaddr_in address;
	int bind(serverFD, &address, sizeof(address));
	
	return (0);
}