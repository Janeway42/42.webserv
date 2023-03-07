#include "../includes/WebServer.hpp"
#include "../includes/RequestParser.hpp"
#include "../includes/CgiData.hpp"

WebServer::WebServer(std::string const & configFileName)
{
    ConfigFileParser configFileData = ConfigFileParser(configFileName);
    _servers =  configFileData.servers;

	// ----------- create kq structure --------------------------
	struct kevent evSet;
	_kq = kqueue();
	if (_kq == -1)
		throw ServerException(("failed kq"));

	// ----------- loop to create all listening sockets ---------  

    std::vector<ServerData>::iterator it_server = _servers.begin();
    for (; it_server != configFileData.servers.cend(); ++it_server) {
    	it_server->setListeningSocket();
		EV_SET(&evSet, it_server->getListeningSocket(), EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
		if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1) 
			throw ServerException(("failed kevent start listening socket"));
	}
}

WebServer::~WebServer()
{
	close(_kq);

	std::vector<ServerData>::iterator it_server = _servers.begin();
    for (; it_server != _servers.cend(); ++it_server) {
		freeaddrinfo(it_server->getAddr());
	}
}

// --------------------------------------------------------- server main loop
// --------------------------------------------------------------------------

void WebServer::runServer()
{
	struct kevent evList[MAX_EVENTS];
	int loop1 = 0;

	while (1)
	{
		std::cout << "WHILE LOOP ------------------------------" << loop1 << std::endl;
		int nr_events = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);
	//	std::cout << "NR EVENTS: " << nr_events << std::endl;

		if (nr_events < 1)
			throw ServerException("failed number events");
		else
		{
			for (int i = 0; i < nr_events; i++)
			{
			//	std::cout << "filter: " << evList[i].filter << std::endl; // test line 

				if (evList[i].flags & EV_ERROR)
					std::cout << "Event error\n";
				else if (evList[i].ident == _listening_socket)
					newClient(evList[i]);
				else if (evList[i].filter == EVFILT_TIMER)
					handleTimeout(evList[i]);
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
						sendResponse(evList[i]);
				}
				// std::cout << std::endl;
			}
		}	
		std::cout << std::endl;
		loop1++;
	}
}

// --------------------------------------------------------- server functions 
// --------------------------------------------------------------------------

void WebServer::handleTimeout(struct kevent &event)
{
	Request* storage = (Request*)event.udata;

	storage->setError(3);      // -------------------    408 Request Timeout
	std::cout << "Unable to process, takes too long\n";

	if ((storage->getCgiData()).getPipesDone() == true)
		(storage->getCgiData()).closeFileDescriptors(event);  // close the pipes if the parent times out. 

	struct kevent evSet;
	EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_WRITE - success parsing");
}

void WebServer::readRequest(struct kevent& event)
{
	char buffer[50];
	memset(&buffer, '\0', 50);
	Request* storage = (Request*)event.udata;

	int ret = recv(event.ident, &buffer, sizeof(buffer) - 1, 0);
	if (ret < 0)
	{
		std::cout << "failed recv\n";
		storage->setError(1);      // -------------------    400 Bad request
		std::cout << "400 Bad Request - send error message back to client\n";
	}	
	// // for cgi ret might be 0 - might need to add later, but for kqueue it will never be 0 
	if (event.ident == (storage->getCgiData()).getPipeCgiOut())
	{
		// put it in a temporary buffer from where write can get it. 
	}
	else
	{
		// add inside the append function, once the header is done, that if cgi then start the pipes. 
		if (storage->getError() == 0)
			storage->appendToRequest(buffer);
		if (storage->getError() != 0 || storage->getDoneParsing() == true)
		{
			struct kevent evSet;
			EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
			if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
				throw ServerException("failed kevent add EVFILT_WRITE - error parsing");
		}
	}
}

void WebServer::sendResponse(struct kevent& event)
{
	Request *storage = (Request *)event.udata;

	// remove READ only once in write (be it success, failure or timeout )
	if (removeEvent(event, EVFILT_READ) == 1)
		throw ServerException("failed kevent remove EVFILT_READ - in WRITE");

	if (event.ident == (storage->getCgiData()).getPipeCgiIn())
	{
		// when and how do we send the header to the client??????
		if (storage->getCgiData().getTempBuffer() != NULL)
		{
			// send the temporary buffer to the client 
			// set buffer to NULL
		}
	}

	if (storage->getError() != 0)
	{
		storage->getResponseData().setResponse(event);
		sendFile(event);

		storage->getCgiData().closeFileDescriptors();
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client - send error");
		closeClient(event);
		std::cout << "closed connection from write - error\n";
	}
	else if (storage->getError() == 0 && storage->getDoneParsing() == true)
	{
		processResponse(event);
		if (removeEvent(event, EVFILT_WRITE) == 1)
			throw ServerException("failed kevent EV_DELETE client - send success");
		closeClient(event);
		std::cout << "closed connection from write - done\n";
	}
}


void WebServer::processResponse(struct kevent& event)
{
	Request *storage = (Request *)event.udata;

	if (((storage->getRequestData()).getRequestMethod()).compare("GET") == 0)
		doGet(event);
	// else if (((storage->getRequestData()).getRequestMethod()).compare("POST") == 0)
	// 	doPost(event);  -- send to CGI
	// else if (((storage->getRequestData()).getRequestMethod()).compare("DELETE") == 0)
	// 	doDelete(event);
	else  
		doNotAllowed(event);
}

void WebServer::doGet(struct kevent& event)
{	
	Request *storage = (Request *)event.udata;

	storage->getResponseData().setResponse(event);
	sendFile(event);
}

// void WebServer::doPost(struct kevent& event)   // to be completed
// {
// 	// Request *storage = (Request *)event.udata;
// }

// void WebServer::doDelete(struct kevent& event)  // to be completed 
// {	
// 	// Request *storage = (Request *)event.udata;
// }

void WebServer::doNotAllowed(struct kevent& event)
{
	Request *storage = (Request *)event.udata;
	
	storage->setError(2); // 405 Method not allowed 
	storage->getResponseData().setResponse(event);
	sendFile(event);
}

// ------------------------------------------------------new client functions 
// --------------------------------------------------------------------------

void WebServer::newClient(struct kevent event)
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

	Request *storage = new Request(fd);
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_READ");

	int timer = 30 * 1000;
	EV_SET(&evSet, fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timer, storage);
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL)  == -1)
		throw ServerException("failed kevent add EVFILT_TIMEOUT");
}

void WebServer::closeClient(struct kevent& event)
{
	Request *storage;
	storage = (Request *)event.udata;

	close(event.ident); 
	std::cout << "connection closed\n";
	delete(storage);
}

// --------------------------------------------------------- utils functions
// -------------------------------------------------------------------------

int WebServer::removeEvent(struct kevent& event, int filter)
{
	struct kevent evSet;
	Request *storage = (Request *)event.udata;

	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		std::cout << "erno: " << errno << std::endl;       
		std::cout << "filter: " << filter << std::endl;                      // test line - to be removed 
		throw ServerException("failed kevent EV_DELETE client");
	}
	return (0);
}

void WebServer::sendFile(struct kevent& event)
{
	Request *storage = (Request *)event.udata;

	std::string message = (storage->getResponseData()).getFullResponse();

	int length = message.length();
	int ret = send(event.ident, message.c_str(), length, 0);
	if (ret == -1)
		throw ServerException("Send failed\n");
	std::cout << "ret: " << ret << std::endl;
}

void WebServer::timeoutCheck(struct kevent & event)
{
	Request *storage = (Request *)event.udata;
	std::time_t elapsedTime = std::time(NULL);

	if (elapsedTime - storage->getTime() > 30)
	{
		std::cout << "Unable to process request, it takes too long!\n";
		storage->setError(3);      // -------------------    408 Request Timeout
		if ((storage->getCgiData()).getPipesDone() == true)
			(storage->getCgiData()).closeFileDescriptors(event);  // close the pipes if the parent times out. 
	}
}

// --------------------------------------------------------- get functions
// -----------------------------------------------------------------------

int WebServer::getKq(void)
{
	return (_kq);
}
