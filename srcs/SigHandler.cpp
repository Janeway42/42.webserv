#include "../includes/WebServer.hpp"

void endSignal(int sig)
{
    // TODO: IF IT IS STUCK ON A LOOP, IT IS NOT QUITTING
	std::cout << "\n============= manual shut down =============\n"; 
	if (sig == SIGINT)
		signalCall = true;
}
