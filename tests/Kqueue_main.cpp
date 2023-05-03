#include "WebServer.hpp"

void func() {
   system("leaks webserv");
}

int main(int ac, char **av)
{
   atexit(func);

	signal(SIGINT, endSignal);
	signalCall = false;

    std::string configFle;
    if (ac == 2) {
        configFle = av[1];
    } else {
        std::cout << GRN << "standard_complete.conf will be used!" << RES << std::endl;
        configFle = "standard_complete.conf";
    }
    try {
		WebServer webServ(configFle);
        webServ.runServer();
    }
    catch (std::exception const & e) {
        std::cout << RED << e.what() << RES << std::endl;
    }
	return (EXIT_SUCCESS);
}
