#include "WebServer.hpp"

WebServer::WebServer(std::string const & configFileName)
{
    try {
        ConfigFileParser configFileData = ConfigFileParser(configFileName);
        _servers = configFileData.servers;

        // ----------- create kq structure --------------------------
        struct kevent evSet;
        _kq = kqueue();
        if (_kq == -1)
            throw ServerException("Failed kq - exit server");

        // ----------- loop to create all listening sockets ---------
        std::vector<ServerData>::iterator it_server;
		int location = 0;
        for (it_server = _servers.begin(); it_server != _servers.end(); ++it_server)
        {
			int tempSocket = -1;
			tempSocket = checkExistingSocket(location, it_server->getListensTo(), it_server->getHost());
			if (tempSocket != -1)
				it_server->setExistingListeningSocket(tempSocket);	// socket already on kq, no need to add it again			
			else 
			{
			   	it_server->setListeningSocket();
            	EV_SET(&evSet, it_server->getListeningSocket(), EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
            	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
               		throw ServerException("Failed kevent start listening socket");
			}
			std::cout << "SERVER NAME: " << it_server->getServerName() << std::endl;
			std::cout << "HOST: " << it_server->getHost() << std::endl;
			std::cout << "PORT: " << it_server->getListensTo() << std::endl;
			std::cout << "LISTENING SOCKET: " << it_server->getListeningSocket() << std::endl;
			std::cout << "--------------------------------------------------------\n";
			location++;
        }
        std::cout << CYN << "WebServer Overloaded Constructor" << RES << std::endl;
    } catch (std::exception const & e) {
        throw ServerException(std::string("Failed to initialise webserv: ") + e.what());
    }
}

int WebServer::checkExistingSocket(int location, std::string port, std::string host)
{
	std::vector<ServerData>::iterator it_server;
	int i = 0;

	if (location > 0)
	{
		for (it_server = _servers.begin(); it_server != _servers.end(); ++it_server)
		{
			if (i != location)
			{
				if (it_server->getListensTo() == port)
				{
					if (it_server->getHost() == host)
						return (it_server->getListeningSocket());
				}
			}
		}
	}
	return (-1);
}

WebServer::~WebServer()
{
	try {
		std::vector<ServerData>::iterator it;
		for (it = _servers.begin(); it != _servers.end(); it++)
		{
			if (fcntl(it->getListeningSocket(), F_GETFD) != -1)
				close(it->getListeningSocket());
		}
		close(_kq);
		std::cout << CYN << "WebServer Destructor" << RES << std::endl;
	} catch (...) {
		throw ServerException(std::string("Failed to deinitialise webser JOYCE"));
	}
}

// --------------------------------------------------------- server main loop
// --------------------------------------------------------------------------

void WebServer::runServer()
{
	struct kevent evList[MAX_EVENTS];
	int i;

	int loop1 = 0;
	while (1)
	{
		std::cout << GRY << "----------------------------------------------------------------------------------------------------------------- WHILE LOOP " << loop1 << RES << std::endl;
		int nr_events = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);

		if (nr_events < 1 || signalCall == true)
			exitProgram(nr_events, evList);
		else
		{
			for (i = 0; i < nr_events; i++)
			{
				ServerData* specificServer = getSpecificServer(evList[i].ident);

				if (evList[i].flags & EV_ERROR)
					std::cout << "Socket was deleted\n";
				else if (specificServer != NULL)
					newClient(evList[i]);
				else if (evList[i].filter == EVFILT_TIMER)
				{
					std::cout << GRY << "----------------------------------------------------------------------------------------------------------------- TIMER" << RES << std::endl;
					handleTimeout(evList[i]);
				}
				else if (evList[i].filter == EVFILT_READ)
				{
					std::cout << GRY << "----------------------------------------------------------------------------------------------------------------- READ" << RES << std::endl;

					if (evList[i].flags & EV_EOF)
					{
						std::cout << "EOF from ReadRequest, close the reading FD: " << evList[i].ident << "\n";
						Request *storage = (Request *)evList[i].udata;
						removeFilter(evList[i], EVFILT_READ, "failed kevent EV_EOF - EVFILT_READ");
						if (storage->getCgiData().getIsCgi() == true) // <--eval--> this is specifically for ret = 0 && EV_EOF
						{
							std::cout << "EOF READ pipe. HTTP STATUS: " << storage->getHttpStatus() << "\n";
							close(evList[i].ident);
							if (addFilter(storage->getFdClient(), evList[i], EVFILT_WRITE) == 1)   //  this allows client write
								cleanAddFilterFail(storage->getFdClient(), evList[i], "failed kevent EV_ADD, EVFILT_WRITE, EOF READ _fd_out[0]");
						}
						else // client
						{
							std::cout << "EOF READ main client\n";
							closeClient(evList[i]);
						}
					}
					else
						readRequest(evList[i]);
				}
				else if (evList[i].filter == EVFILT_WRITE)
				{
					std::cout << GRY << "----------------------------------------------------------------------------------------------------------------- WRITE" << RES << std::endl;
					if (evList[i].flags & EV_EOF)
					{
                        std::cout << RED << "write finished" << std::endl;
						removeFilter(evList[i], EVFILT_WRITE, "failed kevent EV_EOF - EVFILT_WRITE");
						closeClient(evList[i]);
					}
					else {
                        std::cout << RED << "write sendResponse" << std::endl;
                        sendResponse(evList[i]);
                    }
				}
			}
		}	
		std::cout << std::endl;
		loop1++;
	}
}

// --------------------------------------------------------- server functions 
// --------------------------------------------------------------------------

int WebServer::exitProgram(int nr_events, struct kevent evList[MAX_EVENTS])
{
	for(int i = 0; i < nr_events; i++)
	{
	 	Request *storage = (Request *)evList[i].udata;
	 	if (storage && storage != NULL)
	 	{
	 		delete storage;
	 		storage = NULL;
		}
	}
	exit(0);
}

void WebServer::handleTimeout(struct kevent &event)
{
	Request *storage = (Request *)event.udata;

	std::cout << "Unable to process, it takes too long - TIMER\n";
	if (storage->getDone() == false)
	{
		storage->setHttpStatus(REQUEST_TIMEOUT);
		removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ - handle timeout");
	}
	else
		storage->setHttpStatus(GATEWAY_TIMEOUT);
	storage->getCgiData().closePipes();
	if (fcntl(event.ident, F_GETFD) != -1)
	{
		if (addFilter(event.ident, event, EVFILT_WRITE) == 1)
			cleanAddFilterFail(event.ident, event, "failed kevent EV_ADD, EVFILT_WRITE - handle timeout");
	}
	std::cout << "HANDLE TIMEOUT " << storage->getHttpStatus() << std::endl;
}

void WebServer::readRequest(struct kevent& event)
{
	char buffer[BUFFER_SIZE];
	std::memset(&buffer, '\0', BUFFER_SIZE);
	Request* storage = (Request*)event.udata;

	// PIPE EVENT - _fd_out[0] - we READ from CGI
	// --------------------------------------------------------------
	if ((int)event.ident == (storage->getCgiData()).getPipeCgiOut_0())
	{
		ssize_t ret = read(event.ident, &buffer,  BUFFER_SIZE - 1);
		if (ret < 0) // <--eval--> this checks for ret == -1, ret == 0 is checked in the main loop
		{
			std::cout << "failed read in pipe _fd_out[0] from cgi\n";
			storage->setHttpStatus(INTERNAL_SERVER_ERROR);
			if (addFilter(storage->getFdClient(), event, EVFILT_WRITE) == 1)  //  this allows client write 
				cleanAddFilterFail(storage->getFdClient(), event, "failed kevent EV_ADD, EVFILT_WRITE, ret < 0 on _fd_out[0]");
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0 on _fd_out[0]"); // this removes the pipe fd 
			std::cout << "sening error message back to client\n";
		}
		else
		{
			if (ret != 0)
			{
				std::string tempStr = storage->getRequestData().getCgiBody();
				tempStr.append(buffer, ret);
				storage->getRequestData().setCgiBody(tempStr);
			}
		}
	}
	// CLIENT EVENT - _fdClient
	// --------------------------------------------------------------
	else if ((int)event.ident == storage->getFdClient())
	{
		ssize_t ret = recv(event.ident, &buffer, BUFFER_SIZE - 1, 0);
		std::cout << "recv ret = " << ret << std::endl;
		std::cout << "buffer = ["   << buffer << "]" << std::endl;
		if (ret <= 0) // <--eval--> kq will NEVER send a READ event if there is nothing to receive thus ret == 0 will never happen  
		{
			std::cout << "failed recv in client fd\n";
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0, _fdClient");
			removeFilter(event, EVFILT_TIMER, "failed kevent EV_DELETE, EVFILT_TIMER, read < 0, _fdClient");
			closeClient(event);
		}
		else if (storage->getDone() == false)
		{
      std::cout << "recv ret before appendToRequest = " << ret << std::endl;
			storage->appendToRequest(buffer, ret);
			//std::cout << CYN "    returned from ATR(), _parsingDone: " << storage->getDone() << ", isCGI: " << storage->getResponseData().getIsCgi() << "\n" RES;

			std::cout << "HTTP STATUS: " << storage->getHttpStatus() << std::endl;
			std::cout << "CGI STATUS: " << storage->getCgiData().getIsCgi() << std::endl;

            // FOR LOGGING
            //std::cout << "Buffer from recv:[" << PUR << buffer << RES << "]" << std::endl;


			if ((storage->getHttpStatus() != NO_STATUS && storage->getHttpStatus() != OK) || (storage->getDone() == true && storage->getCgiData().getIsCgi() == false))
			{
//                std::cout << YEL << "storage->getHttpStatus(): " << storage->getHttpStatus() << "\n" << RES;
                if (storage->getHttpStatus() != NO_STATUS && storage->getHttpStatus() != OK) {
                    std::cout << YEL << "storage->getCgiData().getIsCgi(): " << storage->getCgiData().getIsCgi() << "\n" << RES;
                    std::cout << RED << "error parsing - sending response - failure, error " << storage->getHttpStatus()  << "\n" << RES;
                } else if (storage->getDone() == true)
					std::cout << GRN << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Done parsing ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << RES << std::endl;

				if (addFilter(event.ident, event, EVFILT_WRITE) == 1)
					cleanAddFilterFail(event.ident, event, "failed kevent EV_ADD, EVFILT_WRITE, success READ");
				removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE EVFILT_READ - success READ");
			}
			else if (storage->getDone() == true && storage->getCgiData().getIsCgi() == true) {
				std::cout << CYN "          ReadRequest: C) Done receiving the request, start CGI\n" RES;
				removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE EVFILT_READ - read success - start cgi");
				chooseMethod_StartCGI(event, storage);
			}
		}
	}
}



void WebServer::sendResponse(struct kevent& event) 
{
	Request *storage = (Request*)event.udata;
	std::vector<uint8_t> & tmpBody = storage->getRequestData().getBody();
	const char* buffer;

	// 	PIPE EVENT - _fd_in[1] - we WRITE to CGI
	// --------------------------------------------------------------
	if ((int)event.ident == (storage->getCgiData()).getPipeCgiIn_1())
	{
		std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ This event FD belongs to CGI ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;

		std::vector<uint8_t>::iterator startPoint = tmpBody.begin() + storage->getCgiData().getBytesToCgi();
		std::vector<uint8_t>::iterator endPoint   = tmpBody.end();
		buffer = reinterpret_cast<const char *>(&(*startPoint));	

		ssize_t ret = write(storage->getCgiData().getPipeCgiIn_1(), buffer, static_cast<std::size_t>(endPoint - startPoint));

		if (ret == -1) // <--eval--> this checks for ret == -1
		{
			storage->setHttpStatus(INTERNAL_SERVER_ERROR);
			storage->getResponseData().createResponse(event);
			if (addFilter(storage->getFdClient(), event, EVFILT_WRITE) == 1)    //  this allows client write 
				cleanAddFilterFail(storage->getFdClient(), event, "failed kevent EV_ADD, EVFILT_WRITE, ret = -1 on _fd_in[1]");
			else
				removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE failure on _fd_in[1]");  // this removes the pipe fd _fd_in[1]
		}
		else // <--eval--> this also accounts for ret = write() = 0 
		{
			storage->getCgiData().setBytesToCgi(ret);
			if (storage->getCgiData().getBytesToCgi() == (storage->getRequestData()).getRequestContentLength()) // ---- this checks if everything has been sent
			{
				std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ CGI Response sent ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
                removeFilter(event, EVFILT_WRITE, "failed kevent EV_DELETE, EVFILT_WRITE - success on _fd_in[1]");
				close(event.ident);
			}
		}
	}
	// SENDING CONTENT TO A CLIENT:
	// The current 'content' string length keeps decreasing, the value of 'sentSoFar' bytes keeps increasing
	// First, stores the whole response into 'content' and gets its current length.
	// Then sends a chunk. If successfully sent, checks the return value (bytes sent).
	// Increases the value of 'bytes sentSoFar'
	// Keeps deleting the sent chunk from the content string, until it has zero length.

	// CLIENT EVENT - _fdClient
	// --------------------------------------------------------------
	else if ((int)event.ident == storage->getFdClient())
	{
        std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ This event FD belongs to CLIENT ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;

        if (storage->getResponseData().getResponseDone() == false)
		{
			storage->getResponseData().createResponse(event);
			storage->getResponseData().setResponseDone(true);
		}

		std::string content = storage->getResponseData().getFullResponse();
		std::cout << CYN << "SENDING CHUNK, response length before send: " << content.size() << RES << "\n";
        ssize_t myRet = send(event.ident, content.c_str(), content.size(), 0);
		std::cout << CYN << "SEND return - sucessfully sent: " << myRet << RES << "\n";

		if (myRet < 0)
		{
			removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, ret < 0 on _fdClient");
			closeClient(event);
		}
		else
		{
			if (static_cast<ssize_t>(content.size()) == myRet) // <--eval--> this includes also the = 0 case ===> myRet == 0 and fullResponse.length() == 0
			{
				std::cout << CYN << "ALL SENT, CLOSE CLIENT" RES "\n";
				removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, success on _fdClient");
				closeClient(event);
			}
			else // <--eval--> includes also the = 0 case ===> if myRet == 0 and fullResponse.length() != 0
			{
				storage->getResponseData().increaseSentSoFar(myRet);
				storage->getResponseData().eraseSentChunkFromFullResponse(myRet);
			}
		}
	}
}

// --------------------------------------------------------- client functions 
// --------------------------------------------------------------------------

void WebServer::newClient(struct kevent event)
{
	struct kevent evSet;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);
	int opt_value = 1;

	int fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	while (fd != -1)
	{
		int failFcntl = fcntl(fd, F_SETFL, O_NONBLOCK);
		int failSet = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));

        Request *storage = new Request(_kq, event.ident, fd, _servers);
		if (failFcntl == - 1 || failSet == -1)
		{
			protectFirstFilterAdd(storage, fd);
			return ;
		}
		EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
		if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		{
			protectFirstFilterAdd(storage, fd);
			return ;
		}

		int time = 30 * 1000;     // TODO needs to be 30 for final version -----------------------------------------
		EV_SET(&evSet, fd, EVFILT_TIMER, EV_ADD, 0, time, storage); 
		if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		{
			protectFirstFilterAdd(storage, fd);
			return ;
		}

		std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << GRN << " New client connection" << RES << " ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
		std::cout << "fd new client: " << fd << std::endl;
		fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	}	
}

void WebServer::protectFirstFilterAdd(Request *storage, int fd)
{
	struct kevent evSet;

	std::string errorResponse = "HTTP/1.1 500 Internal Server Error\r\n"
						        "Content-Type: text/html\r\n"
    							"Content-Encoding: identity\r\n"
    							"Connection: close\r\n"
    							"Content-Length: 27\r\n"
								"\r\n"
								"500 Internal Server Error\r\n";

	storage->getResponseData().setResponseFull(errorResponse);

	EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		close(fd);
		delete(storage);
	}
	std::cout << "failed EV_READ new client" << std::endl;
}

void WebServer::closeClient(struct kevent& event)
{
	Request *storage;
	storage = (Request *)event.udata;

	storage->getCgiData().closePipes();
	if (fcntl(event.ident, F_GETFD) != -1)
		removeFilter(event, EVFILT_TIMER, "failed kevent EV_DELETE EVFILT_TIMER - closeClient");

	std::cout << "fd that will be closed: " << event.ident << std::endl;
	std::cout << "original request: " << storage->getRequestData().getHttpPath() << std::endl;

	close(event.ident);
	std::cout << PUR << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Connection closed ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << RES << std::endl;
    delete(storage);
}

// --------------------------------------------------------- utils functions
// -------------------------------------------------------------------------

ServerData * WebServer::getSpecificServer(int fd)
{
//	std::cout << "size server inside run loop: " << _servers.size() << std::endl;

	std::vector<ServerData>::iterator it_server = _servers.begin();
	for (; it_server != _servers.end(); ++it_server) {
		if (it_server->getListeningSocket() == fd)
			return (&(*it_server));
	}
	return NULL;
}

int WebServer::addFilter(int fd, struct kevent& event, int filter)
{
	Request *storage = (Request *)event.udata;

	struct kevent evSet;
	EV_SET(&evSet, fd, filter, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		return (1);
	return (0);
}

void WebServer::cleanAddFilterFail(int fd, struct kevent& event, std::string errorMessage)
{
	Request *storage = (Request *)event.udata;

	std::cout << errorMessage << std::endl;
	storage->getCgiData().closePipes();
	close(fd);
	delete storage;
}

void WebServer::removeFilter(struct kevent& event, int filter, std::string errorMessage)
{
	Request *storage = (Request *)event.udata;

	struct kevent evSet;
	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		std::cout << errorMessage << std::endl;
}

void	WebServer::chooseMethod_StartCGI(struct kevent event, Request * storage) {
	std::cout << CYN << "Start ChooseMethodStartCGI()\n" << RES ;
	if (storage->getRequestData().getRequestMethod() == GET) {
		std::cout << "start GET, callCGI\n";
		storage->callCGI(event);
	}
	if (storage->getRequestData().getRequestMethod() == POST)
			storage->callCGI(event);
	if (storage->getRequestData().getRequestMethod() == DELETE) {
		std::cout << GRN << "DELETE METHOD" << RES << std::endl;
		if (storage->pathType(storage->getRequestData().getURLPath()) != REG_FILE) {
			std::cout << RED << "ERROR 404 Not Found" << RES << std::endl;
		}
		if (remove(storage->getRequestData().getURLPath().c_str()) != 0) {
			std::cout << RED << "ERROR 204 No Content" << RES << std::endl;
		}
	}
}

// --------------------------------------------------------- other functions
// -------------------------------------------------------------------------

bool WebServer::isListeningSocket(int fd)
{
	std::vector<ServerData>::iterator it_server = _servers.begin();
	for (; it_server != _servers.end(); ++it_server) {
	   if (fd == it_server->getListeningSocket())
	   	return (true);
	}
	return (false);
}
