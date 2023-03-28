#include "../includes/WebServer.hpp"

#include <stdio.h>		// jaka temo, for printf

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
		throw ServerException(("failed kq"));// todo: exceptions call the destructor to delete memory?

	// ----------- loop to create all listening sockets ---------
	std::vector<ServerData>::iterator it_server;
	for (it_server = _servers.begin(); it_server != _servers.end(); ++it_server) {
		it_server->setListeningSocket();
		EV_SET(&evSet, it_server->getListeningSocket(), EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
		if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
			throw ServerException(("failed kevent start listening socket")); // for error:: getaddrinfo uses malloc - freeaddrinfo is needed to free
	}
	std::cout  << "LEAVING WEBSERV CONSTRUCTOR" << std::endl;
}

WebServer::~WebServer()
{
	close(_kq);

	std::vector<ServerData>::iterator it_server = _servers.begin();
	for (; it_server != _servers.end(); ++it_server) {
		freeaddrinfo(it_server->getAddr());
	}
	std::cout << "WebServer Destructor" << std::endl;

}



// --------------------------------------------------------- server main loop
// --------------------------------------------------------------------------

void WebServer::runServer()
{
	struct kevent evList[MAX_EVENTS];
	int i;

//	int loop1 = 0;
	while (1)
	{
	//	std::cout << "WHILE LOOP ------------------------------" << loop1 << std::endl;
		int nr_events = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);

		if (nr_events < 1)
			throw ServerException("failed number events"); // for error:: it needs to return to kevent and try to get the events again. Thus nothing :D
		else
		{
			for (i = 0; i < nr_events; i++)
			{
				ServerData* specificServer = getSpecificServer(evList[i].ident);

				if (evList[i].flags & EV_ERROR)
					std::cout << "Socket was deleted\n";
				else if (specificServer != NULL)
					newClient(evList[i], specificServer);
				else if (evList[i].filter == EVFILT_TIMER)
					handleTimeout(evList[i]);
				else if (evList[i].filter == EVFILT_READ)
				{
					if (evList[i].flags & EV_EOF)  
					{
						std::cout << RED "OEF from ReadRequest, close the reading FD: " << evList[i].ident << "\n" RES;
						removeFilter(evList[i], EVFILT_READ, "failed kevent EV_EOF - EVFILT_READ");
						// if it is CGI, create the response and enable filter WRITE to client
						Request *storage = (Request *)evList[i].udata;			// added Jaka
						if (storage->getResponseData().getIsCgi() == true) {	// added Jaka
							storage->getResponseData().setResponse(evList[i]);	// added Jaka
							addFilter(storage->getFdClient(), evList[i], EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, success on _fd_out[0]");   //  this allows client write 
						}
						//closeClient(evList[i]);	// outcommented by jaka, it should still close if there is no cgi ???
					}
					else
						readRequest(evList[i]);
				}
				else if (evList[i].filter == EVFILT_WRITE)
				{
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
		// std::cout << std::endl;
//		loop1++;
	}
}

// --------------------------------------------------------- server functions 
// --------------------------------------------------------------------------

void WebServer::handleTimeout(struct kevent &event)
{
	Request *storage = (Request *)event.udata;

	if (storage->getHttpStatus() == NO_STATUS && storage->getDone() != true)  // this is just for extra precaution
	{
		storage->setHttpStatus(REQUEST_TIMEOUT);
		std::cout << "Unable to process, it takes too long\n";
		addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE - handle timeout");
	}
}

void WebServer::readRequest(struct kevent& event)
{
	//std::cout << "Start READ REQUEST\n";

	char buffer[BUFFER_SIZE];
	memset(&buffer, '\0', BUFFER_SIZE); // cpp equivalent ? 
	Request* storage = (Request*)event.udata;

	if ((int)event.ident == (storage->getCgiData()).getPipeCgiOut_0())  // read from CGI - we get the info // the event belong to the pipe fd: _fd_out[0]
	{
		size_t ret = read(event.ident, &buffer,  BUFFER_SIZE - 1);
		std::cout << "  ---> read from CGI, ret: " << ret << "\n";		// jaka
		if (ret < 0)
		{
			std::cout << "failed recv in pipe from cgi\n";
			storage->setHttpStatus(INTERNAL_SERVER_ERROR);
			storage->getResponseData().setResponse(event);
			addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, ret < 0 on _fd_out[0]");  //  this allows client write 
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0 on _fd_out[0]"); // this removes the pipe fd 
			std::cout << "sent error message back to client\n";
		}
		else
		{
			if (ret != 0) {
				// Jaka: Keep appending to the string _responseBody
				std::string tempStr = storage->getRequestData().getCgiBody();
				tempStr.append(buffer, ret);
				//std::cout << "BODY FROM CGI [\n" << CYN << tempStr << RES "]\n";		// jaka
				storage->getRequestData().setCgiBody(tempStr);
			}
			if (ret == 0 || buffer[ret] == EOF)	// Jaka: The part "buffer[ret] == EOF" is not usable, I think
			{									// 		 Apparently it never comes here, because read never returns a zero
												//		 Instead, it goes to Kq READ == OEF
				std::cout << " Ret 0 from CGI, prepare the response\n";
				storage->getResponseData().setResponse(event);
				addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, success on _fd_out[0]");   //  this allows client write 
				removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, success on _fd_out[0]"); // this removes the pipe fd 
			}
		}
	}

	else if ((int)event.ident == storage->getFdClient()) // the event belongs to the client fd 
	{
		int ret = recv(event.ident, &buffer, BUFFER_SIZE - 1, 0);

		if (ret <= 0) // kq will NEVER send a READ event if there is nothing to receive thus ret == 0 will never happen  
		{
			std::cout << "failed recv\n";
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0, _fdClient");
			removeFilter(event, EVFILT_TIMER, "failed kevent EV_DELETE, EVFILT_TIMER, read < 0, _fdClient");
			closeClient(event);
		}
		else if (storage->getDone() == false)
		{
			storage->appendToRequest(buffer, event);
			//std::cout << CYN "    returned from ATR(), _parsingDone: " << storage->getDone() << ", isCGI: " << storage->getResponseData().getIsCgi() << "\n" RES;


			// if (storage->getHttpStatus() != NO_STATUS || storage->getDone() == true)				// new Jaka: getIsCGI()
			if (storage->getHttpStatus() != NO_STATUS || (storage->getDone() == true && storage->getResponseData().getIsCgi() == false))
			{
				if (storage->getHttpStatus() != NO_STATUS)
					std::cout << "error parsing - sending response - failure, error " << storage->getHttpStatus() << "\n";// TODO JOYCE MAP ENUM TO STRING
				else if (storage->getDone() == true) {
					std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Done parsing ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
					std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Sending response ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
					//std::cout << "response success\n";
				}
				storage->getResponseData().setResponse(event);
				addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, success READ");
				removeFilter(event, EVFILT_READ, "failed kevent eof - read failure");
			}
			else if (storage->getDone() == true && storage->getResponseData().getIsCgi() == true) {
				removeFilter(event, EVFILT_READ, "failed kevent eof - read failure"); // ??? jaka
				chooseMethod_StartCGI(event, storage);
			}
		}
	}
}


void WebServer::sendResponse(struct kevent& event) 
{
	std::cout << "Start SEND RESPONSE\n";

	Request *storage = (Request*)event.udata;
	std::string buffer;

	// 																	  Jaka changed the name to:  getPipeCgiIn_1()
	if ((int)event.ident == (storage->getCgiData()).getPipeCgiIn_1())  // write to CGI - we send the info // the event belong to the pipe fd: _fd_in[1]
	{
		std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ This event FD belongs to CGI, write to CGI  ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
		//buffer = storage->getResponseData().getResponseBody();
		// Jaka: Set buffer string to the point, from where previous chunk of body was sent
		std::cout << "       getBody().length() " << storage->getRequestData().getBody().length() << "\n";
		std::cout << "                getBody() [" << BLU << storage->getRequestData().getBody() << RES "]\n";

		// buffer = storage->getResponseData().getResponseBody().substr(storage->getCgiData().getBytesToCgi()); // changed jaka: If POST, the _body is to be sent to cgi
		//sleep(2);
		buffer = storage->getRequestData().getBody().substr(storage->getCgiData().getBytesToCgi());				//  If GET, I also put the query string in the _body
																												//	So now it comes to CGI as ENV and as BODY
		ssize_t ret = write(storage->getCgiData().getPipeCgiIn_1(), buffer.c_str(), buffer.length());
		std::cout << "        Returned write: " << ret << "\n";

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
			std::cout << RES "    current sent getBytesToCGI " << storage->getCgiData().getBytesToCgi() << "\n" RES;

			if (storage->getCgiData().getBytesToCgi() == (storage->getRequestData()).getBody().length())
			{
				std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ CGI Response sent ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
				removeFilter(event, EVFILT_WRITE, "failed kevent EV_DELETE, EVFILT_WRITE - success on _fd_in[1]");
				close(event.ident);     // if CGI needs it 
				storage->getCgiData().resetPipeIn();
			}
		}
	}

	// SENDING CONTENT TO A CLIENT:
	// The current 'content' string length keeps decreasing, the value of 'sentSoFar' bytes keeps increasing
	// First, stores the whole response into 'content' and gets its current length.
	// Then sends a chunk. If successfully sent, checks the return value (bytes sent).
	// Increases the value of 'bytes sentSoFar'
	// Keeps deleting the sent chunk from the content string, until it has zero length.
	else if ((int)event.ident == storage->getFdClient()) // the event belongs to the client fd 
	{
		std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ This event FD belongs to CLIENT, send to CLIENT  ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
		//std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Sleeping ...  ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;	// jaka
		//sleep(2); // jaka

		std::string content = storage->getResponseData().getFullResponse();
		std::cout << GRN << "Full Response size: " RES << content.size() << RES << std::endl;
		std::cout << GRN << "Full Response Content:\n[\n" RES << content<< GRN "]\n" RES << std::endl;
		unsigned long myRet = 0;

		std::cout << CYN << "SENDING CHUNK,  remaining response length: " << content.size() << RES << "\n";
		myRet = send(event.ident, content.c_str(), content.size(), 0);

		// Corina :: this would be version without override - as I see people just closing connection if send fails  
		if (myRet < 0)
		{
			removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, ret < 0 on _fdClient");
			removeFilter(event, EVFILT_TIMER, "failed kevent, EV_DELETE, EVFILT_TIMER, ret < 0 on _fdClient");
			closeClient(event);
		}
		else
		{
			if (myRet > 0)
			{
				storage->getResponseData().increaseSentSoFar(myRet);
				storage->getResponseData().eraseSentChunkFromFullResponse(myRet);
				std::cout << CYN << "             Sent chunk: " << myRet << ",  sentSoFar " << storage->getResponseData().getSentSoFar() << "\n" << RES;
				std::cout << CYN << "  remaining contentSize: " << content.size() << "\n" << RES;
			}
			if (content.size() == 0)
			{
				std::cout << CYN << "ALL SENT, CLOSE CLIENT" RES "\n";
				removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, success on _fdClient");
				removeFilter(event, EVFILT_TIMER, "failed kevent, EV_DELETE, EVFILT_TIMER, success on _fdClient");
				closeClient(event);
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

	Request *storage = new Request(fd, specificServer);
	storage->getRequestData().setKqFd(getKq());	// Jaka: I need to store the value of kqueue-FD for later, to create pipes for CGI

	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent EV_ADD, EVFILT_READ, new client");

	int time = 5 * 1000;     // needs to be 30 for final version -----------------------------------------
	EV_SET(&evSet, fd, EVFILT_TIMER, EV_ADD, 0, time, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent EV_ADD, EVFILT_TIMER, new client");
	std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ New client connection ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
}

void WebServer::closeClient(struct kevent& event)
{
	Request *storage;
	storage = (Request *)event.udata;

	storage->getCgiData().closePipes();  // if any pipes are still open, function cleans them out 
	close(event.ident);
	std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Connection closed ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
	delete(storage);
}

// --------------------------------------------------------- utils functions
// -------------------------------------------------------------------------

ServerData * WebServer::getSpecificServer(int fd)
{
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

int WebServer::getKq(void)
{
	return (_kq);
}





void	WebServer::chooseMethod_StartCGI(struct kevent event, Request * storage) {
	std::cout << RED "Start ChooseMethodStartCGI()\n" RES ;
	// if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "") {
	if (storage->getRequestData().getRequestMethod() == "GET" && storage->getRequestData().getQueryString() != "") {
		std::cout << RED "     start GET, callCGI\n" RES ;
		// callCGI(getRequestData(), fdClient);
		storage->callCGI(event);
	}
	if (storage->getRequestData().getRequestMethod() == "POST")
			storage->callCGI(event);
	if (storage->getRequestData().getRequestMethod() == "DELETE") {
		std::cout << GRN_BG << "DELETE METHOD" << RES << std::endl;
		/* DELETE deletes a resource (specified in URI) */
		if (storage->pathType(storage->getRequestData().getPath()) != REG_FILE) {
			std::cout << RED_BG << "ERROR 404 Not Found" << RES << std::endl;
			// status error 404 Not Found -> Server cannot find the requested resource.
		}
		// cgi or just delete the file literally? It's not with CGI: Your program should call the CGI with the file requested as first argument.
		// How to delete a file froma  direcory: https://codescracker.com/cpp/program/cpp-program-delete-file.htm#:~:text=To%20delete%20any%20file%20from,used%20to%20delete%20a%20file.
		if (remove(storage->getRequestData().getPath().c_str()) != 0) {
			std::cout << RED_BG << "ERROR 204 No Content" << RES << std::endl;
			// 500 Internal Server Error -> Server encountered an unexpected condition that prevented it from fulfilling the request.
		}
			/* On successful deletion, it returns HTTP response status code 204 (No Content) */
		// _doneParsing = true;    // 
	}
}
