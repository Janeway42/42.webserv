//
// Created by Joyce Macksuele on 18/01/2023 
// Addapted Jaka
//


#include "../includes/RequestParser.hpp"

namespace data {

/** Default constructor */
Request::Request() {
	_data = RequestData();
	_headerDone = false;

	_doneParsing = false;
	_errorRequest = false;
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



void Request::appendToRequest(const char *str) {
	std::string 			chunk = std::string(str);
	std::string::size_type 	it;

	std::cout << "ATR A)\n";
	if (_headerDone == false) {
		if (chunk.find("\r\n\r\n") == -1) {		// Sep. line not found
			_header.append(chunk);
			//_doneParsing = true;
			//std::cout << "done after append inside parser: " << _doneParsing << std::endl;
		}
		else if ((it = chunk.find("\r\n\r\n")) != std::string::npos) { // Found, but has more chars after it
			_temp.append(chunk);
			_header.append(chunk.substr(0, it));
			_headerDone = true;

			parseHeader(_header);
			appendLastChunkToBody(it);
			// std::cout << "apended header:::::::::::::::::" << _header << std::endl;
			_doneParsing = true;
			std::cout << "A done after append inside parser: " << _doneParsing << std::endl;

		} 
		else if ((it = chunk.find("\r\n\r\n")) == std::string::npos) { // Found, but has more chars after it
			_temp.append(chunk);
			_header.append(chunk.substr(0, it));
			_headerDone = true;

			parseHeader(_header);
			//appendLastChunkToBody(it);
			// std::cout << "apended header:::::::::::::::::" << _header << std::endl;
			_doneParsing = true;
			std::cout << "B done after append inside parser: " << _doneParsing << std::endl;

		}
		else {								// found, and it's the end of string
			it = _temp.find("\r\n\r\n");
			_header = _temp.substr(0, it);
			appendToBody(chunk);
			_headerDone = true;
		}
	}
	else if (_headerDone == true)
	{
		std::cout << "test\n";
		appendToBody(chunk);
	}
	std::cout << "HEADER [" << _header << "]\n";
}


// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody(std::string::size_type it) {
	_body = _temp.substr(it + 4);
	
	// Compare body lenght
	if (_body.length() > _data.getRequestContentLength()) {
		std::cout << RED "Error: Body-Length is bigger than expected Content-Length\n" RES;
		return (1);
	}
	// time out etc etc etc
	// set done 
	// st errors
	return (0);
}


int Request::appendToBody(std::string req) {
	std::cout << GRE "appendToBody\n" RES;
	exit (0);

	_body.append(req);
	
	if (_body.length() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED "Error: Body-Length is bigger than expected Content-Length\n" RES;
		_errorRequest = true;
		return (1);
	}
	else if (_body.length() == _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << GRE "Done parsing true\n" RES;
		_doneParsing = true;
		return (0);
	}
	else if (_body.length() == 0 && _data.getRequestContentLength() == 0) {		// Compare body lenght
		std::cout << GRE "There is no body\n" RES;
		_doneParsing = true;
		return (0);
	}
	// time out etc etc etc
	// set done 
	// st errors
	// std::cout << YEL "ATB, bodyLenght " << _body.length() << "]\n" RES;
	return (0);
}


int Request::checkStoredVars() {
	if (_data.getRequestContentLength() == _body.length()) {
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

std::string Request::getTemp()
{
	return(_temp);
}

} // data
