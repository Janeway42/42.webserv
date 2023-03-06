
// c++ kqueue.cpp Server.cpp srcs/Parser.cpp srcs/RequestData.cpp srcs/RequestParser.cpp && ./a.out


// CALLING CURL 
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d '{"Id": 79, "status": 3}'
// curl -X POST localhost:8080  -H "Content-Type: text/html" -d 'abc'
// curl -X POST localhost:8080  -H "Content-Length: 444"  -H "Content-Type: text/html" -d 'abc'

#include <unistd.h> // sleep

#include "../includes/RequestParser.hpp"

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



/** METHODS ################################################################# */

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
            // Store key:value pairs into vector or map
        }
            // START READING BODY
        else if (i > 0 && lineContent == "\r") {	// Not sure if this \r is 100% good
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


/* 	Split string at '&' and store each line into vector<>
	Then split each line in vector into map<> key:value */
// std::map<std::string, std::string> Request::storeFormData(std::string &queryString)
std::map<std::string, std::string> Request::storeFormData(std::string queryString)
{
    std::cout << GRN << "Start store form data()\n" << RES;
    //std::cout << GRN << "    BODY:        ["   << _body << "]\n" << RES;
    //std::cout << GRN << "    queryString:   [" <<  queryString << "]\n" << RES;
    //std::cout << GRN << "   _queryString:   [" << _data.getQueryString() << "]\n" << RES;

    std::string					line;
    std::vector<std::string>	formList;

    std::stringstream iss(queryString);
    while (std::getline(iss, line, '&'))
        formList.push_back(line);

    _data.setFormList(formList);

    // MAYBE THE FORM MAP WILL NOT BE NEEDED
    std::string							key, val;
    std::map<std::string, std::string>	formDataMap;
    std::vector<std::string>::iterator	it;

    for (it = formList.begin(); it != formList.end(); it++) {
        std::stringstream iss2(*it);								// maybe change name, or reuse the above variable
        std::getline(iss2, key, '=') >> val;
        formDataMap[key] = val;
        //std::cout << YEL << "  ... vector [" << *it << "]\n" << RES;
    }
    _data.setFormData(formDataMap);
    return (formDataMap);
}

// Found GET Method with '?' Form Data
void Request::storePathParts_and_FormData(std::string path) {

    int			temp		= path.find_first_of("?");
    std::string tempStr		= path.substr(0, temp);

    _data.setPath(path.substr(0, temp));
    //std::cout << CYN "StorePathParts() " << _data.getPath() << "\n" << RES;
    int posLastSlash 		= tempStr.find_last_of("/");
    int	posFirstQuestMark	= path.find_first_of("?");
    std::string	queryString	= path.substr(temp, std::string::npos);

    _data.setPathFirstPart(tempStr.substr(0, posLastSlash));
    _data.setPathLastWord(path.substr(posLastSlash, posFirstQuestMark - posLastSlash));

    if (queryString[0] == '?') 	// Skip the '?' in the path
        queryString = &queryString[1];

    if (_data.getRequestMethod() == "GET")
        _data.setQueryString(queryString);

    std::cout << "Stored GET _queryString [\n" << BLU << _data.getQueryString() << RES << "]\n";
    std::cout << "Stored GET _body [\n" << BLU << _data.getBody() << RES << "]\n" << RES;

    storeFormData(queryString);	// maybe not needed (the whole vector and map)
    // if the cgi script can handle the whole queryString
}


// !!! Not storing correctly the path part and file name!
// If last '/' is found in path, then this is a folder, not file
void Request::storePath_and_FolderName(std::string path) {

    size_t 	pos1	= 0;
    size_t	pos2	= 0;
    size_t 	count	= 0;

    std::cout << CYN "Start storePath_and_FolderName(}" << path << "\n" << RES;

    // Check if there is query '?' and store path before it
    // Probably not needed searching for query here, because the method is POST,  so query not possible
    _data.setPath(path);
    pos1 = path.find_first_of("?");
    if (pos1 != std::string::npos)
        _data.setPath(path.substr(0, pos1));

    pos1	= 0;
    pos2	= path.find_first_of("/");
//	pos2	= path.find_last_of("/");
    while (count < path.length()) {
        if ((count = path.find("/", count)) != std::string::npos) {
            pos1 = pos2;
            pos2 = count;
        }
        if ( count == std::string::npos )
            break ;
        count++;
    }
    _data.setPathFirstPart(path.substr(0, pos1 + 1));
    _data.setPathLastWord(path.substr(pos1 + 1, pos2));
}

/*
char* ENV[25] = {
	(char*)"COMSPEC=", (char*)"DOCUMENT_ROOT=", (char*)"GATEWAY_INTERFACE=", (char*)"HTTP_ACCEPT=", (char*)"HTTP_ACCEPT_ENCODING=",
	(char*)"HTTP_ACCEPT_LANGUAGE=", (char*)"HTTP_CONNECTION=", (char*)"HTTP_HOST=", (char*)"HTTP_USER_AGENT=", (char*)"PATH=",
	(char*)"QUERY_STRING=", (char*)"REMOTE_ADDR=", (char*)"REMOTE_PORT=", request_method2, (char*)"REQUEST_URI=", (char*)"SCRIPT_FILENAME=",
	(char*)"SCRIPT_NAME=", (char*)"SERVER_ADDR=", (char*)"SERVER_ADMIN=", (char*)"SERVER_NAME=",(char*)"SERVER_PORT=",(char*)"SERVER_PROTOCOL=",
	(char*)"SERVER_SIGNATURE=", (char*)"SERVER_SOFTWARE=", NULL
};
*/

static std::string runExecve(char *ENV[], char *args[], int fdClient) {
    //std::cout << BLU << "START runExeve\n" << RES;
    //std::cout << "ENV: " << ENV[0] << "\n";

    int    		fd[2];

    int    		fdSendBody[2];

    pid_t		retFork;
    std::string	incomingStr;

    if (pipe(fd) == -1)
        std::cout << "Error: Pipe failed\n";

    //std::cout << "pipe fd[0] " << fd[0] << ", pipe fd[1] " << fd[1] << " \n";
    //dup2(fdClient, fd[1]);
    //std::cout << BLU << "POST BODY ENV : " << ENV[2] << "\n" << RES;
    // BY HERE, THE HUGE TEXTFILE IS STORED OK


    retFork = fork();

    if (retFork == 0) {
        std::cout << "    Start CHILD execve()\n";
        if (retFork < 0)
            std::cout << "Error: Fork failed\n";

        dup2(fd[1], 1);
        (void)fdClient;
        //dup2(fdClient, fd[1]);
        close(fd[0]);

        // BIG BODY NEEDS TO BE SENT TO THE CGI BY WRITE TO PIPE
        close(fdSendBody[0]);	// close stdout reading from
        dup2(fdSendBody[1], 1);
        int ret2 = write(fdSendBody[1], "Something ..." , 13);
        std::cout << YEL << "ret from write to CGI : " << ret2 << "\n" << RES;


        // std::cout << YEL << "POST BODY ENV : " << ENV[2] << "\n" << RES;



        int ret = execve(args[0], args, ENV);
        std::cout << RED << "Error: Execve failed: " << ret << "\n" << RES;
    }
    else {
        wait(NULL);
        //std::cout << "    Start Parent\n";
        char buff[100];

        close(fd[1]);
        dup2(fd[0], 0);
        //dup2(fdClient, fd[0]);

        close(fdSendBody[1]);
        close(fdSendBody[0]);
        //dup2(fdSendBody[0], 0);

        //std::cout << RED << "        Start loop reading from child\n" << RES;
        for (int ret = 1; ret != 0; ) {
            memset(buff, '\0', 100);
            ret = read(fd[0], buff, 99);
            incomingStr.append(buff);
        }
        //std::cout << BLU "\n       All content read from CGI\n[" << incomingStr << "]\n" << RES;
    }
    return (incomingStr);
}

void Request::callCGI(RequestData reqData, int fdClient) {
    std::cout << RED << "START CALL_CGI\n" << RES;

    (void)reqData;
    // Declare all necessary variables
    std::string comspec			= "COMSPEC=";
    std::string request_method	= "REQUEST_METHOD=";
    std::string query_string	= "QUERY_STRING=";
    std::string server_name		= "SERVER_NAME=";

    // Declare a vector and fill it with variables, with attached =values
    std::vector<std::string> temp;
    temp.push_back(comspec.append("default"));
    temp.push_back(request_method.append(_data.getRequestMethod()));
    temp.push_back(query_string.append(_data.getQueryString()));
    temp.push_back(server_name.append("default"));

    //std::cout << "Size of vector temp: " << temp.size() << "\n";
    // std::cout << YEL << "POST BODY: " << temp[2] << "\n" << RES;
    // BY HERE, THE HUGE BODY IS STORED OK

    // Make a char** array and copy all content of the above vector
    char **env = new char*[temp.size()  + 1];

    size_t i = 0;
    for (i = 0; i < temp.size(); i++) {
        env[i] = new char[temp[i].length() + 1];
        strcpy(env[i], temp[i].c_str());
    }
    env[i] = NULL;
    //std::cout << YEL << "POST BODY ENV : " << env[2] << "\n" << RES;
    // BY HERE, THE HUGE BODY IS STORED OK

    // Just for printing
    //for (i = 0; env[i]; i++) {
    //   std::cout << env[i] << std::endl;
    //}

    // Prepare the array of the correct command/cgi file to be executed
    // The path of the executable must be according to the 'action file' from the URL
    // char *args[2];
    // args[0] = (char *)"./jaka_cgi/cpp_cgi";   // Make sure the path is correct on Mac/Linux
    // args[1] = NULL;


    // char *args[3];
    // args[0] = (char *)"/usr/bin/php";   // Make sure the path is correct on Mac/Linux
    // args[1] = (char *)"./jaka_cgi/_somePhp.php"; // MUST BE WITH A DOT !!
    // args[2] = NULL;

    char *args[3];
    args[0] = (char *)"/usr/bin/python";   // Make sure the path is correct on Mac/Linux
    args[1] = (char *)"./resources/cgi/python_cgi.py"; // MUST BE WITH A DOT !!
    args[2] = NULL;

    // (void)ENV;
    // (void)fdClient;
    _data.setCgiBody(runExecve(env, args, fdClient));

    std::cout << "Stored CGI Body: [\n" << BLU << _data.getCgiBody() << RES << "]\n";

    // Cleanup
    for (size_t j = 0; j < temp.size(); j++) {
        delete env[j];
    }
    delete[] env;
}

int Request::checkTypeOfFile() {
    std::cout << GRN << "Start checkTypeofFile(), path [" << _data.getPath() << "]" << "\n" << RES;

    std::string path = _data.getPath();
    std::string temp = _data.getPath();

    if (path[0] == '.')
        temp = path.substr(1, std::string::npos);

    std::size_t found = temp.find_last_of(".");

    if (found != std::string::npos) {
        // std::string extention = temp.substr(found, std::string::npos);
        _data.setFileExtention(temp.substr(found, std::string::npos));
    }
    else
        std::cout << GRN << "There is no extention in the last name\n" << RES;
    return (0);
}

// Some arguments not used
static void printPathParts(std::string str, RequestData reqData) {

    std::cout << "Found path:      [" << BLU << str << RES << "]\n";
//	std::cout << "Path trimmed:    [" << BLU << strTrim << RES << "]\n";
    std::cout << "Path:            [" << PUR << reqData.getPath() << RES << "]\n";
    std::cout << "Path first part: [" << PUR << reqData.getPathFirstPart() << RES << "]\n";
    std::cout << "File/Folder:     [" << PUR << reqData.getPathLastWord() << RES << "]\n";
    std::cout << "File extention:  [" << PUR << reqData.getFileExtention() << RES << "]\n";

    std::map<std::string, std::string> formData;
    formData = reqData.getFormData();

    if (! formData.empty()) {
        std::cout << "\nSTORED FORM DATA PAIRS:\n";// Print the map
        std::map<std::string, std::string>::iterator it;
        for (it = formData.begin(); it != formData.end(); it++)
            std::cout << PUR << "   " << it->first << RES << " ---> " << PUR << it->second << "\n" << RES;
    }
    else
        std::cout << "Form Data:    " << YEL << "(not present)\n" << RES;
    std::cout << "\n";
}

int Request::parsePath(std::string str, int fdClient) {

    std::cout << "    start parse path: [" << str << "]\n";	// sleep(1);
//	std::string path			= removeDuplicateSlash(str);	// here error: read buffer overflow
    std::string path			= str;
    size_t		ret				= 0;
//	std::string pathLastWord	= "";


    if (path == "")
        return (-1);
    if (path[0] == '/')
        path = "." + path;

    if (path == "./") {
        std::cout << GRN << "Path is the root '/'\n" << RES;
    }
    if (path.back() == '/'  && (path.find("?") == std::string::npos)) {
        std::cout << GRN << "The path has no GET-Form data. Last char is '/', it must be a folder.\n" << RES;
        storePath_and_FolderName(path);
    }

        // if the last char is not slash /   then look for question mark
        // else if ((ret = path.find("?")) == std::string::npos ) {
    else if ((ret = path.find("?")) == std::string::npos && _data.getRequestMethod() != "POST") {
        std::cout << YEL << "There is no GET or POST method, also the '?' not found\n" << RES;
        _data.setPath(path);
        int pos			= 0;
        pos				= path.find_last_of("/");
        _data.setPathFirstPart(path.substr(0, pos));
        _data.setPathLastWord(path.substr(pos, std::string::npos));
    }

    else if ((ret = path.find("?")) != std::string::npos) {			// Found '?' in the path, maybe also check != "POST"
        std::cout << GRN << "There is GET Form data, the '?' is found\n" << RES;
        storePathParts_and_FormData(path);
    }

    else if (_data.getRequestMethod() == "POST") {
        std::cout << GRN << "There is POST Form data\n" << RES;
        storePath_and_FolderName(path);	// Not sur if this good here ???
        // path is not extracted correctly
        // _data.setQueryString(getRequestBody());
        _data.setQueryString(_data.getBody());
    }

    //std::cout << GRN << "XXX)\n" << RES;
    checkIfFileExists(_data.getPath());	// What in case of root only "/"  ???
    // What is case of GET??
    checkTypeOfFile();

    printPathParts(str, getRequestData());

    (void)fdClient;
    //callCGI(getRequestData(), fdClient);
    return (0);
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

/*	- What if method is GET (normaly without body) AND content-length is not zero ???
	- What if method POST and content-length is zero ???
*/

void    Request::appendToRequest(const char *str, int fdClient) {
	//std::cout << PUR << "AppendToRequest()\n" << RES; // sleep(1);
	std::string 			chunk = std::string(str);
	std::string				strToFind = "\r\n\r\n";
	std::string::size_type	it;
	std::string				tmpHeader;

	if (_headerDone == false) {
		_data.setTemp(_data.getTemp() + chunk);

		if ((it = _data.getTemp().find(strToFind)) != std::string::npos) {
			//std::cout << PUR << "     a)  Found header ending /r/n, maybe there is body\n" << RES;	// sleep(1);
			// _header.append(_temp.substr(0, it));

			tmpHeader = _data.getHeader();
			// tmpHeader.append(_temp.substr(0, it));
			tmpHeader.append(_data.getTemp().substr(0, it));
			_data.setHeader(tmpHeader);
			_headerDone = true;
			//std::cout << "HEADER: [" << BLU << _header << RES "]\n";	// sleep(1);
			parseHeader(_data.getHeader());
			parsePath(_data.getHttpPath(), fdClient);

			if (_data.getRequestContentLength() == 0){
				if (_data.getRequestMethod() == "GET" && _data.getQueryString() != "")
				 	; //callCGI(getRequestData(), fdClient);
				_doneParsing = true;
				return ;
			}
			appendLastChunkToBody(it + strToFind.length(), fdClient);
			//std::cout << "apr() BODY:   [" << BLU << _body   << RES "]\n\n";	// sleep(1);
		}
	}
	else if (_headerDone == true) {
		//std::cout << PUR << "     _headerDone == TRUE\n" << RES;	// sleep(1);
		appendToBody(chunk);
		if (_doneParsing == true && _data.getRequestMethod() == "POST")
			; // callCGI(getRequestData(), fdClient);
	}
	//std::cout << PUR << "End of AppendToRequest()\n" << RES; // sleep(1);
}

//aaaaaiiiiiaaaaaiiiii22


// Last chunk means, last chunk of header section, so first chunk of body
int Request::appendLastChunkToBody(std::string::size_type it, int fdClient) {
	// _body = _temp.substr(it);
	//std::string tmpStr = _data.getTemp();
	//_data.setBody(tmpStr.substr(it));

	_data.setBody(_data.getTemp().substr(it));
	
	// Compare body lenght
	// if (_body.length() > _data.getRequestContentLength()) {
	if (_data.getBody().length() > _data.getRequestContentLength()) {
		std::cout << RED << "Error: Body-Length is bigger than expected Content-Length\n" << RES;
		std::cout << RED << "       Body:             [" << _data.getBody() << "]\n" << RES; // sleep(2);
		std::cout << RED << "       Body-Length:       " << _data.getBody().size() << "\n" << RES; // sleep(2);
		std::cout << RED << "       Req.ContentLength: " << _data.getRequestContentLength() << "\n" << RES; // sleep(2);
		_errorRequest = true;
		return (1);
	}

	if (_data.getBody().length() == _data.getRequestContentLength()) {
		if (_data.getBody().length() == 0 && _data.getRequestContentLength() == 0) {		// Compare body lenght
			std::cout << GRE << "OK (there is no body)\n" << RES;
			_doneParsing = true;
			return (0);
		}
		std::cout << GRE << "OK: Body-Length is as expected Content-Length\n" << RES;
		_doneParsing = true;
		if (_doneParsing == true && _data.getRequestMethod() == "POST") { // can delete doneParsing == true
			std::cout << "      doneparsing true and POST true, call CGI\n";
			// std::cout << "      _body: [" << getRequestBody() << "]\n"; 
			std::cout << "      _body: [" << _data.getBody() << "]\n"; 
			parsePath(_data.getHttpPath(), fdClient);
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
	//std::cout << GRE << "AppendToBody()\n" << RES;

	std::string tmp = _data.getBody();
	tmp.append(req);
	_data.setBody(tmp);
	
	if (_data.getBody().length() > _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << RED "Error: Body-Length is bigger than expected Content-Length\n" RES; // sleep(2);
		std::cout << RED "       Body-Length:       " << _data.getBody() << "\n" RES; // sleep(2);
		std::cout << RED "       Req.ContentLength: " << _data.getRequestContentLength() << "\n" RES; // sleep(2);
		_errorRequest = true;
		return (1);
	}
	else if (_data.getBody().length() == _data.getRequestContentLength()) {		// Compare body lenght
		std::cout << GRE "OK: Done parsing.\n" RES; // sleep(2);

		if (_data.getRequestMethod() == "POST") {
			std::cout << GRE "      ....    store _body into _queryString\n" RES; // sleep(2);
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

// std::string Request::getTemp()	// moved to RequestData
// {
// 	return(_temp);
// }

std::time_t Request::getTime()
{
	return(_startTime);
}
