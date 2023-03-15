#include "../includes/WebServer.hpp"

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
            throw ServerException(("failed kevent start listening socket"));
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
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ " << __FUNCTION__ << " function called  ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
	struct kevent evList[MAX_EVENTS];
	int i;

//	int loop1 = 0;
	while (1)
	{
	//	std::cout << "WHILE LOOP ------------------------------" << loop1 << std::endl;
		int nr_events = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);
	//	std::cout << "NR EVENTS: " << nr_events << std::endl;

		if (nr_events < 1)
			throw ServerException("failed number events"); // what does this mean? does it just return to kevent?
		else
		{
			for (i = 0; i < nr_events; i++)
			{
			//	std::cout << "filter: " << evList[i].filter << std::endl; // test line 
				ServerData* specificServer = getSpecificServer(evList[i].ident);

				if (evList[i].flags & EV_ERROR)
					std::cout << "Event error\n";  // Corina - is there more that needs to happen here ?!!!!!!!
				else if (specificServer != NULL)
					newClient(evList[i], specificServer);
				else if (evList[i].filter == EVFILT_TIMER)
					handleTimeout(evList[i]);
				else if (evList[i].filter == EVFILT_READ)
				{
				//	std::cout << "READ\n";
					if (evList[i].flags & EV_EOF)  // if client closes connection 
					{
						// removeFilter(evList[i].ident, evList[i], EVFILT_READ, "failed kevent EV_EOF - EVFILT_READ");		// jaka: error, too many arguments
						removeFilter(evList[i], EVFILT_READ, "failed kevent EV_EOF - EVFILT_READ");
						closeClient(evList[i]);
					}
					else
						readRequest(evList[i]);
				}
				else if (evList[i].filter == EVFILT_WRITE)
				{
				//	std::cout << "WRITE\n";
					if (evList[i].flags & EV_EOF)
					{
						// removeFilter(evList[i].ident, evList[i], EVFILT_READ, "failed kevent EV_EOF - EVFILT_READ");			// jaka: error, too many arguments
						removeFilter(evList[i], EVFILT_WRITE, "failed kevent EV_EOF - EVFILT_WRITE");
						closeClient(evList[i]);
					}
					else
						sendResponse(evList[i]);
				}
				// std::cout << std::endl;
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

	if (storage->getError() == 0 && storage->getDone() != true)  // ?
	{
		storage->setError(4);      // -------------------    408 Request Timeout
		std::cout << "Unable to process, takes too long\n";
		addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE - handle timeout");
	}
}

void WebServer::readRequest(struct kevent& event)
{
	char buffer[BUFFER_SIZE];
	memset(&buffer, '\0', BUFFER_SIZE);
	Request* storage = (Request*)event.udata;

	if ((int)event.ident == (storage->getCgiData()).getPipeCgiOut())  // read from CGI - we get the info // the event belong to the pipe fd: _fd_out[0]
	{
		size_t ret = read(event.ident, &buffer,  BUFFER_SIZE);
		if (ret < 0)
		{
			std::cout << "failed recv in pipe from cgi\n";
			storage->setError(5);       // -------------------    500 Internal Server Error  

			storage->getResponseData().setResponse(event);
			addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, ret < 0 on _fd_out[0]");  //  this allows client write 
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0 on _fd_out[0]"); // this removes the pipe fd 
			std::cout << "sent error message back to client\n";
		}
		else
		{
			if (ret != 0)
				(storage->getResponseData()).setResponseBody(buffer);
			if (ret == 0 || buffer[ret] == EOF)
			{
				storage->getResponseData().setResponse(event);
				addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, success on _fd_out[0]");   //  this allows client write 
				removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, success on _fd_out[0]"); // this removes the pipe fd 
			}
		}
	}

	else if ((int)event.ident == storage->getFdClient()) // the event belongs to the client fd 
	{
		int ret = recv(event.ident, &buffer, BUFFER_SIZE - 1, 0);
		//	std::cout << "bytes read: " << ret << std::endl;                 // test line 
		if (ret <= 0) // kq will NEVER send a READ event if there is nothing to receive 
		{
			std::cout << "failed recv\n";
			storage->setError(5);       // -------------------    500 Internal Server Error  

			storage->getResponseData().setResponse(event);
			addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, ret < 0, _fdClient");
			removeFilter(event, EVFILT_READ, "failed kevent EV_DELETE, EVFILT_READ, ret < 0, _fdClient");
			std::cout << "sent error message back to client\n";
		}
	
		else if (storage->getDone() == false)
		{
			storage->appendToRequest(buffer, event.ident);

			if (storage->getError() != 0 || storage->getDone() == true)
			{
				if (storage->getError() != 0)
					std::cout << "error parsing - sending response - failure\n";
				else if (storage->getDone() == true)
					std::cout << "done parsing - sending response - success\n";

				storage->getResponseData().setResponse(event);
				addFilter(event.ident, event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, success READ");
				removeFilter(event, EVFILT_READ, "failed kevent eof - read failure");
			}
		}
	}
}

void WebServer::sendResponse(struct kevent& event) 
{
    Request *storage = (Request*)event.udata;
	std::string buffer;

	if ((int)event.ident == (storage->getCgiData()).getPipeCgiIn())  // write to CGI - we send the info // the event belong to the pipe fd: _fd_in[1]
	{
		buffer = storage->getResponseData().getResponseBody();
		ssize_t ret = write(storage->getCgiData().getPipeCgiIn(), buffer.c_str(), buffer.length());
		if (ret == -1)
		{
			storage->setError(4); // 500 system failure 
			storage->getResponseData().setResponse(event);
			addFilter(storage->getFdClient(), event, EVFILT_WRITE, "failed kevent EV_ADD, EVFILT_WRITE, ret = -1 on _fd_in[1]");    //  this allows client write 
			removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE failure on _fd_in[1]");  // this removes the pipe fd _fd_in[1]
		}
		else 
		{
			storage->getCgiData().setBytesToCgi(ret);
			// Jaka -  keep track of what has been sent-> change file to be written if necessary 
			if (storage->getCgiData().getBytesToCgi() == (storage->getRequestData()).getBody().length())
			{
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
		std::string content = storage->getResponseData().getFullResponse();
        //std::cout << GRN << "Full Response:\n" RES << storage->getResponseData().getFullResponse() << RES << std::endl;
        std::cout << GRN << "Full Response size:\n" RES << content.size() << RES << std::endl;
        unsigned long myRet;
		storage->getResponseData().setCurrentLength(content.size());

		std::cout << YEL"content size: " << content.size() << RES"\n";
		std::cout << YEL"START SENDING CHUNK,  remaining response length: " << storage->getResponseData().getCurrentLength() << RES"\n";

		myRet = send(event.ident, content.c_str(), content.size(), 0);
		if (myRet == std::string::npos) {	// Temporary, just to see. It can be probably removed (Jaka)
			std::cout << RED << "    myRet == std::string::npos" RES << "\n";
		}
		else if (myRet > 0) {
			storage->getResponseData().increaseSentSoFar(myRet);
			storage->getResponseData().eraseSentChunkFromFullResponse(myRet);
			std::cout << CYN << "    Sent chunk " << myRet << ",  sentSoFar " << storage->getResponseData().getSentSoFar() << "\n" << RES;
		}
		if (myRet < 0) {  // system failure // if some of the message has been sent then send the error block - fixed position
            storage->getResponseData().overrideFullResponse();  // content length -> which has already been sent/
		}
		else {	
			if (storage->getResponseData().getOverride() == true) {	// maybe we will not have this feature
				std::cout << RED << "System error! (send) " RES "\n";
				removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, override on _fdClient");
				removeFilter(event, EVFILT_TIMER, "failed kevent, EV_DELETE, EVFILT_TIMER, override on _fdClient");
				closeClient(event);
			}
			else {
				if (myRet == 0 && storage->getResponseData().getCurrentLength() == 0) {
					std::cout << CYN << "ALL SENT, CLOSE CLIENT" RES "\n";
					removeFilter(event, EVFILT_WRITE, "failed kevent, EV_DELETE, EVFILT_WRITE, success on _fdClient");
					removeFilter(event, EVFILT_TIMER, "failed kevent, EV_DELETE, EVFILT_TIMER, success on _fdClient");
					closeClient(event);
				}				
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

    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ New client connection ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
	
	int opt_value = 1;
	int fd = accept(event.ident, (struct sockaddr *)&socket_addr, &socklen);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
	if (fd == -1)
		throw ServerException("failed accept");

	Request *storage = new Request(fd, specificServer);
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent EV_ADD, EVFILT_READ, new client");

	int time = 5 * 1000;     // needs to be 30 for final version -----------------------------------------
	EV_SET(&evSet, fd, EVFILT_TIMER, EV_ADD, 0, time, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent EV_ADD, EVFILT_TIMER, new client");
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
	{
		std::cout << "erno: " << errno << std::endl;                         // test line - to be removed 
		throw ServerException(errorMessage);
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

// void WebServer::sendResponseFile(struct kevent& event, std::string file)
// {
// 	Request *storage = (Request *)event.udata;
// 	std::string response;
// 	std::string headerBlock;
// 	response = streamFile(file);
	
// 	int temp = response.length();
// 	std::string fileLen = std::to_string(temp);
// 	std::string contentLen = "Content-Length: ";// TODO Content-Length is present in ALL responses even DELETE?
// 	contentLen.append(fileLen);
// 	contentLen.append("\r\n");
// 	// std::cout << RED "ContLen: " << contentLen << "\n" << RES;

// 	headerBlock = 	"HTTP/1.1 200 OK\n"
// 					"Content-Type: text/html\n";
// 					// "Content-Type: image/png\n";
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


// NEW SEND_IMAGE
// void WebServer::sendImmage(struct kevent& event, std::string imagePath) {
// 	std::cout << GRN << "FOUND IMAGE extention .jpg or .png\n" << RES;// TODO it works on Chrome but breaks on Safari for me (joyce)
// 	unsigned long ret = 0;

// 	// Stream image and store it into a string
// 	std::fstream imageFile;
// 	std::string content;
// 	imageFile.open(imagePath);
// 	content.assign(std::istreambuf_iterator<char>(imageFile), std::istreambuf_iterator<char>());
// 	content += "\r\n";
// 	imageFile.close();

// 	// Send header block
// 	std::string headerBlock = 	"HTTP/1.1 200 OK\r\n"
// 								"Content-Type: image/jpg\r\n";
// 	headerBlock.append("accept-ranges: bytes\r\n");
// 	std::string contentLen = "Content-Length: ";
// 	std::string temp = std::to_string(content.size());
// 	headerBlock.append(contentLen);
// 	contentLen.append(temp);
// 	headerBlock.append("\r\n\r\n");
// 	ret = send(event.ident, headerBlock.c_str(), headerBlock.length(), 0);
// 	//std::cout << YEL << "Image header block sent, ret: " << ret << RES << "\n";

// 	// Send image content and each time reduce the original by ret
// 	size_t sentSoFar = 0;
// 	size_t imageSize = content.size();
// 	for (int i = 0; sentSoFar < imageSize; i++) {
// 		ret = send(event.ident, content.c_str(), content.size(), 0);
// 		if (ret == std::string::npos) {
// 			std::cout << RED << i << "    Nothing sent (" << ret << RES << "),  sentSoFar " << sentSoFar << "\n";
// 			continue ;
// 		}
// 		else {
// 			content.erase(0, ret);
// 			sentSoFar += ret;
// 			std::cout << YEL << i << "    Sent chunk " << ret << RES << ",  sentSoFar " << sentSoFar << "\n";
// 		}
// 	}
// }

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
