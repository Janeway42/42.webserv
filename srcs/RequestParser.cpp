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

/** Default constructor */
Request::Request() {
	_data = RequestData();
	_headerDone = false;
	_doneParsing = false;
	_errorRequest = false;
	_earlyClose = false;
	_hasBody = false;
	_startTime = std::time(NULL);

}

/** Destructor */
Request::~Request() {
//   delete _data;
}


/** Getters */
RequestData const & Request::getRequestData() const {
	return _data;
}



/** METHODS ################################################################# */

void Request::parseHeader(std::string header) {

//	std::string body;
	std::string lineContent;
	int i = 0;

	std::istringstream iss(header);

	while (std::getline(iss, lineContent)) {
		if (i == 0) {								// FIRST LINE HEADER
			storeWordsFromFirstLine(lineContent);
		} else if (i > 0 && lineContent != "\r") {	// OTHER HEADER LINES
			storeWordsFromOtherLine(lineContent);
			// Store key:value pairs into vector or map
		} else if (i > 0 && lineContent == "\r") {	// Not sure if this \r is 100% good
			// START READING BODY
			//	std::cout << YEL << "Found end of header block, begin of Body\n" << RES;
			storeBody(iss);
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
				std::cout << RED_BG << YEL << "JOYCE REQUEST METHOD: " << *iter << RES << std::endl;
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
				_errorRequest = true;
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


void Request::parseHeaderAndPath(std::string & tmpHeader, int fdClient, std::string::size_type it) {
	(void)fdClient; // maybe will be needed
	_hasBody = true;
	tmpHeader = _data.getHeader();
	tmpHeader.append(_data.getTemp().substr(0, it));
	_data.setHeader(tmpHeader);
	_headerDone = true;
	//std::cout << "HEADER: [" << BLU << _header << RES "]\n";	// sleep(1);
	parseHeader(_data.getHeader());
	parsePath(_data.getHttpPath());

	if (_data.getRequestContentLength() == 0){
		std::cout << PUR << "     DONE PARSING\n" << RES;	// sleep(1);
		if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "")
			; //callCGI(getRequestData(), fdClient);
		_doneParsing = true;
	}
}



void	Request::chooseMethod_StartAction() {
	if (_doneParsing == true && _data.getRequestMethod() == "POST")
			; // callCGI(getRequestData(), fdClient);
	if (_doneParsing == true && _data.getRequestMethod() == "DELETE") {
		std::cout << GRN_BG << "DELETE METHOD" << RES << std::endl;
		/* DELETE deletes a resource (specified in URI) */
		if (pathType(_data.getPath()) != REG_FILE) {
			std::cout << RED_BG << "ERROR 404 Not Found" << RES << std::endl;
			// status error 404 Not Found -> Server cannot find the requested resource.
		}
		// cgi or just delete the file literally? It's not with CGI: Your program should call the CGI with the file requested as first argument.
		// How to delete a file froma  direcory: https://codescracker.com/cpp/program/cpp-program-delete-file.htm#:~:text=To%20delete%20any%20file%20from,used%20to%20delete%20a%20file.
		if (remove(_data.getPath().c_str()) != 0) {
			std::cout << RED_BG << "ERROR 204 No Content" << RES << std::endl;
			// 500 Internal Server Error -> Server encountered an unexpected condition that prevented it from fulfilling the request.
		}
			/* On successful deletion, it returns HTTP response status code 204 (No Content) */
		// _doneParsing = true;    // 
	}
}



void    Request::appendToRequest(const char *str, int fdClient) {
	std::string 			chunk = std::string(str);
	std::string				strToFind = "\r\n\r\n";
	std::string::size_type	it;
	std::string				tmpHeader;

	_hasBody = false;
	std::cout << RED_BG << "JOYCE header chunk: " << str << RES << std::endl;
	if (_headerDone == false) {
		std::cout << PUR << "     _headerDone == FALSE\n" << RES;
		_data.setTemp(_data.getTemp() + chunk);

		if ((it = _data.getTemp().find(strToFind)) != std::string::npos) {
			parseHeaderAndPath(tmpHeader, fdClient, it);
			std::cout << PUR << "Found header ending /r/n, maybe there is body\n" << RES;
			appendLastChunkToBody(it + strToFind.length(), fdClient);
		}
	}
	if (_headerDone == true) {
		std::cout << PUR << "     _headerDone == TRUE\n" << RES;
		if (_hasBody == true) {
			appendToBody(chunk);
		}
		if (_doneParsing == true)	
			chooseMethod_StartAction();
	}
}



// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody(std::string::size_type it, int fdClient) {
	_data.setBody(_data.getTemp().substr(it));
	if (_data.getBody().length() > _data.getRequestContentLength()) {   // Compare body length
		std::cout << RED << "Error: Body-Length (" << _data.getBody().length() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;
		_errorRequest = true;
		return (1);
	}
	if (_data.getBody().length() == _data.getRequestContentLength()) {
		if (_data.getBody().length() == 0 && _data.getRequestContentLength() == 0) {    // Compare body lenght
			std::cout << GRE << "OK (there is no body)\n" << RES;
			_doneParsing = true;
			_hasBody = false;
			return (0);
		}
		std::cout << GRE << "OK: Body-Length is as expected Content-Length\n" << RES;
		_doneParsing = true;
		if (_doneParsing == true && _data.getRequestMethod() == "POST") { // can delete doneParsing == true
			std::cout << "      doneparsing true and POST true, call CGI\n";
			std::cout << "      _body: [" << _data.getBody() << "]\n"; 
			parsePath(_data.getHttpPath());	// Jaka: Probably not needed here?? Already done in parseHeaderAndpath
			std::cout << "      getQueryString: [" << getRequestData().getQueryString() << "]\n"; 
			callCGI(getRequestData(), fdClient);
		}
		return (0);
	}
	// Timeout ???
	// 		In case of wrong header, body length bigger than body, but body is already done, 
	return (0);
}



int Request::appendToBody(std::string req) {
	std::string tmp = _data.getBody();
	tmp.append(req);
	_data.setBody(tmp);
	
	if (_data.getBody().length() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED << "Error: Body-Length (" << _data.getBody().length() << ") is bigger than expected Content-Length (" << _data.getRequestContentLength() << ")\n" << RES;// sleep(2);
		_errorRequest = true;
		return (1);
	}
	else if (_data.getBody().length() == _data.getRequestContentLength()) {
		std::cout << GRE "OK: Done parsing.\n" RES;

		if (_data.getRequestMethod() == "POST") {
			std::cout << GRE "      ....    store _body into _queryString\n" RES;
			_data.setQueryString(_data.getBody());
			storeFormData(_data.getBody());	// maybe not needed
		}
		_doneParsing = true;
		//std::cout << "HEADER: [" BLU << _header << RES "]\n";	// sleep(1);
		//std::cout << "BODY:   [" BLU << _body   << RES "]\n\n";	// sleep(1);
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
	std::cout << "Host:           [" << reqData.getRequestHost() << "]\n";
	std::cout << "Accept:         [" << reqData.getRequestAccept() << "]\n";
	std::cout << "Content-Length: [" << reqData.getRequestContentLength() << "]\n";
	std::cout << "Content-Type:   [" << reqData.getRequestContentType() << "]\n" RES;
	// PRINT BODY
//	std::cout << "REQUEST BODY:\n[" PUR << request.getRequestBody() << RES "]\n";
}


bool Request::getDone()
{
	return (_doneParsing);
}

void Request::setDone(bool val)
{
	_doneParsing = val;
}

bool Request::getError()
{
	return (_errorRequest);
}

void Request::setError(bool val)
{
	_errorRequest = val;
}

bool Request::getEarlyClose()
{
	return (_earlyClose);
}

void Request::setEarlyClose(bool val)
{
	_earlyClose = val;
}

std::time_t Request::getTime()
{
	return(_startTime);
}
