#ifndef SERVER_CPP
#define SERVER_CPP

// ---- C ----
#include <unistd.h>
#include <sys/time.h>

// ---- kqueue ----
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>

#include "RequestParser.hpp"

#define MAX_EVENTS 100

#ifndef DEBUG
    #define DEBUG 0
#endif

class Server
{
	private:
		int _kq;
		struct addrinfo *_addr;
		size_t _listening_socket;		

	public:
		Server();
		~Server(void);

		void runServer();

		void readRequest(struct kevent& event);
		void sendResponse(struct kevent& event);

		void sendProcesssedResponse(struct kevent& event);
		void sendError(struct kevent& event);
		void handleTimeout(struct kevent& event);

		void newClient(struct kevent event);
		int removeEvent(struct kevent& event, int filter);
		void closeClient(struct kevent& event);

		std::string streamFile(std::string file);
		void sendResponseFile(struct kevent& event, std::string file);
		void sendImmage(struct kevent& event, std::string imgFileName);
		// int closeClient(struct kevent event, int filter);

		int getSocket();
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

#endif //SERVER_CPP
