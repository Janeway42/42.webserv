#include "RequestParser.hpp"
#include "ResponseData.hpp"
#include "RequestData.hpp"

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
//    std::cout << GRY_BG << "ResponseData Default Constructor" << RES << std::endl;
}

/** Destructor */
ResponseData::~ResponseData(void) {
//    std::cout << GRY_BG << "ResponseData Destructor" << RES << std::endl;
}

void ResponseData::createResponseHeader(struct kevent& event) {

	Request *storage = (Request *)event.udata;

    std::string content_length = "Content-Length: " + std::to_string(_responseBody.length()) + "\r\n";

    // HACK FOR 42 TESTER
    if (_responsePath.find("youpi.bla") != std::string::npos && storage->getRequestData().getRequestMethod() == POST) {
        content_length = "";
    }

    std::string redirection = storage->getRedirection().empty() ? "" : "Location: " + storage->getRedirection() + "\r\n";

	std::string cookiesHeader = "";
	std::cout << PUR << "cookies in _data: " << storage->getRequestData().getRequestCookie() << RES << std::endl;
	if (storage->getRequestData().getRequestCookie() != "")
		cookiesHeader = "Set-Cookie: " + storage->getRequestData().getRequestCookie() + "\r\n";

	_responseHeader = "HTTP/1.1 " + std::to_string(storage->getHttpStatus()) + " " + httpStatusToString(storage->getHttpStatus()) + "\r\n"
        // "Content-Type: text/html\r\n"
        "Content-Type: " + storage->getRequestData().getResponseContentType() + "\r\n"
        "Content-Encoding: identity\r\n"
        + cookiesHeader
        + content_length
        + redirection +
        "Connection: close\r\n"
        "\r\n";
}

/** #################################### Methods #################################### */

// void ResponseData::createResponseHeader(HttpStatus status, std::string const & redirectionUrl) {

//     // For cookies
//     // if (responsePath is in the setCookies folder)  // function getLocation().getCookies() to be written
//     // 	header.append("Cookies: " + storage->getServerData().getLocationCookies + "\r\n");  // find the location bloc and it's cookies
//     // if (storage->getRequestData().getRequestCookies() != "")
//     // {createResponseHeader(
//     // 	header.append("Cookies: " + storage->getRequestData().getRequestCookies() + "\r\n");
//     // 	if (storage->getRequestData().getURLPath() == "resources/cookies/noCookies.html")
//     // 		_responsePath = "resources/cookies/yesCookies.html"
//     // }

//     std::string redirection = redirectionUrl.empty() ? "" : "Location: " + redirectionUrl;

//     _responseHeader = "HTTP/1.1 " + std::to_string(status) + " " + httpStatusToString(status) + "\r\n"
//         "Content-Type: text/html\r\n"//TODO WE CONT HAVE MORE CONTENT TYPES?
//         "Content-Encoding: identity\r\n"// Corina - added it because firefox complained that it was missing - not sure if we keep because firefox still complains even with it. We leave it for now.
//         "Connection: close\r\n"
//         "Content-Length: " + std::to_string(_responseBody.length()) + "\r\n"
//         + redirection + "\r\n\r\n";
// }

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
 * then createResponse() returns this full content, ready to be sent
 */
void ResponseData::createResponse(struct kevent& event) {
	std::cout << CYN <<  "Start " << __func__ << RES << std::endl;
	Request *storage = (Request *)event.udata;

	std::cout << "fd: " << storage->getFdClient() << std::endl;
    std::cout << "   _responseBody: [" << _responseBody << "]\n";   // to remove, jaka


    // If the Form data that came from the body contained a "delete=" key, and the current request is POST or DELETE
    // we can return 405 Method Not Allowed
    if (storage->getRequestData().formDataHasDelete() && not storage->deleteIsAllowed()) {//} || storage->getRequestData().getRequestMethod() == POST)) {
        std::cout << RED << "Location does not accept the Method - 405 Method not allowed will be returned" << RES << std::endl;
        storage->setHttpStatus(METHOD_NOT_ALLOWED);
    }

    _responsePath = storage->getRequestData().getURLPath_full();
	setResponseStatus(event);

    std::cout << BLU << "_responsePath after SetResponseStatus(): " << _responsePath << "]" << RES << std::endl;
    std::cout << BLU << "getURLPath:      [" << storage->getRequestData().getURLPath() << "]" << RES << std::endl;
    std::cout << BLU << "getFullPath:     [" << storage->getRequestData().getURLPath_full() << "]" << RES << std::endl;
    std::cout << BLU << "getHttpStatus(): [" << storage->getHttpStatus() << "]" << RES << std::endl;
    std::cout << BLU << "response path:   [" << _responsePath << "]" << RES << std::endl;
    std::cout << BLU << "content type:    [" << storage->getRequestData().getResponseContentType() << "]" << RES << std::endl;

    if (storage->getRedirection().empty()) {
        if (storage->getRequestData().getIsFolder() && not storage->getCgiData().getIsCgi()) {
            /* No need to loop through the locations to find the matching,  the getURLPath_full() already contains
             * the matching one. Also, no need to append the corresponding index file (checkIfPathCanBeServed() has done it) */
            std::cout << "The Path is a folder: Checking for autoindex on/off:\n";

            // Handling auto index
            if (storage->getRequestData().getAutoIndex()) {
                std::cout << "AUTOINDEX ON, must call storeFolderContent()\n";
                std::cout << "     URLPathFirstPart: [" << storage->getRequestData().getURLPathFirstPart() << "]" << std::endl;
                // if (storage->getRequestData().getURLPathFirstPart().empty()) {
                // 	std::cout << BLU "first part is empty\n" << RES;
                // 	std::string newPath = "./resources/" + storage->getRequestData().getURLPath();
                // 	_responseBody = storeFolderContent(newPath.c_str());
                // } else
                std::cout << "Setting _responseBody\n";
                _responseBody = storeFolderContent(storage->getRequestData().getURLPathFirstPart().c_str());
                if (not _responseBody.empty()) {
                    storage->setHttpStatus(OK);
                }
            } else {
                std::cout << "AutoIndex off. Setting _responseBody\n";
                _responseBody = streamFile(_responsePath);
            }
        }
        // if it is not a folder (it checks first if cgi -> if not then it checks text (includes error), else image
        else {
            if (storage->getCgiData().getIsCgi() && storage->getHttpStatus() == OK) {
                std::cout << GRN << "The path is a file to send to a cgi interpreter: [" << GRN << _responsePath << "]" << RES << std::endl;
                std::cout << "Setting _responseBody\n";
                _responseBody = storage->getRequestData().getCgiBody();
                std::cout << "Setting cgi _responseBody: [" << _responseBody << "]\n";

                // HACK FOR 42 TESTER
                if (_responsePath.find("youpi.bla") != std::string::npos && storage->getRequestData().getRequestMethod() == POST) {
                    _responseBody += "./TESTS/42TESTER/YOUPIBANANE/YOUPI.BLA";
                    _responseBody += "\0";
                    storage->setHttpStatus(NO_STATUS);
                }
            } else {
                if (storage->getRequestData().getResponseContentType() == "text/html" || storage->getRequestData().getResponseContentType() == "application/pdf") {
                    std::cout << GRN << "The path is a file: [" << GRN << _responsePath << "]" << RES << std::endl;
                    _responseBody = streamFile(_responsePath);
                    // std::cout << "Setting _responseBody: [" << _responseBody << "]\n";
                }
                // IF IMAGE, FULL RESPONSE IS CREATED IN setImage()
                else if (storage->getRequestData().getResponseContentType().find("image/") != std::string::npos) {
                    std::cout << GRN << "The path is an image: [" << GRN << _responsePath << "]" << RES << std::endl;
                    _fullResponse = setImage(_responsePath);
                    // std::cout << BLU "_fullResponse.length(): [\n" << _fullResponse.size() << "\n" RES;
                    return;
                }
            }
        }
        if (_responseBody.empty() && storage->getHttpStatus() != NO_STATUS) {
            if (storage->getRequestData().getIsFolder() && pathType(_responsePath) != DIRECTORY) {
                storage->setHttpStatus(NOT_FOUND);
            } else {
                storage->setHttpStatus(INTERNAL_SERVER_ERROR);
            }
            setResponseStatus(event);
            std::cout << "CHECK IF STRING WAS EMPTY, responspath: "  << _responsePath << "\n";
            _responseBody = streamFile(_responsePath);
        }

        // HACK FOR 42 TESTER
        if (_responsePath.find("youpi.bla") != std::string::npos && storage->getRequestData().getRequestMethod() == POST) {
            storage->setHttpStatus(OK);
        }
    }
	// set up header
	createResponseHeader(event);
	_fullResponse += _responseHeader + _responseBody;
     std::cout << "-------------\n_responseHeader:\n" RES << _responseHeader << "\n-------------" << std::endl;
     std::cout << "-------------\n_responseBody:\n" RES << _responseBody << "\n-------------" << std::endl;
}

std::string getSpecificErrorPage(Request* storage, std::vector<std::string> const & errorPages, std::string const & defaultErrorPage) {
    Parser parser;
    storage->getRequestData().setFileExtension(defaultErrorPage);
    std::vector<std::string>::const_iterator it = errorPages.cbegin();
    for (; it != errorPages.cend(); ++it) {
             std::cout << "errorPages:\n" RES << *it << "\n-------------" << std::endl;
        if (it->find(std::to_string(storage->getHttpStatus())) != std::string::npos) {
            // If error page is found on the config file AND it exists, return it
            if (parser.pathType(*it) == REG_FILE) {
                std::cout << "Error page on config file will be used" << std::endl;
                return *it;
            }
        }
    }
    // Otherwise return a default config file
    std::cout << "Internal server default error page will be used" << std::endl;
    return defaultErrorPage;
}

void ResponseData::setResponseStatus(struct kevent& event)
{
	Request *storage = (Request *)event.udata;
	std::cout << CYN << "Start setResponseStatus(), current status: "<< storage->getHttpStatus() << "\n" << RES;

    std::string defaultStatusPath = "./_server_default_status/";
    //todo add more default pages?
	switch (storage->getHttpStatus()) {
        case 301: {
            break;
        } case 400: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "400BadRequest.html");
            break;
        } case 403: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "403Forbidden.html");
            break;
        } case 404: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "404NotFound.html");
            break;
        } case 405: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "405MethodnotAllowed.html");
            break;
        } case 408: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "408RequestTimeout.html");
            break;
		} case 413: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "413ContentTooLarge.html");
            break;
        } case 500: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "500InternarServerError.html");
            break;
		} case 504: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "504GatewayTimeout.html");
            break;
		} case 505: {
            _responsePath = getSpecificErrorPage(storage, storage->getServerData().getErrorPages(), defaultStatusPath + "505HttpVersionNotSupported.html");
            break;
        } default: {
            // "Set-Cookie: id=123; jaka=500; Max-Age=10; HttpOnly\r\n";
            std::cout << "_responsePath: [[" << GRN << _responsePath << RES << "]]\n";
            break;
        }
	}
}


std::string ResponseData::setImage(std::string imagePath) {
	//std::cout << RED "Start SetImage()\n" RES;

	std::fstream imageFile;		// Stream image and store it into a string
	std::string content;
	imageFile.open(imagePath);
    if (not imageFile)
        return ("");

	content.assign(std::istreambuf_iterator<char>(imageFile), std::istreambuf_iterator<char>());
	content += "\r\n";
	imageFile.close();

	// std::string contentType = getContent...  				// Here it needs to grab the correct Type, jpg, png, gif, ico ...

	// Create the header block
	std::string headerBlock = 	"HTTP/1.1 200 OK\r\n"
								"Content-Type: image/jpg\r\n"
								"Content-Encoding: identity\r\n"	// Here it needs to grab the correct Type, jpg, png, gif, ico ...
								"Connection: close\r\n"
	                            "accept-ranges: bytes\r\n";
	std::string contentLen = "Content-Length: ";
	std::string temp = std::to_string(content.size());
	std::cout << BLU << "from setImage: content-length: " << temp << RES << "\n";
	contentLen.append(temp);
	headerBlock.append(contentLen);
	headerBlock.append("\r\n\r\n");

	headerBlock.append(content);
	return (headerBlock);	// Both header and image content
}

// std::string ResponseData::streamFileOLD(std::string file) {
// 	std::string responseNoFav;
// 	std::fstream    infile;

// 	std::cout << GRN << "File to be streamed: " << file << RES << std::endl;
// 	infile.open(file, std::fstream::in);
// 	if (not infile)
//         return ("");
// 	while (infile) // While there's still stuff left to read
// 	{
// 		std::string strInput;
// 		std::getline(infile, strInput);
//         if (not infile)
//             return ("");
// 		responseNoFav.append(strInput);
// 		responseNoFav.append("\n");
// 	}
// 	infile.close();
// //	std::cout << "Streamed: " << responseNoFav << std::endl;
// 	return (responseNoFav);
// }

std::string ResponseData::streamFile(std::string file) {
	std::string responseNoFav;
	std::fstream    infile;
	std::cout << GRN << "File to be streamed: " << file << RES << std::endl;
	infile.open(file, std::fstream::in);
	if (infile.is_open()) {
	    while (infile) // While there's still stuff left to read
	    {
            std::string strInput;
            std::getline(infile, strInput);
            responseNoFav.append(strInput);
            responseNoFav.append("\n");
	    }
        //	std::cout << "Streamed: " << responseNoFav << std::endl;
	    infile.close();
	    return (responseNoFav);
    }
    return std::string();
}





std::string&	ResponseData::eraseSentChunkFromFullResponse(unsigned long retBytes) {
	return (_fullResponse.erase(0, retBytes));
}

void	ResponseData::increaseSentSoFar(ssize_t bytesSent) {
	_bytesToClient += bytesSent;
}
// void ResponseData::overrideFullResponse()  // NOT USED anymore - to be cleaned out
// {
// 	std::string errorMessage = " insert html code";
// 	_fullResponse = errorMessage;
// 	_errorOverride = true;
// }

// size_t	ResponseData::getCurrentLength() { // jaka
// 	return (_lengthFullResponse);
// }

// void 	ResponseData::setCurrentLength(size_t len) {
// 	_lengthFullResponse = len;
// }

/** #################################### Setters #################################### */

void ResponseData::setResponseBody(std::string file)
{
    _responseBody += file;
}

void ResponseData::setBytesToClient(ssize_t val)
{
    _bytesToClient += val;
}

void ResponseData::setResponsePath(std::string path)
{
    _responsePath = path;
}

void ResponseData::setResponseDone(bool val)
{
    _responseDone = val;
}

void ResponseData::setResponseFull(std::string response)
{
	_fullResponse = response;
}

// void ResponseData::setOverride(bool val)   // NOT USED anymore - to be cleanned out
// {
// 	_errorOverride = val;
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

size_t	ResponseData::getSentSoFar() { // jaka
    return (_bytesToClient);
}

// bool ResponseData::getOverride()  // NOT USED - To be cleaned out!
// {
// 	return (_errorOverride);
// }
