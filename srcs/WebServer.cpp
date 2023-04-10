#include <stdio.h>		// jaka temP, for printf

#include "WebServer.hpp"

WebServer::WebServer(std::string const & configFileName)
{
	ConfigFileParser configFileData = ConfigFileParser(configFileName);

	_servers = configFileData.servers;
	std::cout  << "Server blocks quantity: " << configFileData.numberOfServerBlocks() << std::endl;
	std::cout  << "Location block quantity: " << configFileData.numberOfLocationBlocks() << std::endl;

	// ----------- create kq structure --------------------------
	struct kevent evSet;
	_kq = kqueue();
	if (_kq == -1)
		throw ServerException(("failed kq - exit server"));

	// ----------- loop to create all listening sockets ---------
	std::vector<ServerData>::iterator it_server;
	//std::cout << "before vector loop ---------------\n";
	for (it_server = _servers.begin(); it_server != _servers.end(); ++it_server)
	{
		try
		{
			it_server->setListeningSocket();
		}
		catch (std::exception & e) {
        	std::cout << RED << e.what() << RES << std::endl;
		// catch (...)
		// {
		// 	throw ServerException("failed addr - exit webserv\n");
		}
		EV_SET(&evSet, it_server->getListeningSocket(), EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
		if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
			throw ServerException(("failed kevent start listening socket")); // getaddrinfo uses malloc - freeaddrinfo is needed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! - TO DO
	}
    std::cout << CYN << GRY_BG << "WebServer Overloaded Constructor" << RES << std::endl;
}

WebServer::~WebServer()
{
	close(_kq);
	std::vector<ServerData>::iterator it_server = _servers.begin();
	for (; it_server != _servers.end(); ++it_server) {
		if (it_server->getAddr() != NULL){
			std::cout << "_addr: " << it_server->getAddr() << std::endl;
			freeaddrinfo(it_server->getAddr());
			}
	}
    std::cout << CYN << GRY_BG << "WebServer Destructor" << RES << std::endl;
}

// --------------------------------------------------------- server main loop
// --------------------------------------------------------------------------

void WebServer::runServer()
{
	struct kevent evList[MAX_EVENTS];
	int i;

	int loop1 = 0;
	while (signalCall == false)
	{
		std::cout << "----------------------------------------------------------------------------------------------------------------- WHILE LOOP " << loop1 << std::endl; // Corina: I need these for testing purposes
		int nr_events = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);

		// std::cout << "nr of events: " << nr_events << std::endl;

		if (nr_events < 1)
			throw ServerException("failed number events");
		else
		{
			for (i = 0; i < nr_events; i++)
			{
				ServerData* specificServer = getSpecificServer(evList[i].ident);

				if (evList[i].flags & EV_ERROR)
				{
					std::cout << "Socket was deleted\n";
					std::cout << "Erno: " << errno << std::endl;  // for testing purposes - to be erased
				}
				else if (specificServer != NULL)
					newClient(evList[i], specificServer);
				else if (evList[i].filter == EVFILT_TIMER)
				{
					std::cout << "----------------------------------------------------------------------------------------------------------------- TIMER\n"; // Corina: I need these for testing purposes
					handleTimeout(evList[i]);
				}
				else if (evList[i].filter == EVFILT_READ)
				{
					std::cout << "----------------------------------------------------------------------------------------------------------------- READ\n";
					if (evList[i].flags & EV_EOF)
					{
						std::cout << RED "EOF from ReadRequest, close the reading FD: " << evList[i].ident << "\n" RES;
						Request *storage = (Request *)evList[i].udata;
						removeFilter(evList[i], EVFILT_READ, "failed kevent EV_EOF - EVFILT_READ");
						if (storage->getCgiData().getIsCgi() == true) // pipe // this is specifically for ret = 0 && EV_EOF
						{
							std::cout << "EOF READ pipe\n";
							close(evList[i].ident);
							addFilter(storage->getFdClient(), evList[i], EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, EOF READ _fd_out[0]");   //  this allows client write
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
					std::cout << "----------------------------------------------------------------------------------------------------------------- WRITE\n";
					if (evList[i].flags & EV_EOF)
					{
						removeFilter(evList[i], EVFILT_WRITE, "failed kevent EV_EOF - EVFILT_WRITE");
						closeClient(evList[i]);
					}
					else
						sendResponse(evList[i]);
				}
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
	Request *storage = (Request *)event.udata;

	// std::cout << "existing HTTP status: " << storage->getHttpStatus() << std::endl;
	storage->setHttpStatus(REQUEST_TIMEOUT);
	std::cout << "Unable to process, it takes too long - TIMER\n";
	storage->getCgiData().closePipes();
	// std::cout << "done: " << storage->getDone() << std::endl;
	if (storage->getDone() == false)
	{
		addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE - handle timeout");
		removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ - handle timeout");
	}
}

void WebServer::readRequest(struct kevent& event)
{
	//std::cout << "Start READ REQUEST\n";

	char buffer[BUFFER_SIZE];
	std::memset(&buffer, '\0', BUFFER_SIZE);
	Request* storage = (Request*)event.udata;

	// PIPE EVENT - _fd_out[0] - we READ from CGI
	// --------------------------------------------------------------
	if ((int)event.ident == (storage->getCgiData()).getPipeCgiOut_0())
	{
		size_t ret = read(event.ident, &buffer,  BUFFER_SIZE - 1);
		//std::cout << "  ---> read from CGI, ret: " << ret << "\n";		// jaka
		if (ret < 0)
		{
			std::cout << "failed read in pipe _fd_out[0] from cgi\n";
			storage->setHttpStatus(INTERNAL_SERVER_ERROR);
			addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, ret < 0 on _fd_out[0]");  //  this allows client write 
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0 on _fd_out[0]"); // this removes the pipe fd 
			std::cout << "sening error message back to client\n";
		}
		else
		{
			if (ret != 0)
			{
				std::string tempStr = storage->getRequestData().getCgiBody();
				tempStr.append(buffer, ret);
				//std::cout << "BODY FROM CGI [\n" << CYN << tempStr << RES "]\n";		// jaka
				storage->getRequestData().setCgiBody(tempStr);
			}
		}
	}
	// CLIENT EVENT - _fdClient
	// --------------------------------------------------------------
	else if ((int)event.ident == storage->getFdClient())
	{
		int ret = recv(event.ident, &buffer, BUFFER_SIZE - 1, 0);
		//std::cout << "    recv: " << ret << "\n";

		if (ret <= 0) // kq will NEVER send a READ event if there is nothing to receive thus ret == 0 will never happen  
		{
			std::cout << "failed recv in client fd\n";
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0, _fdClient");
			removeFilter(event, EVFILT_TIMER, "failed kevent EV_DELETE, EVFILT_TIMER, read < 0, _fdClient");
			closeClient(event);
		}
		else if (storage->getDone() == false)
		{
			storage->appendToRequest(buffer, ret);
			//std::cout << CYN "    returned from ATR(), _parsingDone: " << storage->getDone() << ", isCGI: " << storage->getResponseData().getIsCgi() << "\n" RES;


			// if (storage->getHttpStatus() != NO_STATUS || storage->getDone() == true)				// new Jaka: getIsCGI()
			if (storage->getHttpStatus() != NO_STATUS || (storage->getDone() == true && storage->getCgiData().getIsCgi() == false))
			{
				//std::cout << CYN "           ReadRequest: B)\n" RES;
				if (storage->getHttpStatus() != NO_STATUS) {
                    std::cout << YEL << "storage->getCgiData().getIsCgi(): " << storage->getCgiData().getIsCgi() << "\n" << RES;
                    std::cout << RED << "error parsing - sending response - failure, error " << storage->getHttpStatus()  << "\n" << RES;
                } else if (storage->getDone() == true)
					std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Done parsing ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;

				addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, success READ");
				removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE EVFILT_READ - success READ");
			}
			else if (storage->getDone() == true && storage->getCgiData().getIsCgi() == true) {
				std::cout << CYN "          ReadRequest: C) Done receiving the request, start CGI\n" RES;
				removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE EVFILT_READ - read success - start cgi"); // ??? jaka
				chooseMethod_StartCGI(event, storage);
			}
		}
	}
	//std::cout << CYN "   ReadRequest: END)\n" RES;
}

/*
==1684==ERROR: AddressSanitizer: heap-use-after-free on address 0x61900000216c at pc 0x0001000624af bp 0x7ffeefbf51e0 sp 0x7ffeefbf51d8
READ of size 4 at 0x61900000216c thread T0
    #0 0x1000624ae in Request::getHttpStatus() RequestParser.cpp:494
    #1 0x1000c2732 in WebServer::handleTimeout(kevent&) WebServer.cpp:118
    #2 0x1000c092a in WebServer::runServer() WebServer.cpp:70
*/
void WebServer::sendResponse(struct kevent& event) 
{
	//std::cout << "Start SEND RESPONSE\n";

	Request *storage = (Request*)event.udata;
	// std::string buffer;
	std::vector<uint8_t> & tmpBody = storage->getRequestData().getBody();
	const char* buffer;	// the reqBody is now stored as vector, must go to write() as char*

	// 	PIPE EVENT - _fd_in[1] - we WRITE to CGI
	// --------------------------------------------------------------
	if ((int)event.ident == (storage->getCgiData()).getPipeCgiIn_1())
	{
		std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ This event FD belongs to CGI ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
        //buffer = storage->getResponseData().getResponseBody();
		// Jaka: Set buffer string to the point, from where previous chunk of body was sent
		//std::cout << "       getBody().length() " << storage->getRequestData().getBody().size() << "\n";
		//std::cout << "                getBody() [ temp disabled by jaka ]\n";
		// std::cout << "                getBody() [" << BLU << storage->getRequestData().getBody() << RES "]\n";

		std::vector<uint8_t>::iterator startPoint = tmpBody.begin() + storage->getCgiData().getBytesToCgi();
		std::vector<uint8_t>::iterator endPoint   = tmpBody.end();
		//std::vector<uint8_t> bodyChunk(startPoint, endPoint);
		// buffer = reinterpret_cast<const char *>(bodyChunk.data());
		buffer = reinterpret_cast<const char *>(&(*startPoint));	

		ssize_t ret = write(storage->getCgiData().getPipeCgiIn_1(), buffer, static_cast<std::size_t>(endPoint - startPoint));
		//std::cout << "        Returned write: " << ret << "\n";

		if (ret == -1)
		{
			std::cout << "    Send() response returned -1\n";
			storage->setHttpStatus(INTERNAL_SERVER_ERROR);
			storage->getResponseData().setResponse(event);
			addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, ret = -1 on _fd_in[1]");    //  this allows client write 
			removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE failure on _fd_in[1]");  // this removes the pipe fd _fd_in[1]
		}
		else 
		{
			// Jaka -  keep track of what has been sent-> change file to be written if necessary
			storage->getCgiData().setBytesToCgi(ret);
			//std::cout << RES "    current sent getBytesToCGI " << storage->getCgiData().getBytesToCgi() << "\n" RES;

			if (storage->getCgiData().getBytesToCgi() == (storage->getRequestData()).getRequestContentLength())
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
			storage->getResponseData().setResponse(event);
			storage->getResponseData().setResponseDone(true);
			// std::cout << "----------- FULL RESPONSE: -----------------------\n" << storage->getResponseData().getFullResponse() << std::endl;
		}

		std::string content = storage->getResponseData().getFullResponse();
		//std::cout << GRN << "Full Response size: " RES << content.size() << RES << std::endl;
		//std::cout << GRN << "Full Response Content:\n[\n" RES << content<< GRN "]\n" RES << std::endl;
		unsigned long myRet = 0;

		std::cout << CYN << "SENDING CHUNK, response length before send: " << content.size() << RES << "\n";
		myRet = send(event.ident, content.c_str(), content.size(), 0);
		std::cout << CYN << "SEND return - sucessfully sent: " << myRet << RES << "\n";

		if (myRet < 0)
		{
			removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, ret < 0 on _fdClient");
			closeClient(event);
		}
		else
		{
			if (content.size() == myRet)
			{
				std::cout << CYN << "ALL SENT, CLOSE CLIENT" RES "\n";
				removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, success on _fdClient");
				closeClient(event);
			}
			else
			{
				storage->getResponseData().increaseSentSoFar(myRet);
				storage->getResponseData().eraseSentChunkFromFullResponse(myRet);
				//std::cout << CYN << "             Sent chunk: " << myRet << ",  sentSoFar " << storage->getResponseData().getSentSoFar() << "\n" << RES;
				//std::cout << CYN << "  remaining contentSize: " << content.size() << "\n" << RES;
			}
		}
	}
}

// --------------------------------------------------------- client functions 
// --------------------------------------------------------------------------

void WebServer::newClient(struct kevent event, ServerData * specificServer)
{
	struct kevent evSet;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);
	int opt_value = 1;
	int fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
	if (fd == -1)
		throw ServerException("failed accept");

	Request *storage = new Request(_kq, fd, specificServer);
	// storage->getRequestData().setKqFd(getKq());	 // moved to Request itself

	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent EV_ADD, EVFILT_READ, new client");

	int time = 30 * 1000;     // needs to be 30 for final version -----------------------------------------
	EV_SET(&evSet, fd, EVFILT_TIMER, EV_ADD, 0, time, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent EV_ADD, EVFILT_TIMER, new client");
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << GRN << " New client connection" << RES << " ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
    std::cout << "fd new client: " << fd << std::endl;
}

void WebServer::closeClient(struct kevent& event)
{
	Request *storage;
	storage = (Request *)event.udata;

	storage->getCgiData().closePipes();  // if any pipes are still open, function cleans them out
	removeFilter(event, EVFILT_TIMER, "failed kevent EV_DELETE EVFILT_TIMER - closeClient");
	std::cout << "fd that will be closed: " << event.ident << std::endl;
	std::cout << "original request: " << storage->getRequestData().getHttpPath() << std::endl;
	close(event.ident);
	std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Connection closed ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
    delete(storage);
}

// --------------------------------------------------------- utils functions
// -------------------------------------------------------------------------

ServerData * WebServer::getSpecificServer(int fd)
{
	std::cout << "size server inside run loop: " << _servers.size() << std::endl;

	std::vector<ServerData>::iterator it_server = _servers.begin();
	for (; it_server != _servers.end(); ++it_server) {
		if (it_server->getListeningSocket() == fd)
			return (&(*it_server));
	}
	return NULL;
}

void WebServer::addFilter(int fd, struct kevent& event, int filter, std::string errorMessage)
{
	Request *storage = (Request *)event.udata;

	struct kevent evSet;
	EV_SET(&evSet, fd, filter, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException(errorMessage);
}

void WebServer::removeFilter(struct kevent& event, int filter, std::string errorMessage)
{
	Request *storage = (Request *)event.udata;

	struct kevent evSet;
	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException(errorMessage);
}

void	WebServer::chooseMethod_StartCGI(struct kevent event, Request * storage) {
	std::cout << CYN << "Start ChooseMethodStartCGI()\n" << RES ;
	// if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "") {
	if (storage->getRequestData().getRequestMethod() == "GET" && storage->getRequestData().getQueryString() != "") {
		std::cout << "start GET, callCGI\n";
		// callCGI(getRequestData(), fdClient);
		storage->callCGI(event);
	}
	if (storage->getRequestData().getRequestMethod() == "POST")
			storage->callCGI(event);
	if (storage->getRequestData().getRequestMethod() == "DELETE") {
		std::cout << GRN_BG << "DELETE METHOD" << RES << std::endl;
		/* DELETE deletes a resource (specified in URI) */
		if (storage->pathType(storage->getRequestData().getURLPath()) != REG_FILE) {
			std::cout << RED_BG << "ERROR 404 Not Found" << RES << std::endl;
			// status error 404 Not Found -> Server cannot find the requested resource.
		}
		// cgi or just delete the file literally? It's not with CGI: Your program should call the CGI with the file requested as first argument.
		// How to delete a file froma  direcory: https://codescracker.com/cpp/program/cpp-program-delete-file.htm#:~:text=To%20delete%20any%20file%20from,used%20to%20delete%20a%20file.
		if (remove(storage->getRequestData().getURLPath().c_str()) != 0) {
			std::cout << RED_BG << "ERROR 204 No Content" << RES << std::endl;
			// 500 Internal Server Error -> Server encountered an unexpected condition that prevented it from fulfilling the request.
		}
			/* On successful deletion, it returns HTTP response status code 204 (No Content) */
		// _doneParsing = true;    //
	}
}

// --------------------------------------------------------- other functions
// -------------------------------------------------------------------------

std::string WebServer::streamFile(std::string file)
{
	std::string responseNoFav;
	std::fstream    infile;


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

bool WebServer::isListeningSocket(int fd)
{
	std::vector<ServerData>::iterator it_server = _servers.begin();
	for (; it_server != _servers.end(); ++it_server) {
	   if (fd == it_server->getListeningSocket())
	   	return (true);
	}
	return (false);
}

// --------------------------------------------------------- get functions
// -----------------------------------------------------------------------

// int WebServer::getKq(void)
// {
// 	return (_kq);
// }
