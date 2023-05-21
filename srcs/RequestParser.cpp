/* Example of simple URL for methods:
GET http://api.example.com/employee/<any_id>
POST http://api.example.com/employee/
PUT http://api.example.com/employee/
DELETE http://api.example.com/employee/1
*/

// CALLING CURL 
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d '{"Id": 79, "status": 3}'
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d 'abc'
// curl -X POST localhost:8080  -H "Content-Length: 444"  -H "Content-Type: text/html" -d 'abc'
// curl -X DELETE localhost:8080/resources/test_index.html -H "Content-Type: text/html"

// curl --resolve localhost:4243:127.0.0.1 http://localhost:4243
// curl --resolve whatever:8080:127.0.0.1 http://whatever:8080
// curl --resolve whatever:8080:0.0.0.0 http://whatever:8080
// curl --resolve testserver:8080:127.0.0.1 http://testserver:8080
// curl --resolve test:8080:127.0.0.1 http://test:8080

// curl -i -F 'filename=@/Users/jmurovec/Desktop/text_small.txt' http://localhost:8080/cgi/python_cgi_POST_upload.py

// random filename Z5z=k b0JaL9px!u8I]2'N[3Q6@vF.4f7qn~X$^j(s1l{T,y]+eK%=^_ .jpg

#include <unistd.h> // sleep
// #include <limits.h>
#include "RequestParser.hpp"

/** Default constructor */
Request::Request() {
	_kq = -1;
    _clientFd = -1;
    _data = RequestData();
    _answer = ResponseData();
    _cgi = CgiData();

    _headerDone = false;
    _doneParsing = false;
    _httpStatus = NO_STATUS;
    _redirection = std::string();
    _delete_is_allowed = false;
    _interpreter_path = "/usr/bin/python";
    _hasBody = false;
//    std::cout << GRY << "Request Constructor" << RES << std::endl;
}

/** Overloaded constructor */
Request::Request(int kq, int listeningSocket, int fd, std::vector<ServerData> servers) {
	_kq = kq;
	_listeningSocket = listeningSocket;
	_clientFd = fd;
	_servers = servers;
	_data = RequestData();
	_answer = ResponseData();
	_cgi = CgiData();

	_headerDone = false;
	_doneParsing = false;
    _httpStatus = NO_STATUS;
    _redirection = std::string();
    _delete_is_allowed = false;
    _interpreter_path = "/usr/bin/python";
	_hasBody = false;
//    std::cout << GRY << "Request Copy Constructor" << RES << std::endl;
}

/** Destructor */
Request::~Request() {
//    std::cout << GRY << "Request Destructor" << RES << std::endl;
}

/** #################################### Methods #################################### */

void Request::parseHeader(std::string header) {

//	std::string body;
	std::string lineContent;
	int i = 0;

	std::istringstream is(header);

	while (std::getline(is, lineContent)) {
		if (i == 0) {								// FIRST LINE HEADER
			storeWordsFromFirstLine(lineContent);
		} else if (i > 0 && lineContent != "\r") {	// OTHER HEADER LINES
			storeWordsFromOtherLine(lineContent);
		}
		i++;
	}
}


// !!!!!!! need to remove the file and links
int Request::storeWordsFromFirstLine(std::string firstLine) {
	std::vector<std::string> arr;
	std::istringstream iss(firstLine);
	std::string word;

	while(iss >> word)
		arr.push_back(word);
	int vecSize = arr.size();
	if (vecSize > 3) {				// maybe not needed
		std::cout << "Error: There are more then 3 items in the first line header\n";
		return (1);
	}
	std::vector<std::string>::iterator iter = arr.begin();
	for (int i = 0; iter < arr.end(); iter++, i++) {
		if (i == 0) {
			if (*iter == "GET" || *iter == "POST" || *iter == "DELETE") {
				std::cout << GRN << "REQUEST METHOD: " << *iter << RES << std::endl << std::endl;
                if (*iter == "GET")
				    _data.setRequestMethod(GET);
                if (*iter == "POST")
                    _data.setRequestMethod(POST);
                if (*iter == "DELETE")
                    _data.setRequestMethod(DELETE);
			} else {
				std::cout << RED << "Error: This method is not recognized\n" << RES;
				_httpStatus = METHOD_NOT_ALLOWED;
			}
		}
		else if (i == 1)
			_data.setRequestPath(*iter);
		else if (i == 2) {
			if (*iter != "HTTP/1.1" && *iter != "HTTP/1.0")	{	// maybe also HTTP/1.0 needed ??
				std::cout << RED << "Error: wrong http version\n" << RES;
                _httpStatus = HTTP_VERSION_NOT_SUPPORTED;
			}
			_data.setHttpVersion(*iter);
		}
	}
	return (0);
}

int Request::storeWordsFromOtherLine(std::string otherLine) {
	std::vector<std::string> arr;
	std::istringstream is(otherLine);
	std::string wordd;

	while(is >> wordd) {	// maybe not needed, each line has to go to a map member key: value
		arr.push_back(wordd);
	}
	std::vector<std::string>::iterator iter = arr.begin();
	for (int i = 0; iter < arr.end(); iter++, i++)
	{
		if (i == 0) {
			// Maybe all *iters must change tolower(), because key is case insensitive			
			if (*iter == "Accept:") {
				iter++;
				_data.setRequestAccept(*iter);
			}
			// else if (*iter == "Host:") {
			// 	iter++;
			// 	std::string temp = (*iter).substr(0, (*iter).find(":"));
			// 	_data.setRequestHost(temp);
			// }
			else if (*iter == "Host:") {
				iter++;
				std::string temp = (*iter).substr(0, (*iter).find(":"));
				_data.setRequestServerName(temp);
				std::string temp1 = (*iter).substr((*iter).find(":") + 1);
				_data.setRequestPort(temp1);
			}
			else if (*iter == "Content-Length:") {
				iter++;
				_data.setRequestContentLength(*iter);
			}
			else if (*iter == "Cookie:") {
				iter++;
				_data.setRequestCookie(*iter);
			}
			else if ((*iter).substr(0, 13) == "Content-Type:") {
				iter++;		
				if ((*iter).find("multipart") == std::string::npos)	
					_data.setRequestContentType(*iter);
				else			//				multipart;		boundary ---
					_data.setRequestContentType((*iter) + " " + *(++iter));
			}
		}
	}
	return (0);
}

void Request::setSpecificServer()
{
	std::vector<ServerData>::iterator it;

	for(it = _servers.begin(); it != _servers.end(); it++)
	{
		if (_listeningSocket == it->getListeningSocket())
		{
			if(getRequestData().getRequestServerName() == it->getServerName())
			{
				_specificServer = *it;
				return ;
			}
		}
	}
	for(it = _servers.begin(); it != _servers.end(); it++)
	{
		if (_listeningSocket == it->getListeningSocket())
		{
			_specificServer = *it;
			return ;
		}
	}
}

void Request::parseHeaderAndPath(std::string::size_type it) {
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Start parsing ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ fd: " << _clientFd << std::endl;

	_hasBody = true;
    std::string tmpHeader = _data.getHeader();
	tmpHeader.append(_data.getTemp().substr(0, it));
	_data.setHeader(tmpHeader);
	std::cout << BLU << "STORED HEADER: \n" << tmpHeader << "\n" << RES;
	parseHeader(_data.getHeader());
	setSpecificServer();
	std::cout << "__________________ _specificServer: " << _specificServer.getServerName() << std::endl << std::endl;
	
	_headerDone = true;
	if (_httpStatus == NO_STATUS || _httpStatus == OK)
        checkIfPathCanBeServed(_data.getHttpPath());// IF FILE NOT FOUND 404, IT COULD JUST CLOSE THE CONNECTION (return now?)
	else
        checkIfPathCanBeServed(getErrorPage());
	if (_data.getRequestContentLength() == 0){
		_doneParsing = true;
    }
}

// --------------------------------------------------------------- double functions / maybe combine -------------------------------------------

std::string Request::getSpecificErrorPage(std::vector<std::string> const & errorPages, std::string const & defaultErrorPage) {
    _data.setFileExtension(defaultErrorPage);
    std::vector<std::string>::const_iterator it = errorPages.cbegin();
    for (; it != errorPages.cend(); ++it) {
        if (it->find(std::to_string(_httpStatus)) != std::string::npos) {
            // If error page is found on the config file, return it
            return *it;
        }
    }
    // Otherwise return a default config file
    return defaultErrorPage;
}

std::string Request::getErrorPage()
{
	std::string temp = "";
    std::string defaultStatusPath = "./_server_default_status/";

	switch (_httpStatus) {
        case 301: {
            temp = _redirection;
            break;
        } case 400: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "400BadRequest.html");
            break;
        } case 403: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "403Forbidden.html");
            break;
        } case 404: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "404NotFound.html");
            break;
        } case 405: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "405MethodnotAllowed.html");
            break;
        } case 408: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "408RequestTimeout.html");
            break;
        } case 500: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "500InternarServerError.html");
            break;
		} case 504: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "504GatewayTimeout.html");
            break;
		} case 505: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), defaultStatusPath + "HTTPVersionNotSupported.html");
            break;
        } default: {
            std::cout << "_httpStatus: [[" << GRN << _httpStatus << RES << "]]\n";
            break;
        }
	}
	return (temp);
}

// ---------------------------------------------------------------------------------------------------------------------------------------


// OLD OLD OLD

 void Request::appendToRequest(const char str[], ssize_t len) {
     std::cout << PUR << "Start appendToRequest(): _hasBody: " << _hasBody << " | _doneParsing: " << _doneParsing << " \n" << RES;
     std::string strToFind = "\r\n\r\n";
     std::string::size_type it;
     if (_headerDone == false) {
         std::cout << "data.getTemp(): [" << _data.getTemp() << "]\n";
         _data.setTemp(_data.getTemp() + std::string(str));

         if ((it = _data.getTemp().find(strToFind)) != std::string::npos) {
             std::cout << YEL << "Found header ending /r/n, maybe there is body\n" << RES;
             parseHeaderAndPath(it);

             // needed to find the start of body, as char*, not std::string, because body will be a vector
             std::string::size_type it2 = _data.getTemp().find(strToFind) + strToFind.length();

             if (it2 != std::string::npos) {
                 if (std::string(str + it2).find("delete=") != std::string::npos) {
                     std::cout << "----------> Form data has \"delete=\" key on it\n" << RES;
                     _data.setFormDataHasDelete(true);
                 }
//                 std::cout << "recv ret inside appendToRequest = " << len << std::endl;
//                 std::cout << "it2 on appendToRequest = " << it2 << std::endl;
//                 std::cout << "len - it2 on appendToRequest = " << len - it2 << std::endl;
//                 std::cout << "BsF - it2 on appendToRequest = " << _data.getReqHeaderBytesSoFar() - it2 << std::endl;

                 appendLastChunkToBody(str + it2, len - it2);
			}
			return;
		}
	}
	if (_hasBody == true && _doneParsing == false) {
		 appendToBody(str, len); // changed to char*, because it needs to become a vector
	}
}

// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody(const char *str, ssize_t len) {
	_data.setClientBytesSoFar(len);


	if (_data.getClientBytesSoFar() > _data.getRequestContentLength()) {
		std::cout << RED << "Error: Body-Length, first chunk (" << _data.getClientBytesSoFar() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;
        _httpStatus = BAD_REQUEST;
		return (1);
	}

	if (_data.getClientBytesSoFar() > getServerData().getClientMaxBodySize()) {
		std::cout << RED "REQUEST BODY CONTENT TOO LARGE (max allowed: " << getServerData().getClientMaxBodySize() << ")\n" RES;
		_httpStatus = CONTENT_TOO_LARGE;
		return (1);
	}

	if (str != nullptr && len > 0 && len < LONG_MAX) {
		std::vector<uint8_t> tempVec(str, str + len); // convert and assign str to vector
		_data.setBody(tempVec);
	}

	if (_data.getClientBytesSoFar() == _data.getRequestContentLength()) {
		std::cout << GRN << "_doneparsing == true\n" << RES;
		_doneParsing = true;
		if (_data.getClientBytesSoFar() == 0 && _data.getRequestContentLength() == 0) {    // Compare body length
			std::cout << YEL << "There is no body\n" << RES;
			_hasBody = false;
			std::cout << BLU "content type: [" << _data.getResponseContentType() << "]\n" RES;
			return (0);
		}
		std::cout << GRN << "OK: Body-Length is as expected Content-Length\n" << RES;
		_doneParsing = true; // otherwise it went to appendToBody, and appended more stuff, so the body lenght became larger then expected
		return (0);
	}
	return (0);
}

int Request::appendToBody(const char* str, ssize_t len) {
	// std::cout << RED << "appendToBody - str: " << str << RES << std::endl;
	std::cout << RED << "appendToBody - len: " << len << RES << std::endl;
	std::cout << RED << "appendToBody - str + len: [" << str + len << "]\n" << RES << std::endl;
	std::cout << RED << "        getBody().size():" << _data.getBody().size() << "\n" << RES << std::endl;

	//std::cout << CYN << "    body after append:  [" << _data.getBody() << "]\n" << RES;
	//std::cout << CYN << "    getClientMaxBodySize:  [" << getServerData().getClientMaxBodySize() << "]\n" << RES;
	
	if (_data.getClientBytesSoFar() > getServerData().getClientMaxBodySize()) {
		std::cout << RED "REQUEST BODY CONTENT TOO LARGE\n" RES;
		_httpStatus = CONTENT_TOO_LARGE;
		return (1);
	}
	if (_data.getClientBytesSoFar() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED << "Error: Body-Length (" << _data.getClientBytesSoFar() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;// sleep(2);
        _httpStatus = BAD_REQUEST;
		return (1);
	}

	_data.setClientBytesSoFar(len);
	// setting the vector if no error was returned
    if (str != nullptr && len > 0 && len < LONG_MAX) {
        std::vector<uint8_t> newChunk(str, str + len); // convert and assign str to vector
        std::vector<uint8_t> &tmp = _data.getBody();
        tmp.reserve(_data.getRequestContentLength() + len);
        tmp.insert(tmp.end(), newChunk.begin(), newChunk.end());
        _data.setBody(tmp);
    }

	if (_data.getClientBytesSoFar() == _data.getRequestContentLength()) {
		std::cout << GRN << "OK: Done parsing.\n" RES;
		_doneParsing = true;
		return (0);
	}
	return (0);
}

// JUST FOR CHECKING
void Request::printStoredRequestData(Request &request) {
	RequestData reqData = request.getRequestData();
	// PRINT FIRST LINE HEADER
	std::cout << "\nFIRST LINE:\n[" RED << reqData.getRequestMethod() << ", "
										<< reqData.getHttpPath() << ", "
										<< reqData.getHttpVersion() << RES "]\n";
	// PRINT OTHER HEADERS	
	std::cout << "HEADER FIELDS:\n" BLU;
	std::cout << "Host:           [" << reqData.getRequestPort() << "]\n";
	std::cout << "Server Name     [" << reqData.getRequestServerName() << "]\n";
	std::cout << "Accept:         [" << reqData.getRequestAccept() << "]\n";
	std::cout << "Content-Length: [" << reqData.getRequestContentLength() << "]\n";
	std::cout << "Content-Type:   [" << reqData.getResponseContentType() << "]\n" RES;
	// PRINT BODY
//	std::cout << "REQUEST BODY:\n[" PUR << request.getRequestBody() << RES "]\n";
}

/** #################################### Getters #################################### */

RequestData & Request::getRequestData(){
    return _data;
}

ServerData & Request::getServerData(){
    return _specificServer;
}

ResponseData & Request::getResponseData(){
    return (_answer);
}

CgiData & Request::getCgiData(){
    return (_cgi);
}

int Request::getKq()
{
    return (_kq);
}

bool Request::getDone()
{
	return (_doneParsing);
}

HttpStatus Request::getHttpStatus()
{
	return (_httpStatus);
}

std::string Request::getRedirection()
{
    return (_redirection);
}

bool Request::deleteIsAllowed()
{
    return (_delete_is_allowed);
}

std::string Request::getInterpreterPath()
{
    return (_interpreter_path);
}

int Request::getFdClient()
{
	return (_clientFd);
}

/** #################################### Setters #################################### */

void Request::setDone(bool val)
{
	_doneParsing = val;
}

void Request::setHttpStatus(HttpStatus val)
{
    _httpStatus = val;
}

void Request::setRedirection(std::string const & redirection)
{
    _redirection = redirection;
}

void Request::setDeleteIsAllowed(bool b)
{
    _delete_is_allowed = b;
}

void Request::setInterpreterPath(std::string const & interpreter_path)
{
    _interpreter_path = interpreter_path;
}
