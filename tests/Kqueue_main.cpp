#include "WebServer.hpp"

int main(int ac, char **av)
{
    if (ac == 2) {
        try {
            WebServer *webServ = new WebServer(av[1]);
            webServ->runServer();
            delete webServ;
        }
        catch (std::exception const & e) {
            std::cout << RED << e.what() << BACK << std::endl;
        }
    }
	return (EXIT_SUCCESS);
}
