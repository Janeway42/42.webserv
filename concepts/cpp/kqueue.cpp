#include "Server.hpp"

int main()
{
	Server *webserv;
	try
	{
		webserv = new Server();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}	
	
	try
	{
		webserv->runServer();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}	

	return (EXIT_SUCCESS);
}