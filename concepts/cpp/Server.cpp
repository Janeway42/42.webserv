#include "Server.hpp"

Server::Server()
{
	struct addrinfo *_addr = new struct addrinfo();
	struct addrinfo hints;

	hints.ai_family = PF_UNSPEC; 
	hints.ai_flags = AI_PASSIVE; 
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", "8080", &hints, &_addr) != 0)
		throw ServerException(("failed addr"));

	int _listening_socket = socket(_addr->ai_family, _addr->ai_socktype, _addr->ai_protocol);
	if (_listening_socket == -1)
		throw ServerException(("failed socket"));
	fcntl(_listening_socket, F_SETFL, O_NONBLOCK);

	int socket_on = 1;
	setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
	if (bind(_listening_socket, _addr->ai_addr, _addr->ai_addrlen) == -1)
		throw ServerException(("failed bind"));
	if (listen(_listening_socket, 5) == -1)
		throw ServerException(("failed listen"));

	struct kevent evSet;
	_kq = kqueue();
	if (_kq == -1)
		throw ServerException(("failed kq"));
	EV_SET(&evSet, _listening_socket, EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE	, 0, NULL);
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1) 
		throw ServerException(("failed kevent start"));
}

Server::~Server()
{
	close(_kq);
	freeaddrinfo(_addr);
}

// -------------------------------------- server 

void Server::runServer()
{
	struct kevent evList[MAX_EVENTS];
	struct kevent evSet;
	int i;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);

	int loop1 = 0;
	while (1)
	{
		std::cout << "WHILE LOOP ------------------------------ %d\n";
		int nr_events = kevent(_kq, NULL, 0, evList, 32, NULL);
		std::cout << "NR EVENTS: " << nr_events << std::endl;

		if (nr_events < 1)
			throw ServerException("failed number events");

		for (i = 0; i < nr_events; i++)
		{
			std::cout << "filter: " << evList[i].filter << std::endl;

			if (evList[i].flags & EV_ERROR)
				std::cout << "Event error\n";

			else if (evList[i].ident == _listening_socket)
				newClient(evList[i]);
			
			else if (evList[i].filter == EVFILT_READ)
			{
				if (evList[i].flags & EV_EOF)
				{
					if (closeClient(evList[i], EVFILT_READ) == 1)
						throw ServerException("failed kevent eof - read");
				}
				else
					readRequest(evList[i]);
			}

			else if (evList[i].filter == EVFILT_WRITE)
			{
				if (evList[i].flags & EV_EOF)
				{
					if (closeClient(evList[i], EVFILT_WRITE) == 1)
						throw ServerException("failed kevent eof - write");
				}
				else
					sendRequest(evList[i]);
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
		loop1++;
	}
}

void Server::readRequest(struct kevent event)
{
	printf("READ\n");
	std::string buffer;
	std::string request;
	int ret = recv(event.ident, &buffer, sizeof(buffer), 0);
	if (ret < 0)
	{
		printf("error reading\n");
		closeClient(event, EVFILT_READ);
	}
	// else if (ret == 0)
	// 	parsingJaka();
	else
		request.append(buffer);
	std::cout << "buffer received and read: " << buffer << std::endl;
	std::cout << "bytes read: " << ret << std::endl;
	memset(&buffer, '\0', sizeof(buffer));
}

void Server::sendRequest(struct kevent event)
{
	printf("WRITE\n");

	send(event.ident, "HTTP/2 200 OK\n", 14, 0);
	send(event.ident, "Content-Length: 23\n", 19, 0);
	send(event.ident, "\n", 1, 0);
	int ret = send(event.ident, "Hello back from write!\n", 23, 0);
	// if (ret < 0)	
	//	throw ServerException("failed send");
	std::cout << "ret: " << ret << std::endl;
	shutdown(event.ident, SHUT_RDWR);  // not allowed && doesn't make a difference
	close(event.ident);
	printf("closed connection from write\n");
}

// -------------------------------------- client functions 

int Server::newClient(struct kevent event)
{
	struct kevent evSet;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);
	
	printf("make new client connection\n");
	int opt_value = 1;
	int fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	fcntl(fd, F_SETFL, O_NONBLOCK);  
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
	if (fd == -1)
		throw ServerException("failed accept");
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_READ");
	EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_WRITE");
	return (0);
}

int Server::closeClient(struct kevent event, int filter)
{
	struct kevent evSet;

	printf("connection closed - read \n");
	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, NULL); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1) 
		throw ServerException("failed kevent EV_DELETE client");
	close(event.ident); 
	return (0);
}
