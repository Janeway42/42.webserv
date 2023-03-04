#include "WebServer.hpp"  // jaka

WebServer::WebServer(std::string const & configFileName)
{
	_addr = new struct addrinfo();
	struct addrinfo hints;

	hints.ai_family = PF_UNSPEC; 
	hints.ai_flags = AI_PASSIVE; 
	hints.ai_socktype = SOCK_STREAM;

    (void)configFileName;
//    ConfigFileParser configFileData = ConfigFileParser(configFileName);
//    _servers = configFileData.servers;
    //std::vector<ServerData>::iterator it_server = _servers.begin();
    //for (; it_server != _servers.cend(); ++it_server) {
//        std::cout  << "IP ADDRESS: " << it_server->getIpAddress() << std::endl;
//        std::cout  << "PORT: " << it_server->getListensTo() << std::endl;
    if (getaddrinfo("127.0.0.1", "8008", &hints, &_addr) != 0) {
        throw ServerException(("failed addr"));
    }
    _listening_socket = socket(_addr->ai_family, _addr->ai_socktype, _addr->ai_protocol);
        //it_server->setListeningSocket(_listening_socket);// todo I get a "failed addr" when I try this (or anything else, even a log line)
        //break;// todo for now, I am breaking, but we intend to keep looping to the other server blocks!
    //}

	if (_listening_socket < 0)
		throw ServerException(("failed socket"));
	fcntl(_listening_socket, F_SETFL, O_NONBLOCK);

	int socket_on = 1;
	setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
	if (bind(_listening_socket, _addr->ai_addr, _addr->ai_addrlen) == -1)
	{
		perror("... bind error: ");
		throw ServerException(("failed bind"));
	}
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

WebServer::~WebServer()
{
	close(_kq);
	freeaddrinfo(_addr);
}

// --------------------------------------------------------- server main loop
// --------------------------------------------------------------------------

void WebServer::runServer()
{
	struct kevent evList[MAX_EVENTS];
//	struct kevent evSet;
	int i;
//	struct sockaddr_storage socket_addr;
//	socklen_t socklen = sizeof(socket_addr);

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
				else if (evList[i].ident == _listening_socket)
					newClient(evList[i]);
				else if (evList[i].filter == EVFILT_READ)
				{
				//	std::cout << "READ\n";
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
				//	std::cout << "WRITE\n";
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
		// std::cout << std::endl;
//		loop1++;
	}
}

// --------------------------------------------------------- server functions 
// --------------------------------------------------------------------------

void WebServer::readRequest(struct kevent& event)
{
	char buffer[50];
	memset(&buffer, '\0', 50);
	Request* storage = (Request*)event.udata;
	//Request* storage = (Request*)event.udata;

	int ret = recv(event.ident, &buffer, sizeof(buffer) - 1, 0);
//	std::cout << "bytes read: " << ret << std::endl;                 // test line 
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
	//	std::cout << "append buffer\n";
		storage->appendToRequest(buffer, event.ident);

		if (storage->getError() == true)
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

void WebServer::sendResponse(struct kevent& event) {
    if (not event.udata) {
        // JOYCE I got a heap-use-after-free from AddressSanitizer
        // READ of size 8 at 0x615000001140 thread T0
        //    #0 0x10c0a1980 in Request::getTime() RequestParser.cpp:349
        //    ...
        // i think it's in case  storage->getTime() tries runs but event.udata does not exist
        // with this if I did not see the error again but we can keep an eye on it
        throw ServerException("Empty event.udata");
    }
    Request *storage = (Request *) event.udata;

    std::time_t elapsedTime = std::time(NULL);
    if (elapsedTime - storage->getTime() > 15)// JOYCE QUESTION -> TODO this one is supposed to wait for 5 seconds to get a connection, if it does not ir closes the connection and closes write with error? (see logs)
    {
        std::cout << "Unable to process request, it takes too long!\n";
        storage->setError(true);
        storage->setEarlyClose(true);
        if (removeEvent(event, EVFILT_READ) == 1)
            throw ServerException("failed EVFILT_READ removal\n");
    }

    std::vector<ServerData>::iterator it_server = _servers.begin();
    for (; it_server != _servers.cend(); ++it_server) {
        if (storage->getError()) {
            sendResponseFile(event, "./resources/error404.html");
            if (removeEvent(event, EVFILT_WRITE) == 1)
                throw ServerException("failed kevent EV_DELETE client - send error");
            closeClient(event);
            std::cout << "closed connection from write - error\n";
        } else if (storage->getError() == false && storage->getDone() == true) {
            std::string temp = (storage->getRequestData()).getHttpPath();

            if (temp.find(".png") != std::string::npos) {
                // sendImmage(event, "./resources/immage.png");
                // sendImmage(event, "./resources/img_36kb.jpg");
                // sendImmage(event, "./resources/img_109kb.jpg");
                // sendImmage(event, "./resources/img_938kb.jpg");
                // sendImmage(event, "./resources/img_5000kb.jpg");
//                std::cout << "ROOT DIRECTORY: " << it_server->getRootDirectory() << std::endl;
//                sendResponseFile(event, it_server->getIndexFile());
                sendImmage(event, "resources/img_13000kb.jpg");

                //  !!!
                //  If the image path in html file is too long, it started, then address sanitizer
                //  gives error 'heap buffer overflow' !!!
            } else {
                std::cout << "INDEX FILE: " << it_server->getIndexFile() << std::endl;
                sendResponseFile(event, it_server->getIndexFile());
            }
            if (removeEvent(event, EVFILT_WRITE) == 1)
                throw ServerException("failed kevent EV_DELETE client - send success");
            closeClient(event);
            std::cout << "closed connection from write - done" << RES << std::endl;
            std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
        }
        break;// todo for now, I am breaking, but we intend to keep looping to the other server blocks data!
    }
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

	Request *storage = new Request();
	EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_READ");
	EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw ServerException("failed kevent add EVFILT_WRITE");
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
		std::cout << "erno: " << errno << std::endl;                         // test line - to be removed 
		throw ServerException("failed kevent EV_DELETE client");
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
	std::string contentLen = "Content-Length: ";
	contentLen.append(fileLen);
	contentLen.append("\r\n");
	// std::cout << RED "ContLen: " << contentLen << "\n" RES;

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
	std::cout << GRN "FOUND IMAGE extention .jpg or .png\n" RES;
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
	//std::cout << YEL "Image header block sent, ret: " << ret << RES "\n";

	// Send image content and each time reduce the original by ret
	size_t sentSoFar = 0;
	size_t imageSize = content.size();
	for (int i = 0; sentSoFar < imageSize; i++) {
		ret = send(event.ident, content.c_str(), content.size(), 0);
		if (ret == std::string::npos) {
			//std::cout << RED << i << "    Nothing sent (" << ret << RES "),  sentSoFar " << sentSoFar << "\n";
			continue ;
		}
		else {
			content.erase(0, ret);
			sentSoFar += ret;
			//std::cout << YEL << i << "    Sent chunk " << ret << RES ",  sentSoFar " << sentSoFar << "\n";
		}
	}
}

/* OLD SEND_IMAGE
void WebServer::sendImmage(struct kevent& event, std::string imagePath)
{
	std::cout << RED "FOUND extention .jpg or .png\n" RES;

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
	std::cout << YEL "Returned fread:     " << ret << RES "\n";
	
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
