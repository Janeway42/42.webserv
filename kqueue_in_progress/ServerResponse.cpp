#include "includes/ServerResponse.hpp"

namespace data {

Response::Response(void) {}

Response::~Response(void) {}

// ---------------------------------------------------------------------------- set functions
// ------------------------------------------------------------------------------------------

void Response::setResponse(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;	

	_responseHeader += setResponseStatus(event);

	if (storage->getRequestData().getRequestContentType().compare("txt") == 0)
		_responseBody = streamFile(storage->getAnswer().getResponsePath());
	else
		_responseBody = setImage(storage->getAnswer().getResponsePath());

	int temp = _responseBody.length();
	std::string fileLen = std::to_string(temp);
	std::string contentLen = "Content-Length: ";
	contentLen.append(fileLen);
	contentLen.append("\r\n");

	_responseHeader += contentLen;
	_responseHeader += "\r\n\r\n";

	_fullResponse += _responseHeader + _responseBody;
}

std::string Response::setResponseStatus(struct kevent& event)
{
	data::Request *storage = (data::Request *)event.udata;
	std::string status;

	std::string fileType = storage->getRequestData().getRequestContentType();

	if (fileType.compare("txt") != 0 && storage->getError() == 0)
	{
		status = "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/";   // change to take in consideration the image extension
		status += fileType + "\r\n";
		(storage->getAnswer()).setResponsePath(storage->getRequestData().getHttpPath());
		return (status);
	}				
	
	switch (storage->getError())
	{
		case 1:
			status = "HTTP/1.1 400 Bad Request\n"
						"Content-Type: text/html\n";
			(storage->getAnswer()).setResponsePath("error404.html");
			break;
		case 2:
			status = "HTTP/1.1 405 Method Not Allowed\n"
						"Content-Type: text/html\n";
			(storage->getAnswer()).setResponsePath("error404.html");
			break;
		case 3:
			status = "HTTP/1.1 408 Request Timeout\n"
						"Content-Type: text/html\n";
			(storage->getAnswer()).setResponsePath("error404.html");
			break;
		default:
			status = "HTTP/1.1 200 OK\n"
						"Content-Type: text/html\n";
			(storage->getAnswer()).setResponsePath(storage->getRequestData().getHttpPath());
			break;
	}
	return (status);
}

std::string Response::setImage(std::string imagePath)
{
	std::cout << RED "FOUND extention .jpg or .png\n" RES;

	FILE *file;
	unsigned char *buffer;
	std::string imageFile;
	unsigned long imageSize;

	file = fopen(imagePath.c_str(), "rb");
	if (!file)
	{
		std::cerr << "Unable to open file\n";
		// return ;  throw error 
 	}

	fseek(file, 0L, SEEK_END);	// Get file length
	imageSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	std::string temp = std::to_string(imageSize);
	std::string contentLen = "Content-Length: ";
	contentLen.append(temp);
	contentLen.append("\r\n");

	imageFile += contentLen;
	imageFile += "accept-ranges: bytes";
	imageFile += "\r\n\r\n";

	buffer = (unsigned char *)malloc(imageSize);
	if (!buffer)
		{ 
			fprintf(stderr, "Memory error!"); fclose(file); 
			// return ;   throw error 
			}

	int ret = fread(buffer, sizeof(unsigned char), imageSize, file);
	std::cout << YEL "Returned fread:     " << ret << RES "\n";

	imageFile += reinterpret_cast<const char* >(buffer);
	
	fclose(file);
	free(buffer);
	return (imageFile);
}

void Response::setResponsePath(std::string file)
{
	_responsePath = file;
}

// --------------------------------------------------------------------------- util functions
// ------------------------------------------------------------------------------------------

std::string Response::streamFile(std::string file)
{
	std::string responseNoFav;
	std::fstream    infile;


	infile.open(file, std::fstream::in);
	// if (!infile)
		// throw ServerException("Error: File not be opened for reading!");   SET UP ERROR
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

// ---------------------------------------------------------------------------- get functions
// ------------------------------------------------------------------------------------------

std::string Response::getHeader()
{
	return (_responseHeader);
}

std::string Response::getBody()
{
	return (_responseBody);
}

std::string Response::getFullResponse()
{
	return (_fullResponse);
}

std::string Response::getResponsePath()
{
	return (_responsePath);
}

} //data



// ------------------------------------------------------------------------------ HTTP STATUS
// ------------------------------------------------------------------------------------------


// 200 OK  - default
//     Standard response for successful HTTP requests. The actual response will depend on the request method used. 
// 	In a GET request, the response will contain an entity corresponding to the requested resource. 
// 	In a POST request, the response will contain an entity describing or containing the result of the action.

// 400 Bad Request  - error 1
//     The server cannot or will not process the request due to an apparent client error 
// 	(e.g., malformed request syntax, size too large, invalid request message framing, or deceptive request routing).

// 405 Method Not Allowed - error 2
//     A request method is not supported for the requested resource; for example, a GET request on a form that requires data to be presented via POST, 
// 	or a PUT request on a read-only resource.

// 408 Request Timeout - error 3
//     The server timed out waiting for the request. According to HTTP specifications: "The client did not produce a request within the time that 
// 	the server was prepared to wait. The client MAY repeat the request without modifications at any later time."

// ---------------




// 404 Not Found
//     The requested resource could not be found but may be available in the future. Subsequent requests by the client are permissible.

// 411 Length Required
//     The request did not specify the length of its content, which is required by the requested resource.

// 413 Payload Too Large
//     The request is larger than the server is willing or able to process. Previously called "Request Entity Too Large" in RFC 2616.

// 414 URI Too Long
//     The URI provided was too long for the server to process. Often the result of too much data being encoded as a query-string of a GET request, 
// 	in which case it should be converted to a POST request. Called "Request-URI Too Long" previously in RFC 2616.