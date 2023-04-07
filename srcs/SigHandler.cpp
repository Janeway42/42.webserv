#include "../includes/WebServer.hpp"

void endSignal(int sig)
{
	std::cout << "\nmanual turn off\n"; 
	if (sig == SIGINT)
		signalCall = true;
}
