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

// This function creates the header only for text/html requests, but not for images.
// If it is an image, then setImage() is called, where both header and body are created, and
// then setResponse() returns this full content, ready to be sent.
void ResponseData::setResponse(struct kevent& event) {
	
	Request *storage = (Request *)event.udata;	
	_responseHeader += setResponseStatus(event);

	std::string serverRootFolder = storage->getServerData().getRootDirectory();
	if (serverRootFolder.substr(0, 2) == "./") {
		//std::cout << YEL " ...... yes ./ found in root name\n" << RES;
		serverRootFolder.erase(0, 2);
		//std::cout << YEL " ...... erased: [" << serverRootFolder << "]\n" << RES;
	}


	// IF THE PATH IS A FOLDER, THIS FUNCTION NEEDS TO CHECK IF THERE IS A DEFAULT INDEX FILE PRESENT,
	// IF NOT, THEN CHECK IF AUTOINDEX IS ON,
	// 			IF YES, SEND HTML WITH FOLDER CONTENT
	//			IF NOT, SEND ERROR PAGE, NOT ALLOWED ?
	if (storage->getRequestData().getIsFolder() == true) {
		std::cout << YEL "The Path is a folder: check for a default index file and/or autoindex on/off\n" << RES;
		std::cout << YEL "           Stored server root folder: [" << storage->getServerData().getRootDirectory() << "]\n" RES;
	//	std::cout << YEL "          local var. for root folder: [" << serverRootFolder << "]\n" RES;
	//	std::cout << YEL "                             getPath: [" << storage->getRequestData().getPath() << "]\n" RES;

		//if (storage->getRequestData().getRequestContentType().compare("text/html") == 0) {		// IF FOLDER, THE CONT. TYPE SHOULD BE text.html
			
			// IF PATH IS THE SERVER ROOT "./"  (  ./resources/  )
			if (storage->getRequestData().getPath() == ("./" + serverRootFolder)) {		// The path matches the server root
				std::cout << YEL "                The Path is the root: [" << storage->getRequestData().getPath() << "]\n" RES;
				_responsePath = storage->getServerData().getIndexFile();	// It is already stored as the whole path: ./resources/index.html
				std::cout << YEL "                       _responsePath: [" << _responsePath << "]\n" RES;
			}
			// IF PATH IS A FOLDER INSIDE THE ROOT FOLDER
			else {
				std::cout << YEL "          The Path is a folder inside the root: [" << storage->getRequestData().getPath() << "]\n" RES;

				// Here it should compare the path with available locations.
				// If a location is valid in the config file, then take the default index file inside that location
				// and append the filename after the path.
				std::vector<ServerLocation> location_data_vector = storage->getServerData().getLocationBlocks();
				for (size_t i = 0; i < location_data_vector.size(); i++) {
                    std::cout << GRE "   ........ location uri: [" << location_data_vector[i].getLocationPath() << "]\n";
                    std::cout << GRE "   ... location root dir: [" << location_data_vector[i].getRootDirectory() << "]\n";
                    std::cout << GRE "   ....... _responsePath: [" << location_data_vector[i].getRootDirectory() << "]\n";
					if (location_data_vector[i].getRootDirectory() == _responsePath) {// TODO here it should be getLocationPath() ?? talk to joyce
						_responsePath = location_data_vector[i].getIndexFile();
                    	std::cout << BLU "   ....... FinalPath: [" << _responsePath << "]\n";

					}
				}
			}


			_responseBody = streamFile(_responsePath);
			std::cout << YEL "          response path for autoindex_dummy: [" << _responsePath << "]\n" RES;
			std::cout << YEL "          content type should now be text/html: [" << storage->getRequestData().getRequestContentType() << RES "]\n";
		//}
	}
	// IF NOT A FOLDER
	else {	// IF TEXTFILE
		if (storage->getRequestData().getRequestContentType().compare("text/html") == 0)
			_responseBody = streamFile(_responsePath);
		else {	// IF IMAGE, FULL RESPONSE IS CREATED IN setImage()
			_fullResponse = setImage(storage->getResponseData().getResponsePath());
			// std::cout << BLU "_fullResponse.length(): [\n" << _fullResponse.size() << "\n" RES;
			return ;
		}
	}


	// set up header 
	int temp = _responseBody.length();
	std::string fileLen = std::to_string(temp);
	std::string contentLen = "Content-Length: ";
	contentLen.append(fileLen);
	contentLen.append("\r\n");
	_responseHeader += contentLen;
	_responseHeader += "\r\n\r\n";

	// std::cout << YEL "complete response header: [" << _responseHeader << "]\n";
	_fullResponse += _responseHeader + _responseBody;
}


std::string ResponseData::setResponseStatus(struct kevent& event)
{	
	//std::cout << RED "start setResponseStatus\n";
	Request *storage = (Request *)event.udata;
	std::string status;

	std::string fileType = storage->getRequestData().getRequestContentType();

	switch (storage->getError())
	{
		case 1:
			status = "HTTP/1.1 400 Bad Request\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/400BadRequest.html");
			break;
		case 404:
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
		case 6:
			status = "HTTP/1.1 403 Forbidden";
			(storage->getResponseData()).setResponsePath("resources/error_pages/403Forbidden.html");
		default:
			status = "HTTP/1.1 200 OK\n"  
					"Content-Type: " + storage->getRequestData().getRequestContentType() + "\n";	// jaka
			//  _responsePath = storage->getRequestData().getHttpPath();							// jaka: this is old, should be getPath()
			_responsePath = storage->getRequestData().getPath();
			std::cout << GRN "_responsePath: [" << _responsePath << "]\n" RES;
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




// NEW SETIMAGE
// void ResponseData::setImage(struct kevent& event, std::string imagePath) {
std::string ResponseData::setImage(std::string imagePath) {
	//std::cout << RED "Start SetImage()\n" RES;

	std::fstream imageFile;		// Stream image and store it into a string
	std::string content;
	imageFile.open(imagePath);

	content.assign(std::istreambuf_iterator<char>(imageFile), std::istreambuf_iterator<char>());
	content += "\r\n";
	imageFile.close();

	// std::string contentType = getContent...  				// Here it needs to grab the correct Type, jpg, png, gif, ico ...

	// Create the header block
	std::string headerBlock = 	"HTTP/1.1 200 OK\r\n"
								"Content-Type: image/jpg\r\n";	// Here it needs to grab the correct Type, jpg, png, gif, ico ...
	headerBlock.append("accept-ranges: bytes\r\n");
	std::string contentLen = "Content-Length: ";
	std::string temp = std::to_string(content.size());
	headerBlock.append(contentLen);
	contentLen.append(temp);
	headerBlock.append("\r\n\r\n");

	headerBlock.append(content);
	return (headerBlock);	// Both header and image content
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

	std::cout << "File to be streamed: " << file << std::endl;
	infile.open(file, std::fstream::in);
	if (not infile)
        throw ParserException(CONFIG_FILE_ERROR("File to be streamed is ", MISSING));
//		throw ServerException("Error: File not be opened for reading!");   SET UP ERROR
	while (infile)     // While there's still stuff left to read
	{
		std::string strInput;
		std::getline(infile, strInput);
		responseNoFav.append(strInput);
		responseNoFav.append("\n");
	}
	infile.close();

	std::cout << "Streamed: " << responseNoFav << std::endl;
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
