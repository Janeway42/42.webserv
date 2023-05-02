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
// curl --resolve testserver:8080:127.0.0.1 http://testserver:8080


#include <unistd.h> // sleep
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
    _hasBody = false;
    std::cout << GRY << "Request Constructor" << RES << std::endl;
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
	_hasBody = false;
    std::cout << GRY << "Request Copy Constructor" << RES << std::endl;
}

/** Destructor */
Request::~Request() {
    std::cout << GRY << "Request Destructor" << RES << std::endl;
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
			// Store key:value pairs into vector or map
		}
		i++;
	}
}

// void Request::storeBody(std::istringstream &iss)// TODO NOT NEEDED ??
// {
// 	std::string lineContent;
// 	std::string tmp;

// 	while (std::getline(iss, lineContent)) {
// 		tmp = _data.getBody();
// 		tmp.append(lineContent);
// 		_data.setBody(tmp);
// 		// _body.append(lineContent);
// 	}
// 	std::cout << YEL "body [" << _data.getBody() << "]\n" << RES;
// }

// TODO MAYBE WON'T BE NEEDED
void	storeBodyAsFile(std::string body) {
	std::ofstream bodyFile("./resources/_cgi/bodyFile.txt");
	if (bodyFile.is_open()) {
		bodyFile << body;
		bodyFile.close();
		std::cout << GRN "Body string written succesfuly to the file\n" RES;
	}
	else {
		std::cout << RED "Error opening the file to write the body into\n" RES;
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
				std::cout << GRN_BG << YEL << "REQUEST METHOD: " << *iter << RES << std::endl << std::endl;
				_data.setRequestMethod(*iter);
			} else {
				std::cout << RED << "Error: This method is not recognized\n" << RES;
				_httpStatus = METHOD_NOT_ALLOWED; 
			}
		}
		else if (i == 1)
			_data.setRequestPath(*iter);
		else if (i == 2) {
			if (*iter != "HTTP/1.1" && *iter != "HTTP/1.0")	{	// maybe also HTTP/1.0 needed ??
				// TODO: SET CORRECT STATUS ERROR
				std::cout << RED << "Error: wrong http version\n" << RES;
                _httpStatus = HTTP_VERSION_NOT_SUPPORTED;
			}
			_data.setHttpVersion(*iter);
		}
	}
	return (0);
}

int Request::storeWordsFromOtherLine(std::string otherLine) {
	//std::cout << GRN " .... .... header line: [" << otherLine << "]\n" RES;
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
			else if (*iter == "Host:") {
				iter++;
				std::string temp = (*iter).substr(0, (*iter).find(":"));
				_data.setRequestHost(temp);
			}
			else if (*iter == "Content-Length:") {
				iter++;
				_data.setRequestContentLength(*iter);
			}
			else if ((*iter).substr(0, 13) == "Content-Type:") {
				iter++;		
				if ((*iter).find("multipart") == std::string::npos)	
					_data.setRequestContentType(*iter);
				else			//				multipart;		boundary ---
					_data.setRequestContentType((*iter) + " " + *(++iter));
				
				//std::cout << RED " .... .... header line *Iter: [" << *iter << "]\n" RES;
				//std::cout << RED " .... .... header line Content-Type: [" << _data.getRequestContentType() << "]\n" RES;

			}
			// else
			// 	std::cout << RED << "Error: This method is not recognized - other line\n" << RES;
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
			if(getRequestData().getRequestHost() == it->getServerName())
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


/* TODO
 * - What if method is GET (normaly without body) AND content-length is not zero ???
 * - What if method POST and content-length is zero ???
 */

void Request::parseHeaderAndPath(std::string & tmpHeader, std::string::size_type it) {
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Start parsing ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ fd: " << _clientFd << std::endl;

	_hasBody = true;
	tmpHeader = _data.getHeader();
	tmpHeader.append(_data.getTemp().substr(0, it));
	_data.setHeader(tmpHeader);
	std::cout << BLU << "STORED HEADER: \n" << tmpHeader << "\n" << RES;
	parseHeader(_data.getHeader());
	setSpecificServer();

	std::cout << "-------------------- _specificServer: " << _specificServer.getServerName() << std::endl;
	
	_headerDone = true;
	//std::cout << RED "server root path: " << getServerData().getRootDirectory() << "\n" RES;
	if (_httpStatus == NO_STATUS || _httpStatus == OK)
		parsePath(_data.getHttpPath());	// IF FILE NOT FOUND 404, IT COULD JUST CLOSE THE CONNECTION (return now?)
	else
		parsePath(getErrorPage());

	if (_data.getRequestContentLength() == 0){
		// if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "")
		//	; //callCGI(getRequestData(), fdClient); 		// moved to chooseMethod_StartAction()
		_doneParsing = true;
    }
}


// --------------------------------------------------------------- double functions / maybe combine -------------------------------------------

static std::string getSpecificErrorPage(std::vector<std::string> const & errorPages, HttpStatus status, std::string const & defaultErrorPage) {
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

std::string Request::getErrorPage()
{
	std::string temp = "";

	switch (_httpStatus) {
        case 301: {
            temp = _redirection;
            break;
        } case 400: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "400BadRequest.html");
            break;
        } case 403: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "403Forbidden.html");
            break;
        } case 404: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "404NotFound.html");
            break;
        } case 405: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "405MethodnotAllowed.html");
            break;
        } case 408: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "408RequestTimeout.html");
            break;
        } case 500: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "500InternarServerError.html");
            break;
		} case 504: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "504GatewayTimeout.html");
            break;
		} case 505: {
            temp = getSpecificErrorPage(_specificServer.getErrorPages(), _httpStatus,
                                            "HTTPVersionNotSupported.html");
            break;
        } default: {
            // "Set-Cookie: id=123; jaka=500; Max-Age=10; HttpOnly\r\n";
            // "Content-Type: " + storage->getRequestData().getResponseContentType() + "\n";	// jaka
            std::cout << "_httpStatus: [[" << GRN_BG << _httpStatus << RES << "]]\n";
            break;
        }
	}
	return (temp);
}


// ---------------------------------------------------------------------------------------------------------------------------------------


void Request::appendToRequest(const char str[], size_t len) {
	std::cout << PUR << "Start appendToRequest(): _hasBody: " << _hasBody << " | _doneParsing: " << _doneParsing << " \n" << RES;

	// sleep(3); // testing TIMER
	std::string 			chunk = std::string(str);
	std::string				strToFind = "\r\n\r\n";
	std::string::size_type	it, it2;
	std::string				tmpHeader;

	//std::cout << GRN << "Request Chunk: " << RES << str << std::endl;
	if (_headerDone == false) {
		//std::cout << PUR << "     _headerDone == FALSE\n" << RES;
		_data.setTemp(_data.getTemp() + chunk);

		if ((it = _data.getTemp().find(strToFind)) != std::string::npos) {
			parseHeaderAndPath(tmpHeader, it);

			// in case of error shouldn't it directly return here? 
			// if (_doneParsing == true)
			//	return  ;

			std::cout << YEL << "Found header ending /r/n, maybe there is body\n" << RES;
			//std::cout << PUR << "size_type 'it' value: " << it << "\n" << RES;
			it2 = chunk.find(strToFind) + strToFind.length();	// needed to find the start of body, as char*, not std::string, because body will be vector
            if (it2 != std::string::npos) {
			    std::cout << "print start of body [" << PUR << str + it2 << "]\n" << RES;
            }

			//appendLastChunkToBody(it + strToFind.length());
			appendLastChunkToBody2(str + it2, len - it2); // jaka, changed to vector
			return ;
		}
	}
	if (_hasBody == true && _doneParsing == false) {
		//std::cout << "AppenToBody chunk [" << chunk << "]\n";
		appendToBody(str, len);	// changed to char*, because it needs to become a vector
	}
}

// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody2(const char *str, size_t len) {
	//std::cout << GRN << "start appendlastchunktobody()\n" << RES;
	//std::cout << YEL "FIRST BODY CHUNK, CLIENT BYTES SO FAR: " << _data.getClientBytesSoFar() << "\n" << RES;
	_data.setClientBytesSoFar(len);
	//std::cout << YEL "FIRST BODY CHUNK, CLIENT BYTES SO FAR: " << _data.getClientBytesSoFar() << "\n" << RES;

	std::vector<uint8_t> tempVec(str, str + len); // convert adn assign str to vector
	_data.setBody(tempVec);
	//std::cout << YEL "Body:\n" RES;
	//std::copy(tempVec.begin(), tempVec.end(), std::ostream_iterator<uint8_t>(std::cout));  // just to print

	if (_data.getClientBytesSoFar() > _data.getRequestContentLength()) {   // Compare body length
		std::cout << RED << "Error: Body-Length, first chunk (" << _data.getClientBytesSoFar() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;
        _httpStatus = I_AM_A_TEAPOT;
		return (1);
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
		//std::cout << YEL "Body:\n" RES;
		//std::copy(_data.getBody().begin(), _data.getBody().end(), std::ostream_iterator<uint8_t>(std::cout));  // just to print
		// std::cout << CYN << "    Body first chunk [" << _data.getBody() << "]\n" RES;// sleep(2);
		return (0);
	}
	return (0);
}

int Request::appendToBody(const char* str, size_t len) {
	// std::cout << RED << "start appendToBOdy(), current len, " << _data.getBody().length() << " expected len: " << _data.getRequestContentLength() << "\n" RES;
	//std::cout << RED << "    body before append: [" << _data.getBody() << "\n" << RES;
	std::vector<uint8_t> newChunk(str, str + len); // convert adn assign str to vector
	_data.setClientBytesSoFar(len);
	std::cout << YEL "CLIENT BYTES SO FAR: " << _data.getClientBytesSoFar() << "\n" << RES;

	std::vector<uint8_t> & tmp = _data.getBody();
	tmp.reserve(_data.getRequestContentLength() + len);
	tmp.insert(tmp.end(), newChunk.begin(), newChunk.end());
	_data.setBody(tmp);
	//std::cout << CYN << "    body after append:  [" << _data.getBody() << "]\n" << RES;
	
//	if (_data.getBody().size()      > _data.getRequestContentLength()) {		// Compare body lenght
	if (_data.getClientBytesSoFar() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED << "Error: Body-Length (" << _data.getClientBytesSoFar() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;// sleep(2);
		//std::cout << YEL "Body:\n" RES;
		//std::copy(_data.getBody().begin(), _data.getBody().end(), std::ostream_iterator<uint8_t>(std::cout));  // just to print
        _httpStatus = I_AM_A_TEAPOT;
		return (1);
	}
//	else if (_data.getBody().size() == _data.getRequestContentLength()) {
	else if (_data.getClientBytesSoFar() == _data.getRequestContentLength()) {
		std::cout << GRN << "OK: Done parsing.\n" RES;
		_doneParsing = true;
		//std::cout << "HEADER: [" BLU << _header << RES "]\n";	// sleep(1);
		//std::cout << "BODY:   [" BLU << _data.getBody()   << RES "]\n\n";	// sleep(1);
		return (0);
	}
	//std::cout << RED << "End appendToBody()\n" << RES;
	//std::cout << RED << "END appendToBOdy(), current len, " << _data.getClientBytesSoFar() << " expected len: " << _data.getRequestContentLength() << "\n" RES;
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
	std::cout << "Host:           [" << reqData.getRequestHost() << "]\n";
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

ServerData & Request::getServerData(){   // do we need reference? 
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
