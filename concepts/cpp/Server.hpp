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

#define MAX_EVENTS 32

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

		void readRequest(struct kevent event);
		void sendRequest(struct kevent event);

		
		int newClient(struct kevent event);
		int closeClient(struct kevent event, int filter);

		
	class ServerException: public std::exception
	{
		private:
			std::string _errorMessage;

		public:
			ServerException(const char *message): _errorMessage(message){}
			virtual ~ServerException(void) noexcept{}
			virtual const char* what() const throw()
			{
				return (_errorMessage.c_str());
			}
	};


};

#endif