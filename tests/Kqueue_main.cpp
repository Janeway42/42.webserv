#include "Server.hpp"

int main()
{
	try {
        Server* webserv = new Server();
        webserv->runServer();
        delete webserv;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
	return (EXIT_SUCCESS);
}
