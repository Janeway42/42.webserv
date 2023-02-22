#include "Server.hpp"

int main(int ac, char **av)
{
    if (ac == 2) {
        try {
            Server *webserv = new Server(av[1]);
            webserv->runServer();
            delete webserv;
        }
        catch (std::exception const & e) {
            std::cout << RED << e.what() << BACK << std::endl;
        }
    }
	return (EXIT_SUCCESS);
}
