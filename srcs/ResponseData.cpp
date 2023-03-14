#include "../includes/RequestParser.hpp"
#include "../includes/ResponseData.hpp"
#include "../includes/RequestData.hpp"


#include <string.h> // jaka, temp, can be removed

ResponseData::ResponseData(void) {
	_status = "";
	//_length = "";
	_type = "";
	_responseHeader = "";
	_responseBody = "";
	_fullResponse = "";
	_responsePath = "";
	_lengthFullResponse = 0;
	_bytesToClient = 0;
	_errorOverride = false;
}

ResponseData::~ResponseData(void) {}

// ---------------------------------------------------------------------------- set functions
// ------------------------------------------------------------------------------------------



/*

	Problem in setImage:
	It correctly reads into buffer the size of image 35581, but then size of buffer is only 4, and size of image is only 67
	Maybe look into the sendImage that was working int temp02 or temp03
	???

	Here in setResponse() is maybe also a problem
		The image is set in setImage, and it gets everything there, status line, length, type, etc ...
		This should then be sent in sendResponse()
*/


void ResponseData::setResponse(struct kevent& event)
{
	Request *storage = (Request *)event.udata;	

	_responseHeader += setResponseStatus(event);

	if (storage->getRequestData().getRequestContentType().compare("text/html") == 0)
		_responseBody = streamFile(_responsePath);
	else {
		std::cout << YEL "SetResponse():  It is an image, by here it already has a complete header with image body\n";
		_fullResponse = setImage(storage->getResponseData().getResponsePath());
		std::cout << BLU "image _fullResponse: [\n" <<_fullResponse << "\n" RES;
		return ;
	}

	// set up header 
	int temp = _responseBody.length();
	std::string fileLen = std::to_string(temp);
	std::string contentLen = "Content-Length: ";
	contentLen.append(fileLen);
	contentLen.append("\r\n");

	_responseHeader += contentLen;
	_responseHeader += "\r\n\r\n";

	std::cout << YEL "complete response header: [" << _responseHeader << "]\n";

	_fullResponse += _responseHeader + _responseBody;
}


std::string ResponseData::setResponseStatus(struct kevent& event)
{
	Request *storage = (Request *)event.udata;
	std::string status;

	std::string fileType = storage->getRequestData().getRequestContentType();
	//std::cout << GRN "content type: [" << storage->getRequestData().getRequestContentType() << "]\n" RES;

	// if (fileType.compare("text/html") != 0 && storage->getError() == 0)
	// {
	// 	std::cout << RED "A) setResponseStatus\n";
	// 	status = "HTTP/1.1 200 OK\r\n"
	// 				// "Content-Type: image/png";   // change to take in consideration the image extension
	// 				"Content-Type: ";   // change to take in consideration the image extension
	// 	status += fileType + "\r\n";
	// 	// (storage->getResponseData()).setResponsePath(storage->getRequestData().getHttpPath()); jaka: should be getPath()
	// 	(storage->getResponseData()).setResponsePath(storage->getRequestData().getPath());
	// 	std::cout << GRN "getHttpPath: [" << _responsePath << "]\n" RES;
	// 	std::cout << GRN "status: [" << status << "]\n" RES;
	// 	return (status);
	// }			
	
	std::cout << RED "B) setResponseStatus\n";
	switch (storage->getError())
	{
		case 1:
			status = "HTTP/1.1 400 Bad Request\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/400BadRequest.html");
			break;
		case 2:
			status = "HTTP/1.1 404 Not Found\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/404NotFound.html");
			break;
		case 3:
			status = "HTTP/1.1 405 Method Not Allowed\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/405MethodnotAllowed.html");
			break;
		case 4:
			status = "HTTP/1.1 408 Request Timeout\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/408RequestTimeout.html");
			break;
		case 5:
			status = "HTTP/1.1 500 Internal Server Error";
			(storage->getResponseData()).setResponsePath("resources/error_pages/500InternarServerError.html");
		default:
			status = "HTTP/1.1 200 OK\n"  
					"Content-Type: " + storage->getRequestData().getRequestContentType() + "\n";	// jaka
			//_responsePath = "resources/index_dummy.html";
			//_responsePath = "resources/index_just_text.html";
			//  _responsePath = "resources/index_just_text_bible.html";
			//  _responsePath = "resources/index_just_image.html";
			//  _responsePath = "resources/img_36kb.jpg";
			//  _responsePath = storage->getRequestData().getHttpPath();	// jaka: this is old, should be getPath()
			_responsePath = storage->getRequestData().getPath();
			std::cout << GRN "getHttpPath: [" << _responsePath << "]\n" RES;
			break;
	}
	return (status);
}

void ResponseData::setResponseBody(std::string file)  
{
	_responseBody += file;
}

void ResponseData::setBytesToClient(int val)
{
	_bytesToClient += val;
}



std::string ResponseData::setImage(std::string imagePath)
{
	std::cout << RED "FOUND extention .jpg or .png\n" RES;

	FILE *file;
	unsigned char *buffer;
	std::string imageFile;
	// unsigned long imageSize;
	size_t imageSize;

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

	imageFile += "HTTP/1.1 200 OK\n";	// maybe here not good
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
	std::cout << YEL "Returned fread, image size:     " << ret << RES "\n";

	std::cout << GRN "buffer size: " << strlen(reinterpret_cast<const char* >(buffer)) << "\n" RES;
	imageFile += reinterpret_cast<const char* >(buffer);
	
	std::cout << GRN "imageFile size:" << imageFile.size() << "\n" RES;
	std::cout << GRN "setImage(), imageFile [\n" << imageFile << "\n" RES;


	fclose(file);
	free(buffer);
	return (imageFile);
}

void ResponseData::setOverride(bool val)
{
	_errorOverride = val;
}

void ResponseData::setResponsePath(std::string path)
{
	_responsePath = path;
}


// --------------------------------------------------------------------------- util functions
// ------------------------------------------------------------------------------------------

void ResponseData::overrideFullResponse()
{
	std::string errorMessage = " insert html code";
	_fullResponse = errorMessage;
	_errorOverride = true;
}

std::string ResponseData::streamFile(std::string file)
{
	std::string responseNoFav;
	std::fstream    infile;

	// std::cout << "file: " << file << std::endl; 


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

	// std::cout << "streamed: " << responseNoFav << std::endl;
	return (responseNoFav);
}


// ***************************************************************************
// added JAKA, to erase the sent chunk from the remaining response content
std::string&	ResponseData::eraseSentChunkFromFullResponse(unsigned long retBytes) {
	return (_fullResponse.erase(0, retBytes));
}

size_t	ResponseData::getCurrentLength() { // jaka
	return (_lengthFullResponse);
}

size_t	ResponseData::getSentSoFar() { // jaka
	return (_bytesToClient);
}

void 	ResponseData::setCurrentLength(size_t len) {
	_lengthFullResponse = len;
}

void	ResponseData::increaseSentSoFar(size_t bytesSent) {
	_bytesToClient += bytesSent;
}

// ***************************************************************************



// ---------------------------------------------------------------------------- get functions
// ------------------------------------------------------------------------------------------

std::string ResponseData::getHeader()
{
	return (_responseHeader);
}

std::string ResponseData::getBody()
{
	return (_responseBody);
}

std::string& ResponseData::getFullResponse()
{
	return (_fullResponse);
}

std::string ResponseData::getResponsePath()
{
	return (_responsePath);
}

std::string ResponseData::getResponseBody()
{
	return (_responseBody);
}

unsigned long ResponseData::getBytesToClient()
{
	return (_bytesToClient);
}
bool ResponseData::getOverride()
{
	return (_errorOverride);
}

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
