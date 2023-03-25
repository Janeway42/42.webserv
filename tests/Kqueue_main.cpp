#include "WebServer.hpp"

void func() {
    system("leaks webserv");
}

int main(int ac, char **av)
{
    //atexit(func);
    std::string configFle;
    if (ac == 2) {
        configFle = av[1];
    } else {
        std::cout << GRE << "standard_complete.conf will be used!" << RES << std::endl;
        configFle = "standard_complete.conf";
    }
    try {
        WebServer *webServ = new WebServer(configFle);
        webServ->runServer();
        delete webServ;
    }
    catch (std::exception const & e) {
        std::cout << RED << e.what() << RES << std::endl;
    }
	return (EXIT_SUCCESS);
}
