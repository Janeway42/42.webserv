
// c++ kqueue.cpp Server.cpp srcs/Parser.cpp srcs/RequestData.cpp srcs/RequestParser.cpp && ./a.out


// CALLING CURL 
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d '{"Id": 79, "status": 3}'
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d 'abc'
// curl -X POST localhost:8080  -H "Content-Length: 444"  -H "Content-Type: text/html" -d 'abc'

#include <unistd.h> // sleep

#include "../includes/RequestParser.hpp"

namespace data {

/** Default constructor */
Request::Request() {
	_data = RequestData();
	_headerDone = false;

	_doneParsing = false;
	_errorRequest = false;
	_earlyClose = false;
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


std::string const & Request::getRequestBody() const {
	return _body;
}


/** ########################################################################## */


void storeBody(std::istringstream &iss)
{
	std::string lineContent;
	std::string body;

	while (std::getline(iss, lineContent)) {
		body.append(lineContent);
	}
	std::cout << YEL "body [" << body << "]\n" RES;
}


void Request::parseHeader(std::string header) {
	
	std::string lineContent;
	std::string body;
	int i = 0;

	std::istringstream iss(header);

	while (std::getline(iss, lineContent)) {
		if (i == 0) {								// FIRST LINE HEADER
			storeWordsFromFirstLine(lineContent);
		}
		else if (i > 0 && lineContent != "\r") {	// OTHER HEADER LINES
			storeWordsFromOtherLine(lineContent);
		}
													// START READING BODY
		else if (i > 0 && lineContent == "\r") {	// Not sure if this \r is 100% good
			storeBody(iss);	
		//	std::cout << YEL << "Found end of header block, begin of Body\n" RES;
			break ; 
		}
		i++;
	}
}



// !!!!!!! need to remove the file and links
int Request::storeWordsFromFirstLine(std::string firstLine)
{
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
			if (*iter == "GET" || *iter == "POST" || *iter == "DELETE")
				_data.setRequestMethod(*iter);
			else
				std::cout << RED << "Error: This method is not recognized\n" << RES;
		}
		else if (i == 1)
			_data.setRequestPath(*iter);
		else if (i == 2) {
			if (*iter != "HTTP/1.1" && *iter != "HTTP/1.0")		// maybe also HTTP/1.0 needed ??
				std::cout << RED << "Error: wrong http version\n" << RES;
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
		//std::cout << "[" << wordd << "] ";
		arr.push_back(wordd);
	} 	//std::cout << "\n";

	std::vector<std::string>::iterator iter = arr.begin();

	for (int i = 0; iter < arr.end(); iter++, i++)
	{
		if (i == 0) {
			// Maybe all *iters must change tolower(), because key is case insensitive			
			if (*iter == "Accept:") {
				iter++;
				_data.setRequestAccept(*iter);
				// std::cout << GRE "[" << *iter << "]\n";
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

void    Request::appendToRequest(const char *str) {
	std::string 			chunk = std::string(str);
	// std::string				strToFind = "image/avif";
	std::string				strToFind = "\r\n\r\n";
	//std::string				strToFind = "xxx";
	std::string::size_type	it;

	std::cout << PUR "AppendToRequest()\n" RES; // sleep(1);

	// _errorRequest = true;  // --------------------------------------------------error test
	// return ;


	if (_headerDone == false) {
		std::cout << PUR "     _headerDone == FALSE\n" RES;
		
		_temp.append(chunk);
		std::cout << PUR "     chunk appended to _temp\n" RES;	// sleep(1);

		if ((it = _temp.find(strToFind)) != std::string::npos) {
			std::cout << PUR "     a)  Found header ending /r/n, maybe there is body\n" RES;	// sleep(1);
			_header.append(_temp.substr(0, it));
			_headerDone = true;
			std::cout << "HEADER: [" BLU << _header << RES "]\n";	// sleep(1);
			parseHeader(_header);
			if (_data.getRequestContentLength() == 0){
				_doneParsing = true;
				return ;
			}
			appendLastChunkToBody(it + strToFind.length());
			std::cout << "BODY:   [" BLU << _body   << RES "]\n\n";	// sleep(1);
		}

		// else if ((it = _temp.find(strToFind)) == std::string::npos ) { // Keep appending, till /r/n id found
		// 	// std::cout << PUR "     b)  Found header ending, no body\n" RES;
		// 	std::cout << PUR "     b)  Not found /r/n yet. Append _temp_ to _header." RES;
		// 	std::cout << PUR " (it: " << it << ")\n" RES;	// sleep(1);
		// 	std::cout << "     _temp: [" BLU << _temp << RES "]\n";	// sleep(1);
		// }
	}
	else if (_headerDone == true) {
			std::cout << PUR "     _headerDone == TRUE\n" RES;	// sleep(1);
			appendToBody(chunk);
	}
	std::cout << PUR "End of AppendToRequest()\n" RES; // sleep(1);
}




// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody(std::string::size_type it) {
	_body = _temp.substr(it);
	
	// Compare body lenght
	if (_body.length() > _data.getRequestContentLength()) {
		std::cout << RED "Error: Body-Length is bigger than expected Content-Length\n" RES;
		_errorRequest = true;
		return (1);
	}

	if (_body.length() == _data.getRequestContentLength()) {
		if (_body.length() == 0 && _data.getRequestContentLength() == 0) {		// Compare body lenght
			std::cout << GRE "OK (there is no body)\n" RES;
			_doneParsing = true;
			return (0);
		}
		std::cout << GRE "OK: Body-Length is as expected Content-Length\n" RES;
		_doneParsing = true;
		return (0);
	}
	// Timeout ???
	// 		In case of wrong header, body length bigger than body, but body is already done, 

	return (0);
}



int Request::appendToBody(std::string req) {
	std::cout << GRE "AppendToBody()\n" RES;
	//exit (0);

	_body.append(req);
	
	if (_body.length() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED "Error: Body-Length is bigger than expected Content-Length\n" RES; // sleep(2);
		_errorRequest = true;
		return (1);
	}
	else if (_body.length() == _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << GRE "OK: Done parsing.\n" RES; // sleep(2);
		_doneParsing = true;
		std::cout << "HEADER: [" BLU << _header << RES "]\n";	// sleep(1);
		std::cout << "BODY:   [" BLU << _body   << RES "]\n\n";	// sleep(1);
		return (0);
	}
	// // Maybe not needed
	// else if (_body.length() == 0 && _data.getRequestContentLength() == 0) {		// Compare body lenght
	// 	std::cout << GRE "There is no body\n" RES;
	// 	_doneParsing = true;
	// 	return (0);
	// }
	// // time out etc etc etc
	return (0);
}


int Request::checkStoredVars() {
	if (_data.getRequestContentLength() == _body.length()) {
		// if (_body.length() == 0 && _data.getRequestContentLength() == 0) {		// Compare body lenght
		// 	std::cout << GRE "OK (there is no body)\n" RES;
		// 	_doneParsing = true;
		// 	return (0);
		// }
		std::cout << GRE "OK: Body size == Content lenght\n" RES;
	}
	if (_data.getRequestContentLength() > _body.length()) {
		std::cout << RED "Error: Body length is smaller than expected Content-Length\n" RES;
		return (1);
	}
	return (0);
}



// JUST FOR CHECKING
void Request::printStoredRequestData(data::Request &request) {
	data::RequestData reqData = request.getRequestData();

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
	std::cout << "REQUEST BODY:\n[" PUR << request.getRequestBody() << RES "]\n";
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

std::string Request::getTemp()
{
	return(_temp);
}

std::time_t Request::getTime()
{
	return(_startTime);
}








} // data

