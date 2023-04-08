#include "../includes/WebServer.hpp"

void endSignal(int sig)
{
	std::cout << "\n============= manual shut down =============\n"; 
	if (sig == SIGINT)
		signalCall = true;
}
