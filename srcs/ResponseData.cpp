#include "RequestParser.hpp"
#include "ResponseData.hpp"
#include "RequestData.hpp"

#include <string.h> // jaka, temp, can be removed

/** Default constructor */
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
	_responseDone = false;
	// _errorOverride = false;
    std::cout << GRY_BG << "ResponseData Default Constructor" << RES << std::endl;
}

/** Destructor */
ResponseData::~ResponseData(void) {
    std::cout << GRY_BG << "ResponseData Destructor" << RES << std::endl;
}

/** #################################### Setters #################################### */

/* If the path from the URL is:
 * - Directory:
 *      - the correct index file was already added to the storage->getRequestData().getURLPath_full()
 *      - the storage->getRequestData().getIsFolder() will return true (even for requests that does not finish with /)
 *      - auto index allowed:
 *          - if auto index is off: Error status 404
 *          - if auto index is on and directory does not exist: Error status 404
 *          - if auto index is on: send the html for the directory content
 * - File:
 *      - the storage->getRequestData().getExtension() wll return the extension of the requested file
 *      - auto index not allowed
 * - Query:
 *      - the storage->getRequestData().getIsCgi() will return true
 *      - the storage->getRequestData().getExtension() wll return the extension of the requested file
 *      - auto index not allowed
 * --------------------------------------------------------------------------------------------------------------------
 * This function creates the header only for text/html requests, but not for images.
 * If it is an image, then setImage() is called, where both header and body are created, and
 * then setResponse() returns this full content, ready to be sent
 */
void ResponseData::setResponse(struct kevent& event) {
	std::cout << CYN <<  "Start setResponse()\n" << RES;
	Request *storage = (Request *)event.udata;
    _responsePath = storage->getRequestData().getURLPath_full();
	_responseHeader += setResponseStatus(event);


	if (storage->getRequestData().getIsFolder() == true && storage->getCgiData().getIsCgi() == false) {
        /* No need to loop through the locations to find the matching,  the getURLPath_full() already contains
         * the matching one. Also, no need to append the corresponding index file */
         std::cout << BLU "The Path is a folder: check for autoindex on/off\n" << RES;
         std::cout << BLU "   getURLPath: [" << storage->getRequestData().getURLPath() << "]\n" RES;
         std::cout << BLU "  getFullPath: [" << storage->getRequestData().getURLPath_full() << "]\n" RES;

		//if (storage->getRequestData().getResponseContentType().compare("text/html") == 0) { // IF FOLDER, THE CONT. TYPE SHOULD BE text.html todo use this check still?
        if (storage->getHttpStatus() == OK) {
            std::cout << "Setting _responsePath and _responseBody\n";
			_responseBody = streamFile(_responsePath);

			std::cout << "   getHttpStatus(): [" << storage->getHttpStatus() << "]\n" << RES;
			std::cout << "   response path:   [" << _responsePath << "]\n" << RES;
			std::cout << "   content type:    [" << storage->getRequestData().getResponseContentType() << "]\n" << RES;
        }
        // Handling auto index
        if (storage->getRequestData().getAutoIndex() == true) {
			std::cout << BLU "AUTOINDEX ON, must call storeFolderContent()\n" << RES;
			std::cout << BLU "     URLPathFirstPart: [" << storage->getRequestData().getURLPathFirstPart() << "]\n" << RES;
			// if (storage->getRequestData().getURLPathFirstPart().empty()) {
			// 	std::cout << BLU "first part is empty\n" << RES;
			// 	std::string newPath = "./resources/" + storage->getRequestData().getURLPath();
			// 	_responseBody = storeFolderContent(newPath.c_str());
			// }
			// else
	        _responseBody = storeFolderContent(storage->getRequestData().getURLPathFirstPart().c_str());
		}
	}
    // if it is not a folder (it checks first if cgi -> if not then it checks text (includes error), else image
    else {
        if (storage->getCgiData().getIsCgi() == true && storage->getHttpStatus() == OK) {
            _responseBody = storage->getRequestData().getCgiBody();
        } else {
            if (storage->getRequestData().getResponseContentType().compare("text/html") == 0) {
                std::cout << GRN << "The path is a file: [" << GRN_BG << _responsePath << RES << "]\n";
                _responseBody = streamFile(_responsePath);
                //_responseBody = streamFile(storage->getServerData().getRootDirectory() + "/" + _responsePath);
            }
            else {	// IF IMAGE, FULL RESPONSE IS CREATED IN setImage()
                std::cout << GRN << "The path is an image: [" << GRN_BG << _responsePath << RES << "]\n";
                _fullResponse = setImage(_responsePath);
                // std::cout << BLU "_fullResponse.length(): [\n" << _fullResponse.size() << "\n" RES;
                return ;
            }
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

	// std::cout << GRN << "complete response header: [" << _responseHeader << "]\n" << RES;
	_fullResponse += _responseHeader + _responseBody;
	//std::cout << GRN << "\n_fullResponse:\n[\n" RES << _fullResponse << "]\n" <<RES;
}

static std::string selectErrorPage(std::vector<std::string> const & errorPages, HttpStatus status, std::string const & defaultErrorPage) {
    std::vector<std::string>::const_iterator it = errorPages.cbegin();
    for (; it != errorPages.cend(); ++it) {
        if (it->find(std::to_string(status)) != std::string::npos) {
            // If error page is found on the config file, return it
            return *it;
        }
    }
    // Otherwise return a default config file
    return "./resources/_server_default_status/" + defaultErrorPage;
}

static std::string setResponseHeader(HttpStatus status) {
    return "HTTP/1.1 " + std::to_string(status) + " " + httpStatusToString(status) + "\r\n"
           "Content-Type: text/html\r\n"
           "Content-Encoding: identity\r\n"// Corina - added it because firefox complained that it was missing - not sure if we keep because firefox still complains even with it. We leave it for now.
           "Connection: close\r\n";
}

std::string ResponseData::setResponseStatus(struct kevent& event)
{
	std::cout << CYN << "start setResponseStatus()\n" << RES;
	Request *storage = (Request *)event.udata;
	std::string header = setResponseHeader(storage->getHttpStatus());
//	std::string fileType = storage->getRequestData().getResponseContentType();	// fileType not used ?

    //todo add more default pages?
	switch (storage->getHttpStatus()) {
        case 301: {
            header = header + "Location: " + storage->getRedirection() + "close\r\n";
            break;
        } case 400: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "400BadRequest.html");
            break;
        } case 403: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "403Forbidden.html");
            break;
        } case 404: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "404NotFound.html");
            break;
        } case 405: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "405MethodnotAllowed.html");
            break;
        } case 408: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "408RequestTimeout.html");
            break;
        } case 500: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "500InternarServerError.html");
            break;
		} case 505: {
            _responsePath = selectErrorPage(storage->getServerData().getErrorPages(),
                                            storage->getHttpStatus(),
                                            "500HttpVersionNotSupported.html");
            break;
        } default: {
            // "Set-Cookie: id=123; jaka=500; Max-Age=10; HttpOnly\r\n";
            // "Content-Type: " + storage->getRequestData().getResponseContentType() + "\n";	// jaka
            std::cout << "_responsePath: [[" << GRN_BG << _responsePath << RES << "]]\n";
            break;
        }
	}
    // if (responsePath is in the setCookies folder)  // function getLocation().getCookies() to be written
    // 	status.append("Cookies: " + storage->getServerData().getLocationCookies + "\r\n");  // find the location bloc and it's cookies
    // if (storage->getRequestData().getRequestCookies() != "")
    // {
    // 	status.append("Cookies: " + storage->getRequestData().getRequestCookies() + "\r\n");
    // 	if (storage->getRequestData().getURLPath() == "resources/cookies/noCookies.html")
    // 		_responsePath = "resources/cookies/yesCookies.html"
    // }
	return (header);
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
								"Content-Type: image/jpg\r\n"
								"Content-Encoding: identity\r\n"	// Here it needs to grab the correct Type, jpg, png, gif, ico ...
								"Connection: close\r\n";
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

void ResponseData::setResponsePath(std::string path)
{
	_responsePath = path;
}

void ResponseData::setResponseDone(bool val)
{
	_responseDone = val;
}

// void ResponseData::setOverride(bool val)   // NOT USED anymore - to be cleanned out
// {
// 	_errorOverride = val;
// }

// --------------------------------------------------------------------------- util functions
// ------------------------------------------------------------------------------------------

// void ResponseData::overrideFullResponse()  // NOT USED anymore - to be cleaned out
// {
// 	std::string errorMessage = " insert html code";
// 	_fullResponse = errorMessage;
// 	_errorOverride = true;
// }

std::string ResponseData::streamFile(std::string file)
{
	std::string responseNoFav;
	std::fstream    infile;

	std::cout << GRN << "File to be streamed: " << file << RES << std::endl;
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

size_t	ResponseData::getSentSoFar() { // jaka
	return (_bytesToClient);
}

void	ResponseData::increaseSentSoFar(size_t bytesSent) {
	_bytesToClient += bytesSent;
}

// size_t	ResponseData::getCurrentLength() { // jaka
// 	return (_lengthFullResponse);
// }

// void 	ResponseData::setCurrentLength(size_t len) {
// 	_lengthFullResponse = len;
// }

/** #################################### Getters #################################### */

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

bool ResponseData::getResponseDone()
{
	return (_responseDone);
}

// bool ResponseData::getOverride()  // NOT USED - To be cleaned out!
// {
// 	return (_errorOverride);
// }

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
