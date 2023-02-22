#include <unistd.h> // sleep

#include "RequestParser.hpp"

// c++ kqueue.cpp Server.cpp srcs/Parser.cpp srcs/RequestData.cpp srcs/RequestParser.cpp && ./a.out

// CALLING CURL 
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d '{"Id": 79, "status": 3}'
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d 'abc'
// curl -X POST localhost:8080  -H "Content-Length: 444"  -H "Content-Type: text/html" -d 'abc'

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
RequestData Request::getRequestData() const {
	return _data;
}

std::string Request::getRequestBody() const {
	return _body;
}

/** METHODS ################################################################# */

void Request::storeBody(std::istringstream &iss)
{
	std::string lineContent;
//	std::string body;

	while (std::getline(iss, lineContent)) {
		_body.append(lineContent);
	}
	std::cout << YEL "body [" << _body << "]\n" RES;

	// IF METHOD == POST AND IF Content-Type 	application/x-www-form-urlencoded
	//	Store key:value pars in map<>
	// storeFormData(_body); // here too early
}

void Request::parseHeader(std::string header) {
	
//	std::string body;
	std::string lineContent;
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
		//	std::cout << YEL << "Found end of header block, begin of Body\n" RES;
			storeBody(iss);	
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
			{
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

/* 	Split string at '&' and store each line into vector<>
	Then split each line in vector into map<> key:value */
std::map<std::string, std::string> Request::storeFormData(std::string pathForm)
{
    //std::cout << GRE << "Start store form data()\n" << RES;
    std::cout << GRE << "   BODY:        [" << _body << "]\n" << RES;
    std::cout << GRE << "   FORM PATH:   [" << pathForm << "]\n" << RES;

    std::string					line;
    std::vector<std::string>	formList;

    std::stringstream iss(pathForm);
    while (std::getline(iss, line, '&'))
        formList.push_back(line);

    std::string							key, val;
    std::map<std::string, std::string>	formDataMap;
    std::vector<std::string>::iterator	it;

    for (it = formList.begin(); it != formList.end(); it++) {
        std::stringstream inss(*it);
        std::getline(inss, key, '=') >> val;
        formDataMap[key] = val;
    }
    _data.setFormData(formDataMap);
    return (formDataMap);
}

// Found GET Method with '?' Form Data
void	Request::storePathParts_and_FormData(std::string path) {

    int temp				= path.find_first_of("?");
    std::string tempStr		= path.substr(0, temp);
    int posLastSlash 		= tempStr.find_last_of("/");
    int	posFirstQuestMark	= path.find_first_of("?");
    std::string	pathForm	= path.substr(temp, std::string::npos);

    _data.setPathFirstPart(tempStr.substr(0, posLastSlash));
    _data.setPathLastWord(path.substr(posLastSlash, posFirstQuestMark - posLastSlash));

    if (pathForm[0] == '?') 	// Skip the '?' in the path
        pathForm = &pathForm[1];
    storeFormData(pathForm);
}

// Last word in path must be a folder (last '/' found)
// The 2nd and 3rd args not needed anymore
// void	Request::storePath_and_FolderName(std::string path, std::string pathFirstPart, std::string pathLastWord, RequestData reqData) {
void	Request::storePath_and_FolderName(std::string path) {

    int 	pos1	= 0;
    int		pos2	= 0;
    size_t 	count	= 0;
    pos2 			= path.find_first_of("/");

    while (count < path.length()) {
        if ((count = path.find("/", count)) != std::string::npos) {
            pos1 = pos2;
            pos2 = count;
        }
        if ( count == std::string::npos )
            break ;
        count++;
    }
    //	pathFirstPart	= path.substr(0, pos1 + 1);
    //	pathLastWord	= path.substr(pos1 + 1, pos2);

    _data.setPathFirstPart(path.substr(0, pos1 + 1));
    _data.setPathLastWord(path.substr(pos1 + 1, pos2));
    //	reqData.setPathLastWord(path.substr(pos1 + 1, pos2));
}

int Request::parsePath(std::string str) {
    // maybe also trim white spaces front and back
//	Request		req;
    std::string path			= removeDuplicateSlash(str);
    size_t		ret				= 0;
    std::string pathLastWord	= "";

    if (path == "")
        return (-1);
    else if (path == "/") {
        std::cout << GRE << "The path has no GET-Form data. Path is the root '/'\n" << RES;
    }
    else if (path.back() == '/'  && (path.find("?") == std::string::npos)) {
        std::cout << GRE << "The path has no GET-Form data. Last char is '/', it must be a folder.\n" << RES;
        storePath_and_FolderName(path);
        printPathParts(str, path, "", "", getRequestData());
    }

        // if the last char is not slash /   then look for question mark
    else if ((ret = path.find("?")) == std::string::npos ) {
        std::cout << GRE << "There is no Form data, the '?' not found\n" << RES;
        int pos			= 0;
        pos				= path.find_last_of("/");

        _data.setPathFirstPart(path.substr(0, pos));
        _data.setPathLastWord(path.substr(pos, std::string::npos));
        printPathParts(str, path, "", "", getRequestData());
    }

    else if ((ret = path.find("?")) != std::string::npos) {			// Found '?' in the path
        std::cout << GRE << "There is GET Form data, the '?' is found\n" << RES;
        storePathParts_and_FormData(path);
        printPathParts(str, path, "", "", getRequestData());
    }

    checkIfFileExists(path);	// What in case of root only "/"  ???
    checkTypeOfFile(path);
    //checkTypeOfFile(_data.getPathLastWord());
    //std::cout << RED << "Last word " << _data.getPathLastWord() << RES << "\n";
    return (0);
}

/*	- What if method is GET (normaly without body) AND content-length is not zero ???
	- What if method POST and content-length is zero ???
*/

void    Request::appendToRequest(const char *str) {
	std::string 			chunk = std::string(str);
	std::string				strToFind = "\r\n\r\n";
	std::string::size_type	it;

	//std::cout << PUR "AppendToRequest()\n" RES; // sleep(1);

	//test error: 
	// _errorRequest = true;  // --------------------------------------------------error test
	// return ;

	//test request takes too long: 
	// sleep(3);

	if (_headerDone == false) {
		//std::cout << PUR "     _headerDone == FALSE\n" RES;
		
		_temp.append(chunk);
		//std::cout << PUR "     chunk appended to _temp\n" RES;	// sleep(1);

		if ((it = _temp.find(strToFind)) != std::string::npos) {
			//std::cout << PUR "     a)  Found header ending /r/n, maybe there is body\n" RES;	// sleep(1);
			_header.append(_temp.substr(0, it));
			_headerDone = true;
			std::cout << "HEADER: [" BLU << _header << RES "]\n";	// sleep(1);
			parseHeader(_header);

			parsePath(_data.getHttpPath());	// INSERTED JAKA, can maybe be moved to parseHeader()

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
	//std::cout << PUR "End of AppendToRequest()\n" RES; // sleep(1);
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

		if (_data.getRequestMethod() == "POST")
			storeFormData(_body);

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
