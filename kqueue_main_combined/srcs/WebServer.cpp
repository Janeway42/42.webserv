#include "WebServer.hpp"

WebServer::WebServer(std::string const & configFileName)
{
    ConfigFileParser configFileData = ConfigFileParser(configFileName);
    _servers = configFileData.servers;
    std::cout  << "Server blocks quantity: " << configFileData.numberOfServerBlocks() << std::endl;

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
			throw ServerException("failed number events");
		else
		{
			for (i = 0; i < nr_events; i++)
			{
			//	std::cout << "filter: " << evList[i].filter << std::endl; // test line 

				if (evList[i].flags & EV_ERROR)
					std::cout << "Event error\n";
				else if (evList[i].ident == _servers.begin()->getListeningSocket())//todo loop to know if evList[i].ident == current client listening socket
					newClient(evList[i]);
				else if (evList[i].filter == EVFILT_TIMER)
					handleTimeout(evList[i]);
				else if (evList[i].filter == EVFILT_READ)
				{
				//	std::cout << "READ\n";
					if (evList[i].flags & EV_EOF)  // if client closes connection 
					{
						removeEvent(evList[i], EVFILT_READ, "failed kevent eof - read");
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
						removeEvent(evList[i], EVFILT_WRITE, "failed kevent eof - write");
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

	storage->setError(3);      // -------------------    408 Request Timeout
	std::cout << "Unable to process, takes too long\n";

	struct kevent evSet;
	EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_WRITE - handle timeout");
}

void WebServer::readRequest(struct kevent& event)
{
	char buffer[BUFFER_SIZE];
	memset(&buffer, '\0', BUFFER_SIZE);
	Request* storage = (Request*)event.udata;

	if (event.ident == (storage->getCgiData()).getPipeCgiOut())  // read from CGI - we get the info 
	{
		size_t ret = read(event.ident, &buffer,  BUFFER_SIZE);
		if (ret < 0)
		{
			std::cout << "failed recv in pipe from cgi\n";
			storage->setError(4);       // -------------------    500 Internal Server Error  

			// processResponse();
			struct kevent evSet;
			EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
			if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
				throw ServerException("failed kevent add EVFILT_WRITE");
			removeEvent(event, EVFILT_READ, "failed kevent eof - read failure");
			std::cout << "send error message back to client\n";
		}
		else
		{
			if (ret != 0)
				// append to responseBody in ResponseData - (storage->Responsedata().getResponseBody()).append(buffer)
			if (ret == 0 || buffer[ret] == EOF)
			{
				//processResponse();
				struct kevent evSet;
				EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
				if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
					throw ServerException("failed kevent add EVFILT_WRITE");
				removeEvent(event, EVFILT_READ, "failed kevent eof - read failure");
			}
		}
	}

	else if (event.ident == storage->getFdClient())
	{
		int ret = recv(event.ident, &buffer, BUFFER_SIZE - 1, 0);
		//	std::cout << "bytes read: " << ret << std::endl;                 // test line 
		if (ret <= 0) // kq itself will NEVER send a READ event if there is nothing to receive 
		{
			std::cout << "failed recv\n";
			storage->setError(1);       // -------------------    400 Bad request
			storage->setDone(true);

			// processResponse();
			struct kevent evSet;
			EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
			if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
				throw ServerException("failed kevent add EVFILT_WRITE");
			removeEvent(event, EVFILT_READ, "failed kevent eof - read failure");
			std::cout << "send error message back to client\n";
		}
	
		else if (storage->getDone() == false)
		{
		//	std::cout << "append buffer\n";
			storage->appendToRequest(buffer, event.ident);

			if (storage->getError() != 0 || storage->getDone() == true)
			{
				if (storage->getError() != 0)
					std::cout << "error parsing - sending response - failure\n";
				else if (storage->getDone() == true)
					std::cout << "done parsing - sending response - success\n";

				// processResponse();
				struct kevent evSet;
				EV_SET(&evSet, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
				if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
					throw ServerException("failed kevent add EVFILT_WRITE");
				removeEvent(event, EVFILT_READ, "failed kevent eof - read failure");
			}
		}
	}
}

void WebServer::sendResponse(struct kevent& event) {
//    if (not event.udata) {
//        // JOYCE I got a heap-use-after-free from AddressSanitizer
//        // READ of size 8 at 0x615000001140 thread T0
//        //    #0 0x10c0a1980 in Request::getTime() RequestParser.cpp:349
//        //    ...
//        // i think it's in case  storage->getTime() tries runs but event.udata does not exist
//        // with this if I did not see the error again but we can keep an eye on it
//        throw ServerException("Empty event.udata");
//    }
    Request *storage = (Request*)event.udata;
	std::string buffer;

	if (event.ident == (storage->getCgiData()).getPipeCgiIn())  // write to CGI - we send the info
	{
		buffer = storage->getResponseData().getResponseBody();
		size_t ret = write(storage->getCgiData().getPipeCgiIn(), buffer.c_str(), buffer.length());
		if (ret == -1)
		{
			storage->setError(4); // 500 system failure 
			// processResponse();
			// allow WRITE
		}
		else 
		{
			storage->getCgiData().setBytesToCgi(ret);
			// keep track of what has been sent
			if (storage->getCgiData().getBytesToCgi() == storage->getRequestData().getRequestContentLength())
			{
				// remove write on this fd 
				// close fd   - is this actually necessary???
			}
		}
	}
	else if (event.ident == storage->getFdClient())
	{
		buffer = storage->getResponseData().getFullResponse();
		int ret = send(event.ident, buffer.c_str(), buffer.length(), 0);
		if (ret == -1)  // system failure 
		{
			// if some of the message has been sent then send the error block
			// remove WRITE, READ , POSIIBLE PIPES
			// closeclient()
		}
		else
		{
			
			if (ret != buffer.length());
				// erase first part of storage->getResponsedata().getFullResponse();
			else
			{
				if (storage->getResponseData().getBytesToClient())
			}
		}
	}



	if ((storage->getCgiData()).getPipesDone() == true)
		(storage->getCgiData()).closeFileDescriptors(event);  // close the pipes if the parent times out. 

	// process response
	// send file 
}

// --------------------------------------------------------- client functions 
// --------------------------------------------------------------------------

void WebServer::newClient(struct kevent event)
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

	Request *storage = new Request(fd);
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_READ");
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

int WebServer::removeEvent(struct kevent& event, int filter, std::string errorMessage)
{
	struct kevent evSet;
	Request *storage = (Request *)event.udata;

	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		std::cout << "erno: " << errno << std::endl;                         // test line - to be removed 
		throw ServerException(errorMessage);
	}
	return (0);
}

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

void WebServer::sendResponseFile(struct kevent& event, std::string file)
{
	Request *storage = (Request *)event.udata;
	std::string response;
	std::string headerBlock;
	response = streamFile(file);
	
	int temp = response.length();
	std::string fileLen = std::to_string(temp);
	std::string contentLen = "Content-Length: ";// TODO Content-Length is present in ALL responses even DELETE?
	contentLen.append(fileLen);
	contentLen.append("\r\n");
	// std::cout << RED "ContLen: " << contentLen << "\n" << RES;

	headerBlock = 	"HTTP/1.1 200 OK\n"
					"Content-Type: text/html\n";
					// "Content-Type: image/png\n";
	if (storage->getError() == true)
		headerBlock = 	"HTTP/1.1 404 Not Found\n"
						"Content-Type: text/html\n";
	headerBlock.append(contentLen);
	headerBlock.append("\r\n\r\n");
	headerBlock.append(response);

	int ret = send(event.ident, headerBlock.c_str(), headerBlock.length(), 0);
	if (ret == -1)
		throw ServerException("Send failed\n");
	std::cout << "ret: " << ret << std::endl;

}


// NEW SEND_IMAGE
void WebServer::sendImmage(struct kevent& event, std::string imagePath) {
	std::cout << GRN << "FOUND IMAGE extention .jpg or .png\n" << RES;// TODO it works on Chrome but breaks on Safari for me (joyce)
	unsigned long ret = 0;

	// Stream image and store it into a string
	std::fstream imageFile;
	std::string content;
	imageFile.open(imagePath);
	content.assign(std::istreambuf_iterator<char>(imageFile), std::istreambuf_iterator<char>());
	content += "\r\n";
	imageFile.close();

	// Send header block
	std::string headerBlock = 	"HTTP/1.1 200 OK\r\n"
								"Content-Type: image/jpg\r\n";
	headerBlock.append("accept-ranges: bytes\r\n");
	std::string contentLen = "Content-Length: ";
	std::string temp = std::to_string(content.size());
	headerBlock.append(contentLen);
	contentLen.append(temp);
	headerBlock.append("\r\n\r\n");
	ret = send(event.ident, headerBlock.c_str(), headerBlock.length(), 0);
	//std::cout << YEL << "Image header block sent, ret: " << ret << RES << "\n";

	// Send image content and each time reduce the original by ret
	size_t sentSoFar = 0;
	size_t imageSize = content.size();
	for (int i = 0; sentSoFar < imageSize; i++) {
		ret = send(event.ident, content.c_str(), content.size(), 0);
		if (ret == std::string::npos) {
			//std::cout << RED << i << "    Nothing sent (" << ret << RES << "),  sentSoFar " << sentSoFar << "\n";
			continue ;
		}
		else {
			content.erase(0, ret);
			sentSoFar += ret;
			//std::cout << YEL << i << "    Sent chunk " << ret << RES << ",  sentSoFar " << sentSoFar << "\n";
		}
	}
}

/* OLD SEND_IMAGE
void WebServer::sendImmage(struct kevent& event, std::string imagePath)
{
	std::cout << RED << "FOUND extention .jpg or .png\n" << RES;

	FILE *file;
	unsigned char *buffer;
	unsigned long imageSize;

	file = fopen(imagePath.c_str(), "rb");
	if (!file)
	{
		std::cerr << "Unable to open file\n";
		return ;
 	}

	fseek(file, 0L, SEEK_END);	// Get file length
	imageSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	std::string temp = std::to_string(imageSize);
	std::string contentLen = "Content-Length: ";
	contentLen.append(temp);
	contentLen.append("\r\n");

	std::string headerBlock = 	"HTTP/1.1 200 OK\r\n"
								"accept-ranges: bytes\r\n"
								"Content-Type: image/jpg\r\n";
	headerBlock.append(contentLen);
	headerBlock.append("accept-ranges: bytes");
	headerBlock.append("\r\n\r\n");

	buffer = (unsigned char *)malloc(imageSize);
	if (!buffer)
		{ fprintf(stderr, "Memory error!"); fclose(file); return ; }

	int ret = fread(buffer, sizeof(unsigned char), imageSize, file);
	std::cout << YEL << "Returned fread:     " << ret << RES << "\n";
	
	ret = send(event.ident, headerBlock.c_str(), headerBlock.length(), 0);
	ret = send(event.ident, reinterpret_cast <const char* >(buffer), imageSize, 0);
	std::cout << YEL "Image sent, returned from send() image: " << ret << RES "\n";
	fclose(file);
	free(buffer);
}
*/


// --------------------------------------------------------- get functions
// -----------------------------------------------------------------------

//int WebServer::getSocket(void)
//{
//	return (_listening_socket);
//}

int WebServer::getKq(void)
{
	return (_kq);
}
