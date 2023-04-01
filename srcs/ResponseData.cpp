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
	_isCgi = false;
}

ResponseData::~ResponseData(void) {}




// ---------------------------------------------------------------------------- set functions
// ------------------------------------------------------------------------------------------


// THIS SAME FUNCTION IS ALREADY INSIDE RequestParserURLpath, it can be used as one
static int checkIfPathExists(const std::string& path, struct kevent event) {
	
	(void)event;
	std::cout << GRN << "Start CheckIfFIleExists(), path [" << path << "] \n" << RES;

	
	std::ifstream file(path.c_str());

	if (not file.is_open()) {		// ??? what is this syntax? -> joyce for cpp we can use not in the pace of ! for readability :)
		std::cout << RED << "Error: File " << path << " not found\n" << RES;
		return(NOT_FOUND);
	}
	std::cout << GRN << "File/folder " << path << " exists\n" << RES;

	return 0;
}





// This function creates the header only for text/html requests, but not for images.
// If it is an image, then setImage() is called, where both header and body are created, and
// then setResponse() returns this full content, ready to be sent.
void ResponseData::setResponse(struct kevent& event) {
	std::cout << YEL "Start setResponse()\n" << RES;
	
	Request *storage = (Request *)event.udata;	
	_responseHeader += setResponseStatus(event);

	// NEED TO REMOVE THE SLASH AT THE END OF THE URL, IN CASE IT IS THERE, ie: .../location_random_dir/


	// DO WE HAVE AN INDICATOR, IF THE REQUEST IS A CGI?
	//	A)  NO CGI, RESPONSE IS JUST FILE OR IMAGE
	// 	B)	YES CGI, THE RESPONSE BODY IS STORED CGI CONTENT


	// A)
	std::string serverRootDir = storage->getServerData().getRootDirectory();

	// IF THE PATH IS A FOLDER, THIS FUNCTION NEEDS TO CHECK IF THERE IS A DEFAULT INDEX FILE PRESENT,
	// IF NOT, THEN CHECK IF AUTOINDEX IS ON,
	// 			IF YES, SEND HTML WITH FOLDER CONTENT
	//			IF NOT, SEND ERROR PAGE, NOT ALLOWED ?
	// if (storage->getRequestData().getIsFolder() == true) {
	if (storage->getRequestData().getIsFolder() == true && _isCgi == false) {
		std::cout << YEL "The Path is a folder: check for a default index file and/or autoindex on/off\n" << RES;
		std::cout << YEL "           Stored server root folder: [" << storage->getServerData().getRootDirectory() << "]\n" RES;
	//	std::cout << YEL "          local var. for root folder: [" << serverRootDir << "]\n" RES;
	//	std::cout << YEL "                             getURLPath: [" << storage->getRequestData().getURLPath() << "]\n" RES;

		//if (storage->getRequestData().getResponseContentType().compare("text/html") == 0) {		// IF FOLDER, THE CONT. TYPE SHOULD BE text.html
			
			// IF PATH IS THE SERVER ROOT "./"  (  ./resources/  )
			if (storage->getRequestData().getURLPath() == serverRootDir) {		// The path matches the server root
				std::cout << YEL "                The Path is the root: [" << storage->getRequestData().getURLPath() << "]\n" RES;
				_responsePath = storage->getServerData().getRootDirectory() + "/" + storage->getServerData().getIndexFile();
				_responsePath = storage->getServerData().getRootDirectory() + "/" + storage->getServerData().getIndexFile();
				std::cout << YEL "                       _responsePath: [" << _responsePath << "]\n" RES;
			}
			// IF PATH IS A FOLDER INSIDE THE ROOT FOLDER
			else {
				std::cout << YEL "          The URLpath is a folder inside the root: [" << storage->getRequestData().getURLPath() << "]\n" RES;

				// Here it should compare the path with available locations.
				// If a location is valid in the config file, then take the default index file inside that location
				// and append the filename after the path.
				std::vector<ServerLocation> location_data_vector = storage->getServerData().getLocationBlocks();
				size_t i;
				for (i = 0; i < location_data_vector.size(); i++) {
                    std::cout << GRE "   .... incoming URLpath: [" << storage->getRequestData().getURLPath() << "]\n";
                    std::cout << GRE "   ... location URI name: [" << location_data_vector[i].getLocationUriName() << "]\n";
                    std::cout << GRE "   ... location root dir: [" << location_data_vector[i].getRootDirectory() << "]\n";
                    std::cout << GRE "   ....... _responsePath: [" << _responsePath << "]\n";
					// if (location_data_vector[i].getRootDirectory() == _responsePath) {// TODO here it should be getLocationUriName()
					if (location_data_vector[i].getLocationUriName() == storage->getRequestData().getURLPath()) {// TODO here it should be getLocationUriName()
						_responsePath = location_data_vector[i].getRootDirectory() + "/" + location_data_vector[i].getIndexFile();
                    	std::cout << BLU "   ....... FinalPath: [" << location_data_vector[i].getRootDirectory() << "]\n";
						break ;
					}
				}

				// ???
				if (i == location_data_vector.size()) {
                    std::cout << RED "This path exists but does not match any location block: [" << _responsePath << "]\n";
                    std::cout << RED "		Here the _responsepath / error page needs to be set to 404 NOT FOUND\n";
					storage->setHttpStatus(NOT_FOUND);
				}

				// If chosen path filename is not in this folder, try default server filename:
				else {
					if (checkIfPathExists(_responsePath, event) == NOT_FOUND)
						_responsePath = location_data_vector[i].getRootDirectory() + "/" + storage->getServerData().getIndexFile();
					if (checkIfPathExists(_responsePath, event) == NOT_FOUND) {
						storage->setHttpStatus(FORBIDDEN);
	                    std::cout << RED "There is no such index file in this location: [" << _responsePath << "]\n";
						return ;
					}
				}
			}


			_responseBody = streamFile(_responsePath);
			std::cout << YEL "   getHttpStatus(): [" << storage->getHttpStatus() << "]\n" RES;// todo JOYCE map enums to strings
			std::cout << YEL "   response path:   [" << _responsePath << "]\n" RES;
			std::cout << YEL "   content type:    [" << storage->getRequestData().getResponseContentType() << RES "]\n";
		//}
	}
	// IF NOT A FOLDER
	else if (storage->getRequestData().getIsFolder() == false && _isCgi == false) {	// IF TEXTFILE
        std::cout << BLU "The path is a file: [" << _responsePath << "]\n" << RES;
		if (storage->getRequestData().getResponseContentType().compare("text/html") == 0)
			//_responseBody = streamFile(storage->getServerData().getRootDirectory() + "/" + _responsePath);
			_responseBody = streamFile(_responsePath);
		else {	// IF IMAGE, FULL RESPONSE IS CREATED IN setImage()
			_fullResponse = setImage(storage->getResponseData().getResponsePath());
			// std::cout << BLU "_fullResponse.length(): [\n" << _fullResponse.size() << "\n" RES;
			return ;
		}
	}

	// B) IF IT IS A CGI:
	if (_isCgi == true) {
		_responseBody = storage->getRequestData().getCgiBody();
		// std::cout << YEL "Content from CGI:\n[\n" RES << _responseBody << YEL "]\n" RES;
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
	//std::cout << YEL "\n_fullResponse:\n[\n" RES << _fullResponse << YEL "]\n" RES;

}


std::string ResponseData::setResponseStatus(struct kevent& event)
{	
	//std::cout << RED "start setResponseStatus\n";
	Request *storage = (Request *)event.udata;
	std::string status;

	std::string fileType = storage->getRequestData().getResponseContentType();	// fileType not used ?

	switch (storage->getHttpStatus())
	{
		case 400:
			status = "HTTP/1.1 400 Bad Request\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/400BadRequest.html");
			break;
		case 404:
			status = "HTTP/1.1 404 Not Found\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/404NotFound.html");
			break;
		case 405:
			status = "HTTP/1.1 405 Method Not Allowed\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/405MethodnotAllowed.html");
			break;
		case 408:
			status = "HTTP/1.1 408 Request Timeout\n"
						"Content-Type: text/html\n";
			(storage->getResponseData()).setResponsePath("resources/error_pages/408RequestTimeout.html");
			break;
		case 500:
			status = "HTTP/1.1 500 Internal Server Error";
			(storage->getResponseData()).setResponsePath("resources/error_pages/500InternarServerError.html");
		case 403:
			status = "HTTP/1.1 403 Forbidden";
			(storage->getResponseData()).setResponsePath("resources/error_pages/403Forbidden.html");
		default:
			status = "HTTP/1.1 200 OK\n"  
						"Content-Type: text/html\n";
					// "Content-Type: " + storage->getRequestData().getResponseContentType() + "\n";	// jaka
			//  _responsePath = storage->getRequestData().getHttpPath();							// jaka: this is old, should be getURLPath()
			_responsePath = storage->getRequestData().getURLPath();
			std::cout << GRN "_responsePath: [[" << _responsePath << "]]\n" RES;
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
	std::cout << BLU << "from setImage: content-length: " << temp << RES << "\n";
	contentLen.append(temp);
	headerBlock.append(contentLen);
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


void ResponseData::setIsCgi(bool b)	// added jaka
{
	_isCgi = b;
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

	std::cout << BLU << "File to be streamed: " << file << RES << std::endl;
	infile.open(file, std::fstream::in);
	if (not infile)
        throw ParserException(CONFIG_FILE_ERROR("File to be streamed", MISSING));
//		throw ServerException("Error: File not be opened for reading!");   SET UP ERROR
	while (infile)     // While there's still stuff left to read
	{
		std::string strInput;
		std::getline(infile, strInput);
		responseNoFav.append(strInput);
		responseNoFav.append("\n");
	}
	infile.close();

	//std::cout << "Streamed: " << responseNoFav << std::endl;
	std::cout << BLU << "Streamed: temp turned off by jaka" <<  RES << std::endl;
	return (responseNoFav);
}


// ***************************************************************************
// added JAKA, to erase the sent chunk from the remaining response content
std::string&	ResponseData::eraseSentChunkFromFullResponse(unsigned long retBytes) {
	return (_fullResponse.erase(0, retBytes));
}

// size_t	ResponseData::getCurrentLength() { // jaka
// 	return (_lengthFullResponse);
// }

size_t	ResponseData::getSentSoFar() { // jaka
	return (_bytesToClient);
}

// void 	ResponseData::setCurrentLength(size_t len) {
// 	_lengthFullResponse = len;
// }

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


bool ResponseData::getIsCgi()	// added jaka
{
	return (_isCgi);
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
