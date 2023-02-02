#ifndef SERVER_CPP
#define SERVER_CPP

// ---- C ----
#include <unistd.h>

// ---- kqueue ----
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>

// ---- C++ ----
#include <iostream>
#include <string>

#include "requestStorage.hpp"

#define MAX_EVENTS 100

class Server
{
	private:
		int _kq;
		struct addrinfo *_addr;
		int _listening_socket;

		

	public:
		Server();
		~Server(void);
		void runServer();

		void readRequest(struct kevent& event);
		void sendResponse(struct kevent& event);

		int newClient(struct kevent event);
		int removeEvent(struct kevent& event, int filter);
		void closeClient(struct kevent& event);
		
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

#endif