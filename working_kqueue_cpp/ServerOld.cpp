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

	_listening_socket = socket(_addr->ai_family, _addr->ai_socktype, _addr->ai_protocol);
	if (_listening_socket == -1)
		throw ServerException(("failed socket"));
	fcntl(_listening_socket, F_SETFL, O_NONBLOCK);

	int socket_on = 1;
	setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
	if (bind(_listening_socket, _addr->ai_addr, _addr->ai_addrlen) == -1)
		throw ServerException(("failed bind"));
	if (listen(_listening_socket, SOMAXCONN) == -1)  // max nr of accepted connections 
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
		std::cout << "WHILE LOOP ------------------------------" << loop1 << std::endl;
		int nr_events = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);
		std::cout << "NR EVENTS: " << nr_events << std::endl;

		if (nr_events < 1)
			throw ServerException("failed number events");
		// else if (nr_events == 0)
		// 	handleTimeout(evList[0]);
		else
		{
			for (i = 0; i < nr_events; i++)
			{
				std::cout << "filter: " << evList[i].filter << std::endl; // test line 

				if (evList[i].flags & EV_ERROR)
					std::cout << "Event error\n";
				else if (evList[i].ident == _listening_socket)
					newClient(evList[i]);
				else if (evList[i].filter == EVFILT_READ)
				{
					printf("READ\n");
					if (evList[i].flags & EV_EOF)  // if client closes connection 
					{
						if (removeEvent(evList[i], EVFILT_READ) == 1)
							throw ServerException("failed kevent eof - read");
						closeClient(evList[i]);
					}
					else
						readRequest(evList[i]);
				}
				else if (evList[i].filter == EVFILT_WRITE)
				{
					printf("WRITE\n");
					if (evList[i].flags & EV_EOF)
					{
						if (removeEvent(evList[i], EVFILT_WRITE) == 1)
							throw ServerException("failed kevent eof - write");
						closeClient(evList[i]);
					}
					else
						sendResponse(evList[i]);
				}
				std::cout << std::endl;
			}
		}	
		std::cout << std::endl;
		loop1++;
	}
}

// -------------------------------------- server functions 

void Server::readRequest(struct kevent& event)
{
	char buffer[50];
	memset(&buffer, '\0', 50);
	data::Request *storage;
	storage = (data::Request *)event.udata;

	int ret = recv(event.ident, &buffer, sizeof(buffer) - 1, 0);
	std::cout << "fd: " << event.ident << std::endl;
	std::cout << "bytes read: " << ret << std::endl;   // test line 
	if (ret < 0)
	{
		if (storage->getEarlyClose() == false)
		{
			std::cout << "failed recv\n";
			storage->setError(true);
			if (removeEvent(event, EVFILT_READ) == 1)
				throw ServerException("failed EVFILT_READ removal\n");
			std::cout << "send error message back to client\n";
		}
		else
			std::cout << "EVFILT_READ already closed\n";
		return ;
	}
	// for cgi ret might be 0 - might need to add later, but for kqueue it will never be 0 

	if (storage->getEarlyClose() == false && storage->getDone() == false)
	{
		std::cout << "append buffer\n";
		std::cout << "buffer I send: " << buffer << std::endl;
		storage->appendToRequest(buffer);
		std::cout << "done after append: " << storage->getDone() << std::endl;
		std::cout << "temp buffer: " << storage->getTemp() << std::endl;

		// storage->setDone(true);
		if (storage->getError() == true)
		{
			std::cout << "error parsing - sending response - failure\n";
			storage->setEarlyClose(true);
			if (removeEvent(event, EVFILT_READ) == 1)
				throw ServerException("failed kevent eof - read failure");
		}
		else if (storage->getDone() == true)
		{
			std::cout << "done parsing - sending response - success\n";
			if (removeEvent(event, EVFILT_READ) == 1)
				throw ServerException("failed kevent eof - read success");
		}
	}
	// std::cout << "buffer received and read: " << buffer << std::endl;  // test line 	
}

void Server::sendResponse(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;

	std::time_t elapsedTime = std::time(NULL);
	if (elapsedTime - storage->getTime() > 30)
	{
		std::cout << "Unable to process request, it takes too long!\n";   // test line 
		storage->setError(true);
		storage->setEarlyClose(true);
		std::cout << "storage->closeEarly: " << storage->getEarlyClose() << std::endl;
		if (removeEvent(event, EVFILT_READ) == 1)
			throw ServerException("failed EVFILT_READ removal\n"); 
	}

	std::cout << "storage->done: " << storage->getDone() << std::endl;
	if (storage->getError() == true)
	{
		std::string responseNoFav =	"<!DOCTYPE htlm>"
						"<html>"
						"<head>"
						"<title>Favicon</title>"
						"<link rel='icon' href='data:,'>"
						"</head>"
						"<body>"
						"<h4>Failure!!!</h4>"
						"</body>"
						"</html>";
		send(event.ident, "HTTP/1.1 200 OK\r\n", 17, 0);
		send(event.ident, "Content-Length: 127\r\n", 20, 0);
		send(event.ident, "Content-Type: text/html; charset=UTF-8\r\n", 40, 0);
		send(event.ident, "\r\n\r\n", 4, 0);
		int ret = send(event.ident, responseNoFav.c_str(), responseNoFav.length(), 0);
		if (ret < 0)	
			throw ServerException("failed send");
		std::cout << "ret: " << ret << std::endl;
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client");
		closeClient(event);
		printf("closed connection from write - error\n");   // test line 
	}
	else if (storage->getDone() == true)
	{
		std::string responseNoFav =	"<!DOCTYPE htlm>"
						"<html>"
						"<head>"
						"<title>Favicon</title>"
						"<link rel='icon' href='data:,'>"
						"</head>"
						"<body>"
						"<h4>Hello</h4>"
						"</body>"
						"</html>";
		// std::cout << "size body: " << responseNoFav.length() << std::endl;
								
		send(event.ident, "HTTP/1.1 200 OK\r\n", 17, 0);
		send(event.ident, "Content-Length: 122\r\n", 20, 0);
		send(event.ident, "Content-Type: text/html; charset=UTF-8\r\n", 40, 0);
		send(event.ident, "\r\n\r\n", 4, 0);
		int ret = send(event.ident, responseNoFav.c_str(), responseNoFav.length(), 0);
		if (ret < 0)	
			throw ServerException("failed send");
		std::cout << "ret: " << ret << std::endl;

		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client");
		closeClient(event);
		printf("closed connection from write - success\n");   // test line 
	}
	// else 
	// 	std::cout << "just passing by\n"; // test line 
}

// -------------------------------------- client functions 

void Server::newClient(struct kevent event)
{
	struct kevent evSet;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);
	struct timespec timeout = {0, 0};  /// to be later removed 

	printf("make new client connection\n");
	
	int opt_value = 1;
	int fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	fcntl(fd, F_SETFL, O_NONBLOCK);  
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
	if (fd == -1)
		throw ServerException("failed accept");

	data::Request *storage = new data::Request();
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, &timeout) == -1)
		throw ServerException("failed kevent add EVFILT_READ");
	EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_WRITE");
}

int Server::removeEvent(struct kevent& event, int filter)
{
	struct kevent evSet;
	data::Request *storage;
	storage = (data::Request *)event.udata;

	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		std::cout << errno << std::endl;
		throw ServerException("failed kevent EV_DELETE client");
	}
	return (0);
}

// void Server::handleTimeout(struct kevent& event)
// {
// 	struct kevent evSet;
// 	data::Request *storage;
// 	storage = (data::Request *)event.udata;

// 	std::cout << "Handle timeout!\n";
// 	storage->setDone(true);
// 	storage->setError(true);

// 	EV_SET(&evSet, event.ident, EVFILT_READ, EV_DELETE, 0, 0, storage); 
// 	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
// 	{
// 		std::cout << errno << std::endl;
// 		throw ServerException("failed kevent EV_DELETE client");
// 	}
// }

void Server::closeClient(struct kevent& event)
{
	data::Request *storage;
	storage = (data::Request *)event.udata; 

	delete(storage);
	std::cout << "connection closed\n";
	close(event.ident); 
}

// -------------------------------------- get functions

int Server::getSocket(void)
{
	return (_listening_socket);
}

int Server::getKq(void)
{
	return (_kq);
}
