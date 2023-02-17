#include "includes/Server.hpp"

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

// ------------------------------------------------------------------------- server main loop
// ------------------------------------------------------------------------------------------

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
					std::cout << "READ\n";
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
					std::cout << "WRITE\n";
					if (evList[i].flags & EV_EOF)
					{
						if (removeEvent(evList[i], EVFILT_WRITE) == 1)
							throw ServerException("failed kevent eof - write");
						closeClient(evList[i]);
					}
					else
						writeResponse(evList[i]);
				}
				std::cout << std::endl;
			}
		}	
		std::cout << std::endl;
		loop1++;
	}
}

// --------------------------------------------------------------------------- read and write 
// ------------------------------------------------------------------------------------------

void Server::readRequest(struct kevent& event)
{
	char buffer[50];
	memset(&buffer, '\0', 50);
	data::Request *storage = (data::Request *)event.udata;

	int ret = recv(event.ident, &buffer, sizeof(buffer) - 1, 0);
	std::cout << "bytes read: " << ret << std::endl;                 // test line 
	if (ret < 0)
	{
		if (storage->getEarlyClose() == false) // if it fails 
		{
			std::cout << "failed recv\n";
			storage->setError(true);
			if (removeEvent(event, EVFILT_READ) == 1)
				throw ServerException("failed EVFILT_READ removal\n");
			std::cout << "send error message back to client\n";
		}
		else
			std::cout << "EVFILT_READ already closed\n";  // if it has been closed in write due to timeout  - test line
	}

	// for cgi ret might be 0 - might need to add later, but for kqueue it will never be 0 

	else if (storage->getEarlyClose() == false && storage->getDone() == false)
	{
		std::cout << "append buffer\n";
		storage->appendToRequest(buffer);

		if (storage->getError() == 1)  // 400 bad request 
		{
			std::cout << "error parsing - sending response - failure\n";
			if (removeEvent(event, EVFILT_READ) == 1)
				throw ServerException("failed kevent eof - read failure");
		}
		else if (storage->getDone() == true)
		{
			std::cout << "done parsing - sending response - success\n";
			if (removeEvent(event, EVFILT_READ) == 1)
				throw ServerException("failed kevent eof - read success");
			return ;
		}
	}
	else 
		std::cout << "just passing by\n";
}


void Server::writeResponse(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;

	std::time_t elapsedTime = std::time(NULL);
	if (elapsedTime - storage->getTime() > 1)
	{
		std::cout << "Unable to process request, it takes too long!\n";
		storage->setError(3); 										// 408 Request Timeout ---------
		storage->setEarlyClose(true);
		if (removeEvent(event, EVFILT_READ) == 1)
			throw ServerException("failed EVFILT_READ removal\n"); 
	}

	if (storage->getError() != 0)
	{
		storage->getAnswer().setResponse(event);
		sendFile(event);
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client - send error");
		closeClient(event);
		std::cout << "closed connection from write - error\n";
	}
	else if (storage->getError() == 0 && storage->getDone() == true)
	{
		processResponse(event);
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client - send success");
		closeClient(event);
		std::cout << "closed connection from write - done\n";
	}
}

void Server::processResponse(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;

	if (((storage->getRequestData()).getRequestMethod()).compare("GET") == 0)
		doGet(event);
	// else if (((storage->getRequestData()).getRequestMethod()).compare("POST") == 0)
	// 	doPost(event);
	// else if (((storage->getRequestData()).getRequestMethod()).compare("DELETE") == 0)
	// 	doDelete(event);
	else
		doNotAllowed(event);
}

void Server::doGet(struct kevent& event)
{	
	data::Request *storage = (data::Request *)event.udata;

	storage->getAnswer().setResponse(event);
	sendFile(event);
}

void Server::doPost(struct kevent& event)   // to be completed
{
	data::Request *storage = (data::Request *)event.udata;
}

void Server::doDelete(struct kevent& event)  // to be completed 
{	
	data::Request *storage = (data::Request *)event.udata;
}

void Server::doNotAllowed(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;
	
	storage->setError(2); // 405 Method not allowed 
	storage->getAnswer().setResponse(event);
	sendFile(event);
}

// -------------------------------------------------------------------- new client connection
// ------------------------------------------------------------------------------------------

void Server::newClient(struct kevent event)
{
	struct kevent evSet;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);

	std::cout << "make new client connection\n";
	
	int opt_value = 1;
	int fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
	if (fd == -1)
		throw ServerException("failed accept");

	data::Request *storage = new data::Request();
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_READ");
	EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_WRITE");
}

void Server::closeClient(struct kevent& event)
{
	data::Request *storage;
	storage = (data::Request *)event.udata; 

	close(event.ident); 
	std::cout << "connection closed\n";
	delete(storage);
}

// -------------------------------------------------------------------------- utils functions
// ------------------------------------------------------------------------------------------

int Server::removeEvent(struct kevent& event, int filter)
{
	struct kevent evSet;
	data::Request *storage = (data::Request *)event.udata;

	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		std::cout << "erno: " << errno << std::endl;                         // test line - to be removed 
		throw ServerException("failed kevent EV_DELETE client");
	}
	return (0);
}

void Server::sendFile(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;

	std::string message = storage->getAnswer().getFullResponse();
	int length = message.length();
	int ret = send(event.ident, &message, length, 0);
	if (ret == -1)
		throw ServerException("Send failed\n");
	std::cout << "ret: " << ret << std::endl;
}

// ---------------------------------------------------------------------------- get functions
// ------------------------------------------------------------------------------------------

int Server::getSocket(void)
{
	return (_listening_socket);
}

int Server::getKq(void)
{
	return (_kq);
}




// std::string Server::streamFile(std::string file)
// {
// 	std::string responseNoFav;
// 	std::fstream    infile;


// 	infile.open(file, std::fstream::in);
// 	if (!infile)
// 		throw ServerException("Error: File not be opened for reading!");
// 	while (infile)     // While there's still stuff left to read
// 	{
// 		std::string strInput;
// 		std::getline(infile, strInput);
// 		responseNoFav.append(strInput);
// 		responseNoFav.append("\n");
// 	}
// 	infile.close();
// 	return (responseNoFav);
// }

// void Server::sendResponseFile(struct kevent& event, std::string file)
// {
// 	data::Request *storage = (data::Request *)event.udata;
// 	std::string response;
// 	std::string headerBlock;
// 	response = streamFile(file);
	
// 	int temp = response.length();
// 	std::string fileLen = std::to_string(temp);
// 	std::string contentLen = "Content-Length: ";
// 	contentLen.append(fileLen);
// 	contentLen.append("\r\n");
// 	// std::cout << RED "ContLen: " << contentLen << "\n" RES;

// 	headerBlock = 	"HTTP/1.1 200 OK\n"
// 					"Content-Type: text/html\n";
// 	if (storage->getError() == true)
// 		headerBlock = 	"HTTP/1.1 404 Not Found\n"
// 						"Content-Type: text/html\n";
// 	headerBlock.append(contentLen);
// 	headerBlock.append("\r\n\r\n");
// 	headerBlock.append(response);

// 	int ret = send(event.ident, headerBlock.c_str(), headerBlock.length(), 0);
// 	if (ret == -1)
// 		throw ServerException("Send failed\n");
// 	std::cout << "ret: " << ret << std::endl;

// }

// void Server::sendImmage(struct kevent& event, std::string imgFileName)
// {
// 	std::cout << RED "FOUND extention .jpg or .png\n" RES;

// 	FILE *file;
// 	unsigned char *buffer;
// 	unsigned long imageSize;

// 	file = fopen(imgFileName.c_str(), "rb");
// 	if (!file)
// 	{
// 		std::cerr << "Unable to open file\n";
// 		return ;
//  	}

// 	fseek(file, 0L, SEEK_END);	// Get file length
// 	imageSize = ftell(file);
// 	fseek(file, 0L, SEEK_SET);

// 	std::string temp = std::to_string(imageSize);
// 	std::string contentLen = "Content-Length: ";
// 	contentLen.append(temp);
// 	contentLen.append("\r\n");

// 	std::string headerBlock = 	"HTTP/1.1 202 OK\r\n"
// 								"accept-ranges: bytes\r\n"
// 								"Content-Type: image/jpg\r\n";
// 	headerBlock.append(contentLen);
// 	headerBlock.append("accept-ranges: bytes");
// 	headerBlock.append("\r\n\r\n");

// 	buffer = (unsigned char *)malloc(imageSize);
// 	if (!buffer)
// 		{ fprintf(stderr, "Memory error!"); fclose(file); return ; }

// 	int ret = fread(buffer, sizeof(unsigned char), imageSize, file);
// 	std::cout << YEL "Returned fread:     " << ret << RES "\n";
	
// 	ret = send(event.ident, headerBlock.c_str(), headerBlock.length(), 0);
// 	ret = send(event.ident, reinterpret_cast <const char* >(buffer), imageSize, 0);
// 	std::cout << YEL "Image sent, returned from send() image: " << ret << RES "\n";
// 	fclose(file);
// 	free(buffer);
// }







