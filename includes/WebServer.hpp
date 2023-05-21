#ifndef WEBSERVER_CPP
#define WEBSERVER_CPP

// ---- C ----
#include <unistd.h>
#include <sys/time.h>

// ---- kqueue ----
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>

#include "RequestParser.hpp"
#include "ConfigFileParser.hpp"

#define MAX_EVENTS 100
#define BUFFER_SIZE 4000
#define TIMEOUT 30

static bool signalCall;

class WebServer
{
	private:
        WebServer() {};
		int _kq;
        std::vector<ServerData> _servers;

	public:
        WebServer(std::string const & configFileName);
		~WebServer(void);

		// main functions 
		void	runServer();
		void	readRequest(struct kevent& event);
		void	sendResponse(struct kevent& event);
		void	handleTimeout(struct kevent &event);
		void	newClient(struct kevent event);
		int		addFilter(int fd, struct kevent& event, int filter);
		void	removeFilter(struct kevent& event, int filter, std::string errorMessage);
		void	closeClient(struct kevent& event);
		void	cleanAddFilterFail(int fd, struct kevent& event, std::string errorMessage);
		void	protectFirstFilterAdd(Request *storage, int fd);
		int		exitProgram(int nr_events,struct kevent evList[MAX_EVENTS]);
		
		// utils 
		bool isListeningSocket(int fd);
		ServerData* getSpecificServer(int fd);
		int checkExistingSocket(int location, std::string port, std::string host);
		void chooseMethod_StartCGI(struct kevent event, Request* storage);
		void sendProcesssedResponse(struct kevent& event);

        class ServerException: public std::exception
        {
            private:
                std::string _errorMessage;

            public:
                // explicit: https://leimao.github.io/blog/CPP-Explicit-Constructor/
                explicit ServerException(const std::string& message) throw()
                {
                    _errorMessage = message;
                }
                virtual const char* what() const throw()
                {
                    return (_errorMessage.c_str());
                }
                virtual ~ServerException() throw(){}
        };
};

void endSignal(int sig);

#endif //WEBSERVER_CPP
