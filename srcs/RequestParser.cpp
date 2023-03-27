// c++ kqueue.cpp Server.cpp srcs/Parser.cpp srcs/RequestData.cpp srcs/RequestParser.cpp && ./a.out

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

#include <unistd.h> // sleep
#include "../includes/RequestParser.hpp"


/*
	In case error 404, file not found, it's probably good to not continue after parsePath(), 
	and just close the connection

*/

/** Default constructor */
Request::Request() {
    _clientFd = -1;
    _server = ServerData();
    _data = RequestData();
    _answer = ResponseData();
    _cgi = CgiData();

    _headerDone = false;
    _doneParsing = false;
    _httpStatus = NO_STATUS;
    _hasBody = false;
}

/** Overloaded constructor */
Request::Request(int fd, ServerData *specificServer) {
	_clientFd = fd;
	_server = *specificServer;
	_data = RequestData();
	_answer = ResponseData();
	_cgi = CgiData();

	_headerDone = false;
	_doneParsing = false;
    _httpStatus = NO_STATUS;
	_hasBody = false;
}

/** Destructor */
Request::~Request() {
//   delete _data;
}


/** Getters */
RequestData & Request::getRequestData(){
	return _data;
}

ServerData & Request::getServerData(){
	return _server;
}

ResponseData & Request::getResponseData(){
	return (_answer);
}

CgiData & Request::getCgiData(){
	return (_cgi);
}



/** METHODS ################################################################# */

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
		} else if (i > 0 && lineContent == "\r") {	// Not sure if this \r is 100% good
			// START READING BODY
			//	std::cout << YEL << "Found end of header block, begin of Body\n" << RES;
			storeBody(is);
			break ;
		}
		i++;
	}
}

void Request::storeBody(std::istringstream &iss)
{
	std::string lineContent;
	std::string tmp;

	while (std::getline(iss, lineContent)) {
		tmp = _data.getBody();
		tmp.append(lineContent);
		_data.setBody(tmp);
		// _body.append(lineContent);
	}
	std::cout << YEL "body [" << _data.getBody() << "]\n" << RES;

}


// MAYBE WON'T BE NEEDED
void	storeBodyAsFile(std::string body) {
	std::ofstream bodyFile("./resources/cgi/bodyFile.txt");
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
				std::cout << GRN_BG << YEL << "REQUEST METHOD: " << *iter << RES << std::endl;
				_data.setRequestMethod(*iter);
			} else {
				std::cout << RED << "Error: This method is not recognized\n" << RES;
				// TODO -> Set 405 Method Not Allowed
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
				_data.setRequestHost(*iter);
			}
			else if (*iter == "Content-Length:") {
				iter++;
				_data.setRequestContentLength(*iter);
			}
			else if (*iter == "Content-Type:") {
				iter++;
				_data.setRequestContentType(*iter);
			}
			// else
			// 	std::cout << RED << "Error: This method is not recognized - other line\n" << RES;
		}
	}
	return (0);
}

/*	- What if method is GET (normaly without body) AND content-length is not zero ???
	- What if method POST and content-length is zero ???
*/


// void Request::parseHeaderAndPath(std::string & tmpHeader, int fdClient, std::string::size_type it) {
void Request::parseHeaderAndPath(std::string & tmpHeader, struct kevent event, std::string::size_type it) {
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Start parsing ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
	_hasBody = true;
	tmpHeader = _data.getHeader();
	tmpHeader.append(_data.getTemp().substr(0, it));
	_data.setHeader(tmpHeader);
	_headerDone = true;
	//std::cout << "HEADER: [" << BLU << _header << RES "]\n";	// sleep(1);
	parseHeader(_data.getHeader());
	std::cout << RED "server root path: " << getServerData().getRootDirectory() << "\n" RES;
	parsePath(_data.getHttpPath(), event);	// IF FILE NOT FOUND 404, IT COULD JUST CLOSE THE CONNECTION AND STOP

	if (_data.getRequestContentLength() == 0){
		// if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "")
		//	; //callCGI(getRequestData(), fdClient); 		// moved to chooseMethod_StartAction()
		_doneParsing = true;
    }
}


// Jaka: moved to the WebServer
// void	Request::chooseMethod_StartAction(struct kevent event) {
// 	std::cout << RED "Start ChooseMethodStartAction()\n" RES ;
// 	if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "") {
// 		std::cout << RED "     start GET, callCGI\n" RES ;
// 		// callCGI(getRequestData(), fdClient);
// 		callCGI(event);
// 	}
// 	if (_data.getRequestMethod() == "POST")
// 			callCGI(event);
// 	if (_data.getRequestMethod() == "DELETE") {
// 		std::cout << GRN_BG << "DELETE METHOD" << RES << std::endl;
// 		/* DELETE deletes a resource (specified in URI) */
// 		if (pathType(_data.getPath()) != REG_FILE) {
// 			std::cout << RED_BG << "ERROR 404 Not Found" << RES << std::endl;
// 			// status error 404 Not Found -> Server cannot find the requested resource.
// 		}
// 		// cgi or just delete the file literally? It's not with CGI: Your program should call the CGI with the file requested as first argument.
// 		// How to delete a file froma  direcory: https://codescracker.com/cpp/program/cpp-program-delete-file.htm#:~:text=To%20delete%20any%20file%20from,used%20to%20delete%20a%20file.
// 		if (remove(_data.getPath().c_str()) != 0) {
// 			std::cout << RED_BG << "ERROR 204 No Content" << RES << std::endl;
// 			// 500 Internal Server Error -> Server encountered an unexpected condition that prevented it from fulfilling the request.
// 		}
// 			/* On successful deletion, it returns HTTP response status code 204 (No Content) */
// 		// _doneParsing = true;    // 
// 	}
// }



// void    Request::appendToRequest(const char *str, int fdClient) {
void    Request::appendToRequest(const char *str, struct kevent event) {
	std::cout << PUR << "Start appendToRequest(): _hasBody "<< _hasBody << " _doneParsing " << _doneParsing << " \n" << RES;

	std::string 			chunk = std::string(str);
	std::string				strToFind = "\r\n\r\n";
	std::string::size_type	it;
	std::string				tmpHeader;

	//std::cout << GRE << "Request Chunk: " << RES << str << std::endl;
	if (_headerDone == false) {
		//std::cout << PUR << "     _headerDone == FALSE\n" << RES;
		_data.setTemp(_data.getTemp() + chunk);

		if ((it = _data.getTemp().find(strToFind)) != std::string::npos) {
			parseHeaderAndPath(tmpHeader, event, it);
			std::cout << PUR << "Found header ending /r/n, maybe there is body\n" << RES;
			appendLastChunkToBody(it + strToFind.length());
		//	if (_doneParsing == true)
		//		chooseMethod_StartAction(event);
			return ;
		}
	}
	std::cout << PUR << "     _headerDone == TRUE\n" << RES;
	if (_hasBody == true && _doneParsing == false) {
		appendToBody(chunk);
	}
//	if (_doneParsing == true) {
//		chooseMethod_StartAction(event);
//	}
}



// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody(std::string::size_type it) {
	std::cout << GRN << "start appendlastchunktobody()\n" << RES;
	_data.setBody(_data.getTemp().substr(it));
	std::cout << "    Body [" GRN << _data.getBody() << "]\n" RES;

	if (_data.getBody().length() > _data.getRequestContentLength()) {   // Compare body length
		std::cout << RED << "Error: Body-Length, first chunk (" << _data.getBody().length() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;
        _httpStatus = I_AM_A_TEAPOT;
		return (1);
	}
	if (_data.getBody().length() == _data.getRequestContentLength()) {
		std::cout << GRN << "    _doneparsing == true\n" << RES;

		_doneParsing = true;
		if (_data.getBody().length() == 0 && _data.getRequestContentLength() == 0) {    // Compare body lenght
			std::cout << GRE << "OK (there is no body)\n" << RES;
			_hasBody = false;
			std::cout << RED "content type: [" << _data.getRequestContentType() << "]\n" RES;
			return (0);
		}
		std::cout << GRE << "OK: Body-Length is as expected Content-Length\n" << RES;
		_doneParsing = true; // otherwise it went to appendToBody, and appended more stuff, so the body lenght became larger then expected
		std::cout << CYN << "    Body first chunk [" << _data.getBody() << "]\n" RES;// sleep(2);
		return (0);
	}
	// Timeout ???
	// 		In case of wrong header, body length bigger than body, but body is already done, 
	std::cout << CYN << "    Body first chunk [" << _data.getBody() << "]\n" RES;

	return (0);
}



int Request::appendToBody(std::string req) {
	std::cout << RED << "start appendToBOdy(), req [" << req << "\n" << RES;
	//std::cout << RED << "    body before append: [" << _data.getBody() << "\n" << RES;
	std::string tmp = _data.getBody();
	tmp.append(req);
	_data.setBody(tmp);
	//std::cout << CYN << "    body after append:  [" << _data.getBody() << "]\n" << RES;
	
	if (_data.getBody().length() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED << "Error: Body-Length (" << _data.getBody().length() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;// sleep(2);
		std::cout << CYN << "       Body [" << _data.getBody() << "]\n" RES;
        _httpStatus = I_AM_A_TEAPOT;
		return (1);
	}
	else if (_data.getBody().length() == _data.getRequestContentLength()) {
		std::cout << GRE "OK: Done parsing.\n" RES;

		if (_data.getRequestMethod() == "POST") {
			//std::cout << GRE "      ....    store _body into _queryString\n" RES;
			_data.setQueryString(_data.getBody());					// querystring not needed, just to print it
			getResponseData().setResponseBody(_data.getBody());		// _responseBody to be sent to CGI
			storeFormData(_data.getBody());	// maybe not needed
		}
		_doneParsing = true;
		//std::cout << "HEADER: [" BLU << _header << RES "]\n";	// sleep(1);
		std::cout << "BODY:   [" BLU << _data.getBody()   << RES "]\n\n";	// sleep(1);
		return (0);
	}
	//std::cout << RED << "End appendToBody()\n" << RES;
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
	std::cout << "Content-Type:   [" << reqData.getRequestContentType() << "]\n" RES;
	// PRINT BODY
//	std::cout << "REQUEST BODY:\n[" PUR << request.getRequestBody() << RES "]\n";
}


// getters 

bool Request::getDone()
{
	return (_doneParsing);
}

HttpStatus Request::getHttpStatus()
{
	return (_httpStatus);
}

int Request::getFdClient()
{
	return (_clientFd);
}

// setters

void Request::setDone(bool val)
{
	_doneParsing = val;
}

void Request::setHttpStatus(HttpStatus val)
{
    _httpStatus = val;
}
