#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

// ---- C ----
#include <unistd.h>
#include <sys/time.h>

// ---- kqueue ----
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>

#include "ConfigFileParser.hpp"

#define MAX_EVENTS 100

#ifndef DEBUG
    #define DEBUG 0
#endif

class WebServer
{
	private:
		int _kq;
		struct addrinfo *_addr;
		size_t _listening_socket;// todo delete? it is now on the ServerLocation
        std::vector<ServerData> _servers;

	public:
        // todo add a default constructor? -> maybe private so no one can call it
        WebServer(std::string const & configFileName);
		~WebServer(void);

		void runServer();
		void readRequest(struct kevent& event);
		void sendResponse(struct kevent& event);

		void newClient(struct kevent event);
		int removeEvent(struct kevent& event, int filter);
		void closeClient(struct kevent& event);

		// process request 
		void processResponse(struct kevent& event);
		void doGet(struct kevent& event);
		void doPost(struct kevent& event);
		void doDelete(struct kevent& event);
		void doNotAllowed(struct kevent& event);

		// utils request
		void sendFile(struct kevent& event);

		// getters
		int getKq();

        class ServerException: public std::exception
        {
            private:
                std::string _errorMessage;

            public:
                ServerException(std::string message) throw()
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

#endif 
