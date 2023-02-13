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

// --------------------------------------------------------- server main loop
// --------------------------------------------------------------------------

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

// --------------------------------------------------------- server functions 
// --------------------------------------------------------------------------

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

	// std::time_t elapsedTime = std::time(NULL);
	// if (elapsedTime - storage->getTime() > 2)
	// {
	// 	std::cout << "Unable to process request, it takes too long!\n";   // test line 
	// 	storage->setError(true);
	// 	storage->setEarlyClose(true);
	// 	std::cout << "storage->closeEarly: " << storage->getEarlyClose() << std::endl;
	// 	if (removeEvent(event, EVFILT_READ) == 1)
	// 		throw ServerException("failed EVFILT_READ removal\n"); 
	// }

	std::cout << "storage->done: " << storage->getDone() << std::endl;

	if (storage->getError() == true)
	{

		sendErrorAutoLength(event.ident, event);
		// std::string responseNoFav;
		// responseNoFav = streamFile("error404.html");

		// send(event.ident, "HTTP/1.1 404 KO\r\n", 17, 0);
		// send(event.ident, "Content-Length: 316\r\n", 21, 0);
		// send(event.ident, "Content-Type: text/html; charset=UTF-8\r\n", 40, 0);
		// send(event.ident, "\r\n\r\n", 4, 0);
		// int ret = send(event.ident, responseNoFav.c_str(), responseNoFav.length(), 0);
		// if (ret < 0)
		// 	throw ServerException("failed send");
		// std::cout << "ret: " << ret << std::endl;
	
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client");
		sleep(10);
		closeClient(event);
		printf("closed connection from write - error\n");   // test line 
	}
	else if (storage->getDone() == true)
	{
		// std::string responseNoFav;
		// responseNoFav = streamFile("index_dummy.html");
		// int size = responseNoFav.length();
		// std::string sizeStr = std::to_string(size);

		// send(event.ident, "HTTP/1.1 200 OK\r\n", 17, 0);
		// // send(event.ident, "Content-Length: 1654\r\n", 20, 0);
		// send(event.ident, "Content-Length: ", 16, 0);
		// send(event.ident, &sizeStr, std::to_string(size).length(), 0);
		// send(event.ident, " \r\n", 3, 0);
		// send(event.ident, "Content-Type: text/html; charset=UTF-8\r\n", 40, 0);
		// send(event.ident, "\r\n\r\n", 4, 0);
		// int ret = send(event.ident, responseNoFav.c_str(), responseNoFav.length(), 0);
		// if (ret < 0)	
		// 	throw ServerException("failed send");
		// std::cout << "ret: " << ret << std::endl;

		std::string temp = (storage->getRequestData()).getHttpPath();

		if (temp.find(".png") != std::string::npos)
		{
			int		ch;
			int		i;
			int 	buffer[1000000];
			int 	socketfd = event.ident;
	
			FILE	*fptr;
			fptr = fopen("immage.png", "r");
			if (fptr == NULL)
				std::cout << "Null pointer\n";
			memset(buffer, '\0', 1000000);

			//for (i = 0; (i < (100000) && ((ch = fgetc(fptr)) != EOF) && (ch != '\n')); i++)
			for (i = 0; (i < (1000000) && ((ch = fgetc(fptr)) != EOF)); i++)
			{
				// printf("%d\n", i);
				buffer[i] = ch; 
			}
			std::cout << "test immage ------------------\n";
			printf("\nSIZE i: %d\n", i);
			
			fclose(fptr);

			std::string responseHeader =	"HTTP/1.1 200 OK\r\n"
											"Content-Type: image/jpg\r\n"
											"Content-Length: 109894\r\n\r\n";
			
			send(socketfd, &responseHeader, responseHeader.length(), 0);
			send(socketfd, &buffer, i, 0);			
		}
		else 
			processResponse(event.ident, "index_dummy.html");
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client");
		closeClient(event);
		printf("closed connection from write - success\n");   // test line 
	}
	// else 
	// 	std::cout << "just passing by\n"; // test line 
}

void Server::processResponse(int fd, std::string file)
{
	std::string responseNoFav;
	responseNoFav = streamFile(file);
	int size = responseNoFav.length();
	std::cout << "size: " << size << std::endl;
	std::string sizeStr = std::to_string(size);

	std::string content = "Content-Length: ";
	content.append(std::to_string(size));
	content.append("\r\n");
	int length = 18 + std::to_string(size).length();
	std::cout << "content: " << content << std::endl;
	std:: cout << "length: " << length << std::endl;

	send(fd, "HTTP/1.1 200 OK\r\n", 17, 0);
	// send(event.ident, "Content-Length: 1654\r\n", 20, 0);
	
	send(fd, &content, length, 0);

	// send(fd, "Content-Length: " + &sizeStr, 16, 0);
	// send(fd, &sizeStr, std::to_string(size).length(), 0);
	// send(fd, "\r\n", 2, 0);

	send(fd, "Content-Type: text/html; charset=UTF-8\r\n", 40, 0);
	send(fd, "\r\n\r\n", 4, 0);
	int ret = send(fd, responseNoFav.c_str(), responseNoFav.length(), 0);
	if (ret < 0)
		throw ServerException("failed send");
	std::cout << "ret: " << ret << std::endl;
}

// --------------------------------------------------------- client functions 
// --------------------------------------------------------------------------

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

void Server::closeClient(struct kevent& event)
{
	data::Request *storage;
	storage = (data::Request *)event.udata; 

	delete(storage);
	std::cout << "connection closed\n";
	close(event.ident); 
}

// --------------------------------------------------------- utils functions
// -------------------------------------------------------------------------

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

std::string Server::streamFile(std::string file)
{
	std::string responseNoFav;
	std::fstream    infile;

	// infile.open(fileName, std::fstream::in | std::fstream::out | std::fstream::app);
	infile.open(file, std::fstream::in);
	if (!infile)
		throw ServerException("Error: File not be opened for reading!");
	while (infile)     // While there's still stuff left to read
	{
		std::string strInput;
		std::getline(infile, strInput);
		responseNoFav.append(strInput);
		responseNoFav.append("\n");
	}
	infile.close();
	return (responseNoFav);
}

void Server::sendErrorAutoLength(int fd, struct kevent& event)
{
	std::string error404;
	error404 = streamFile("error404.html");
	
	int temp = error404.length();
	std::string fileLen = std::to_string(temp);
	std::string contentLen = "Content-Length: ";
	contentLen.append(fileLen);
	contentLen.append("\n");
	std::cout << RED "ContLen: " << contentLen << "\n" RES;

	std::string headerBlock = 	"HTTP/1.1 404 Not Found\n"
								"Content-Type: text/html\n";
	headerBlock.append(contentLen);
	headerBlock.append("\r\n\r\n");

	int ret = 0;
	ret = send(fd, headerBlock.c_str(), headerBlock.length(), 0);
	ret = send(fd, error404.c_str(), error404.length(), 0);

	// if (removeEvent(event, EVFILT_WRITE) == 1)
	// 	throw ServerException("failed kevent EV_DELETE client");
	// sleep(3);
	// closeClient(event);
	// printf("closed connection from write - error\n");   // test line 
}

// --------------------------------------------------------- get functions
// -----------------------------------------------------------------------

int Server::getSocket(void)
{
	return (_listening_socket);
}

int Server::getKq(void)
{
	return (_kq);
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