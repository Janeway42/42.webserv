#include "WebServer.hpp"

void func() {
    system("leaks webserv");
}

int main(int ac, char **av)
{
    //atexit(func);
    if (ac == 2) {
        try {
            WebServer *webServ = new WebServer(av[1]);
            webServ->runServer();
            delete webServ;
        }
        catch (std::exception const & e) {
            std::cout << RED << e.what() << RES << std::endl;
        }
    }
	return (EXIT_SUCCESS);
}
