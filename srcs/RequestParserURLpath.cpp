#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

// #include <sys/types.h>
#include <sys/wait.h>	// for wait() on Linux

#include "Parser.hpp"
#include "RequestParser.hpp"

/*
	TODO What happens if you dont have a form on your page, but you directly write ?city=aaa in the URL?
	In this case, no action file is specified ???
*/

void Request::runExecve(char *ENV[], char *args[], struct kevent event) {
	//std::cout << BLU << "START runExeve\n" << RES;
	(void)event;

	// Create pipes
	Request *storage = (Request *)event.udata;

	_cgi.createPipes(storage->getKq(), event);
	// _cgi.createPipes(_data.getKqFd(), event); // moved to Request itself

	int ret = 0;
	pid_t		retFork;

	retFork = fork();

	if (retFork == 0) { // CHILD
		std::cout << CYN <<  "Start CHILD execve()\n" << RES;
		if (retFork < 0)
			std::cout << "Error: Fork failed\n";
	
		close(_cgi.getPipeCgiOut_0());
		close(_cgi.getPipeCgiIn_1());

		ret = dup2(_cgi.getPipeCgiIn_0()   ,  0);		// cgi reads from parent via pipe fd_out
		if (ret == -1)
		 	std::cout << RED "Error dup2() of PipeCgiIn_0, child\n" RES;
		close(_cgi.getPipeCgiIn_0());
		
		//sleep(1);
		ret = dup2(_cgi.getPipeCgiOut_1()   ,  1);	// cgi writes to parent via pipe fd_out NONBLOCK
		if (ret == -1)
		 	std::cout << RED "Error dup2() of PipeCgiOut_1, child\n" RES;
		close(_cgi.getPipeCgiOut_1());

	//	std::cerr << RED "Before execve in child\n" << RES;
		ret = execve(args[0], args, ENV);
	//	ret = execv(args[0], const_cast<char**>(args));
		std::cerr << RED << "Error: Execve failed: " << ret << "\n" << RES;
	}
	else {				// PARENT
		//wait(NULL);
		
		std::cerr << "    Start Parent\n";
		close(_cgi.getPipeCgiOut_1());
		close(_cgi.getPipeCgiIn_0());
		//std::cout << BLU "\n       End runExecve()\n" << RES;
		// sleep(1);
	}
}



void Request::callCGI(struct kevent event) {
	std::cout << CYN << "Start callCGI, cgi path: " << _data.getURLPath_full() << "\n" << RES;
	//(void)reqData;

	// Declare all necessary variables
	std::string request_method	= "REQUEST_METHOD=";
	std::string content_type	= "CONTENT_TYPE=";
	std::string content_length	= "CONTENT_LENGTH=";
	std::string query_string	= "QUERY_STRING=";
	std::string server_name		= "SERVER_NAME=";
	std::string comspec			= "COMSPEC=";

	// Convert length to string
	std::stringstream ssContLen;
	ssContLen << _data.getRequestContentLength();

	// Declare a vector and fill it with variables, with attached =values
	std::vector<std::string> temp;
	temp.push_back(request_method.append(_data.getRequestMethod()));
	temp.push_back(content_type.append(_data.getRequestContentType()));
	temp.push_back(content_length.append(ssContLen.str()));
	temp.push_back(query_string.append(_data.getQueryString()));
	temp.push_back(server_name.append("default"));// TODO add server name?
	temp.push_back(comspec.append("default"));

	// std::cout << "Size of vector temp: " << temp.size() << "\n";
	// std::cout << YEL << "POST BODY: " << temp[2] << "\n" << RES;
	// BY HERE, THE HUGE BODY IS STORED OK

	// Make a char** array and copy all content of the above vector
	//std::cout << GRN " ...... TEMP SIZE:  " << temp.size() << " \n" << RES "\n";

	char **env = new char*[temp.size() + 1];

	size_t i = 0;
	for (i = 0; i < temp.size(); i++) {
		env[i] = new char[temp[i].length() + 1];
		strcpy(env[i], temp[i].c_str());
	}
	env[i] = nullptr;
	//std::cout << YEL << "POST BODY ENV : " << env[2] << "\n" << RES;
	// BY HERE, THE HUGE BODY IS STORED OK

	// Just for printing
	std::cout << GRN "STORED ENV:\n" RES;
	for (i = 0; env[i]; i++) {
	  std::cout << "    " << i+1 << " " << env[i] << std::endl;
	}

	// char *args[3];
	// args[0] = (char *)"/usr/bin/php";   // Make sure the path is correct on Mac/Linux
	// args[1] = (char *)"./jaka_cgi/_somePhp.php"; // MUST BE WITH A DOT !!
	// args[2] = NULL;

	char *args[3];
	args[0] = (char *)"/usr/bin/python";// todo add from config file
	std::string tempPath = _data.getURLPath_full();
	char *path = (char *)tempPath.c_str();	//  ie: "./resources/cgi/python_cgi_GET.py"
	args[1] = path;
	args[2] = NULL;

	// (void)ENV;
	// (void)fdClient;
	//_data.setCgiBody(runExecve(env, args, event));
	runExecve(env, args, event);

	//std::cout << "Stored body from CGI: [\n" << BLU << _data.getCgiBody() << RES << "]\n";

	// Cleanup
	for (size_t j = 0; j < temp.size(); j++) {
		delete env[j];
	}
	delete[] env;
	//std::cout << BLU "\n       End callCGI()\n" << RES;

}

// Not in use
// There is a read buffer overflow
//std::string removeDuplicateSlash(std::string pathOld) {
//
//	char *temp = (char *)malloc(pathOld.length() * sizeof(char) + 1);
//	if (temp == NULL)
//	{ std::cout << "Error: removeDuplicate failed mallocing\n";  exit(-1); }
//
//	int beginOfQuery = false;
//	size_t j = 0, i = 0;
//	while (i < pathOld.length()) {
//		if (pathOld[i] == '/' && pathOld[i - 1] == '/' && i != 0 && beginOfQuery == false) {
//			i++;
//			continue ;
//		}
//		if (pathOld[i] == '?')
//			beginOfQuery = true;
//		temp[j++] = pathOld[i++];
//	}
//	temp[j] = '\0';
//	std::string pathNew(temp);
//	free(temp);
//
//	if (pathNew[0] == '/')	{
//		std::string prefix = "."; // Not sure why this was necessary ???
//		prefix.append(pathNew);
//		return prefix;
//	}
//	return pathNew;
//}

////////////////////////////////////////////////////////////////


/* 	Split string at '&' and store each line into vector<>
	Then split each line in vector into map<> key:value */
// std::map<std::string, std::string> Request::storeFormData(std::string &queryString)
std::map<std::string, std::string> Request::storeFormData(std::string queryString)
{
	std::cout << CYN << "Start store form data()\n" << RES;
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
void Request::storePathParts_and_FormData(std::string const & originalUrlPath) {
	std::string::size_type temp = originalUrlPath.find_first_of("?");
	std::string pathWithoutQuery = originalUrlPath.substr(0, temp);


	int posLastSlash = pathWithoutQuery.find_last_of("/");
	int	posFirstQuestMark = originalUrlPath.find_first_of("?");

//	_data.setPathFirstPart(pathWithoutQuery.substr(0, posLastSlash));
	_data.setPathLastPart(originalUrlPath.substr(posLastSlash, posFirstQuestMark - posLastSlash));
}

void Request::storeURLPathParts(std::string const & originalUrlPath, std::string const & URLPath_full) {
	std::cout << CYN << "Start storeURLPathParts(). URLPath_full: [" << URLPath_full << "]\n" << RES;
    std::string urlPath = originalUrlPath;
    std::string urlPath_full = URLPath_full;
    std::string::size_type lastPart = std::string::npos;
    std::string::size_type posLastSlash = urlPath_full.find_last_of('/');// todo: can a / be inserted as a POST data? ex: city: ams/erdam. Then this would not work

    // If there is query '?', store path before it and anything after the '?' (the query string)
    std::string::size_type positionQuestionMark = URLPath_full.find_first_of('?');
	if (positionQuestionMark != std::string::npos) {
        urlPath = originalUrlPath.substr(0, positionQuestionMark);
        urlPath_full = URLPath_full.substr(0, positionQuestionMark);
        lastPart = positionQuestionMark - posLastSlash;
        // Skip the '?' in the path and store the Query string
        std::string	queryString = URLPath_full.substr(positionQuestionMark);
        if (queryString[0] == '?') {
            queryString = &queryString[1];
        }
        if (_data.getRequestMethod() == "GET") {
            _data.setQueryString(queryString);
            std::cout << "queryString:                   [" << GRN_BG << _data.getQueryString() << RES << "]" << std::endl;
            // _data.setBody(queryString);  // too early todo JAKA is it needed here?
        }
        storeFormData(queryString);	// maybe not needed (the whole vector and map) if the cgi script can handle the whole queryString todo JAKA is it needed?
    }

    _data.setURLPath(urlPath);
    _data.setURLPath_full(urlPath_full);
	_data.setPathFirstPart(urlPath_full.substr(0, posLastSlash + 1));
	_data.setPathLastPart(urlPath_full.substr(posLastSlash + 1, lastPart));
}

/*
char* ENV[25] = {
	(char*)"COMSPEC=", (char*)"DOCUMENT_ROOT=", (char*)"GATEWAY_INTERFACE=", (char*)"HTTP_ACCEPT=", (char*)"HTTP_ACCEPT_ENCODING=",
	(char*)"HTTP_ACCEPT_LANGUAGE=", (char*)"HTTP_CONNECTION=", (char*)"HTTP_HOST=", (char*)"HTTP_USER_AGENT=", (char*)"PATH=",
	(char*)"QUERY_STRING=", (char*)"REMOTE_ADDR=", (char*)"REMOTE_PORT=", request_method2, (char*)"REQUEST_URI=", (char*)"SCRIPT_FILENAME=",
	(char*)"SCRIPT_NAME=", (char*)"SERVER_ADDR=", (char*)"SERVER_ADMIN=", (char*)"SERVER_NAME=",(char*)"SERVER_PORT=",(char*)"SERVER_PROTOCOL=",
	(char*)"SERVER_SIGNATURE=", (char*)"SERVER_SOFTWARE=", NULL
}; TODO THIS COMMENT CAN BE DELETED??
*/

//int checkTypeOfFile() {
//	std::cout << "Start checkTypeofFile(). Path: [" << GRN_BG << _data.getURLPath() << RES << "] " RES;
//
//	std::string path = _data.getURLPath();
//	std::string temp = _data.getURLPath();
//
//	// CHECK IF THE PATH IS A FILE OR FOLDER, REGARDLlES IF IT HAS ANY EXTENSION
//    // TODO: USE THE FUNCTION INSIDE PARSER.CPP
//	struct stat s;
//    if (stat(path.c_str(), &s) == 0) {
//        if (S_ISDIR(s.st_mode)) {
//            std::cout << CYN "is a directory\n";
//			_data.setIsFolder(true);// is being set already inside parsePath_dir
//			// return (0);
//        } else if (S_ISREG(s.st_mode)) {
//            std::cout << CYN "is a file\n" RES;
//        } else {
//            std::cout << CYN "is not a valid directory or file\n" RES;
//        }
//    } else {
//        std::cerr << RED << "Error getting file/directory info: " << strerror(errno) << "\n" RES;
//    }
//	return (0);
//}

HttpStatus Request::checkIfPathExists(std::string const & URLPath_full) {
    std::cout << CYN << "Start CheckIfFIleExists(), URLPath_full [" << URLPath_full << "] \n" << RES;

    // Here at the end URLPath_full will always be a file, either because the request was a file, or because
    // the correct index file was appended to it
    if (pathType(URLPath_full) != REG_FILE) {
        std::cout << RED << "Error: File " << URLPath_full << " not found" << RES << std::endl << std::endl;
        setHttpStatus(NOT_FOUND);
        return (NOT_FOUND);
    } else {

    }
    std::cout << GRN << "File " << RES << URLPath_full << GRN << " exists" << RES << std::endl << std::endl;
//    setHttpStatus(OK); if I set it to ok it has a weird behavior on "error parsing - sending response - failure, error" log line
    return NO_STATUS;
}

static void printPathParts(RequestData reqData) {
	std::cout << std::endl;
	std::cout << "URL Path:        [" << PUR << reqData.getURLPath() << RES << "]\n";
	std::cout << "Full URI Path:   [" << PUR << reqData.getURLPath_full() << RES << "]\n";
	std::cout << "Path first part: [" << PUR << reqData.getURLPathFirstPart() << RES << "]\n";
	std::cout << "Path LAST part:  [" << PUR << reqData.getURLPathLastPart() << RES << "]\n";
	std::cout << "File extension:  [" << PUR << reqData.getFileExtention() << RES << "]\n";
    //std::cout << YEL "Body:\n" RES;
	//std::copy(reqData.getBody().begin(), reqData.getBody().end(), std::ostream_iterator<uint8_t>(std::cout));  // just to print
	//std::cout << "Body:            [" << PUR << reqData.getBody() << RES << "]\n";

	std::map<std::string, std::string> formData;
	formData = reqData.getFormData();
	if (not formData.empty()) {
		std::cout << std::endl << "STORED FORM DATA PAIRS:" << std::endl;
		std::map<std::string, std::string>::iterator it;
		for (it = formData.begin(); it != formData.end(); it++) {
            std::cout << PUR << "   " << it->first << RES << " ---> " << PUR << it->second << "\n" << RES;
        }
	} else {
        std::cout << "Form Data:    " << YEL << "(not present)\n" << RES;
    }
    std::cout << std::endl;
}

static std::string getExtension(std::string const & originalUrlPath) {
    // Ex.: localhost:8080/favicon.ico or localhost:8080/cgi/python_cgi_GET.py?street=test&city=test+city or localhost:8080/index.html
    std::string urlPath = originalUrlPath;
    std::string extension = std::string();

    std::string::size_type hasQuery = originalUrlPath.find_first_of('?');
    if (hasQuery != std::string::npos) {
        urlPath = originalUrlPath.substr(0, hasQuery);
    }

    std::string::size_type hasExtension = urlPath.find_last_of(".");
    if (hasExtension != std::string::npos) {
        extension = urlPath.substr(hasExtension);
        std::cout << BLU << "File extension: "  << extension << RES << std::endl;
    } else {
        std::cout << YEL << "There is no extension in the file" << RES << std::endl;
    }
    return extension;
}

bool Request::parsePath_cgi(std::string const & originalUrlPath, std::string const & locationBlockUriName) {
    if (originalUrlPath.find('?') != std::string::npos) {
        getCgiData().setIsCgi(true);

        std::cout << BLU << "'?' was found, so cgi root_directory is needed" << RES << std::endl;
        std::cout << "Path is a script file. ";

        // blocks down below are just for logging
        if (not _data.getFileExtention().empty()) {
            std::cout << "Deleting file name from it so the extension can be matched against the location block uri extension name. Extension: ";
            std::cout << GRN_BG << _data.getFileExtention() << RES << std::endl;
        }
        if (_data.getRequestMethod() == "GET") {
            std::cout << GRN << "There is GET Form data" << std::endl << RES;
        } else if (_data.getRequestMethod() == "POST") {
            std::cout << PUR << "There is POST Form data" << RES;
        } else if (_data.getRequestMethod() == "DELETE") {
            std::cout << PUR << "There is DELETE Form data" << RES;
        }

        /* If the url is a script file, the match will be done between the extension of it, against the location uri
         * ex: url localhost/cgi/script.py -> the .py part will be checked against a location uri */
        if (_data.getFileExtention() == locationBlockUriName) {
            return true;
        }
    }
    return false;
}

bool Request::parsePath_dir(std::string const & originalUrlPath) {
    if (originalUrlPath.find('?') == std::string::npos) {
        _data.setIsFolder(true);
        //old log: //std::cout << GRN << "The URLPath_full has no GET-Form data. Last char is '/', it must be a folder.\n" << RES;
        std::cout << BLU << "No '?' found and URLPath_full has no GET-Form data" << RES << std::endl;
        std::cout << "Path is a directory." << std::endl << RES;
        return true ;
    }
    // if the last char is not slash / then look for question mark TODO this comment was on the code before
    // todo: does it mean that is there is no / at the end you were considering it was not a folder?
    // todo: and so if it is not a folder we ca look for a '?' but if it a folder no '?' is possible?
    // todo: otherwise, if '?' if possible with a url containing a folder, then we need to handler query here too
    return false;
}

bool Request::parsePath_file(std::string const & originalUrlPath, std::string const & locationBlockUriName) {
    std::string DirFromUrl;

    if (originalUrlPath.find('?') == std::string::npos && _data.getRequestMethod() != "POST") {
        std::cout << BLU << "No '?' found, so no cgi root_directory needed" << RES << std::endl;
        std::cout << "Path is a file. ";

        // If there is only one / then it has to keep it, otherwise DirFromUrl can be without it
        std::string::size_type positionLastSlash = originalUrlPath.find_last_of('/');
        if (positionLastSlash == 0) {
            positionLastSlash += 1;
        }
        DirFromUrl = originalUrlPath.substr(0, positionLastSlash);

        // The if block down below is just for logging
        if (not DirFromUrl.empty()) {
            std::cout << "Deleting file from it so it can be matched against the location block uri name. Path: ";
            std::cout << GRN_BG << DirFromUrl << RES << std::endl;
        }
    }

    /* If the url is a file, the match will be done between the directory where the file is, against the location uri
     * ex: url localhost/cgi/cgi_index.html -> the /cgi part will be checked against a location uri */
    if (DirFromUrl == locationBlockUriName) {
        return true;
    }
    return false;
}

std::string Request::parsePath_edgeCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {//todo mayne not needed?
    std::string URLPath_full = std::string();
    /* https://www.tutorialspoint.com/http/http_requests.htm
     * The most common form of Request-URI is that used to identify a resource on an origin server or gateway. Note that
     * the absolute path cannot be empty; if none is present in the original URI, it MUST be given as "/" (the server root) */
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();
    /* The absoluteURI is used when an HTTP request is being made to a proxy.
     * The proxy is requested to forward the request or service from a valid cache, and return the response.
     * For example: GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1 */
    //if (originalUrlPath == "./") {// TODO WHEN IT CAN BE LIKE THIS ./ ???????????????
    //    URLPath_full = serverBlockDir;// is being done down below if it does not match any location
    //    std::cout << "Path is the root '/'    [" << GRN_BG << URLPath_full << RES << "]\n";
    //    break;
    //}
    //Todo Ex.: ????
    if (originalUrlPath == "./" && locationBlockUriName == "/") {
        std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
        std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
        std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
        _data.setIsFolder(true);
        URLPath_full = locationBlockRootDir + '/' + location->getIndexFile();
    }
    return URLPath_full;
}

std::string Request::parsePath_regularCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    std::string URLPath_full = std::string();
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();

    // Ex.: localhost:8080
    if (originalUrlPath.size() == 1) {// was: originalUrlPath == "/"
        /* If the URI is "/", nginx will look for a file named "index.html" in the root directory of the server block.
         * If it finds the file, it will be served to the client as the response to the request.
         * If it doesn't find the file, nginx will return a "404 Not Found" error to the client (at the end of this function) */
        if (originalUrlPath == locationBlockUriName) {
            std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
            std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
            std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
            _data.setIsFolder(true);
            URLPath_full = locationBlockRootDir + '/' + location->getIndexFile();
        }
    } else if (originalUrlPath != "/") {// TODO SEND THIS PART TO parsePath_file and parsePath_cgi
        // -------------------------------------------------------------------------------------- FILE or QUERY
        // Ex.: localhost:8080/favicon.ico or localhost:8080/cgi/python_cgi_GET.py?street=test&city=test+city
        if (pathType(locationBlockRootDir + originalUrlPath.substr(0, originalUrlPath.find('?'))) == REG_FILE
            || pathType(locationBlockRootDir + originalUrlPath) == REG_FILE) {

            _data.setFileExtention(getExtension(originalUrlPath));
            if (parsePath_file(originalUrlPath, locationBlockUriName) == true ||
                parsePath_cgi(originalUrlPath, locationBlockUriName) == true) {
                std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
                std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
                std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
                URLPath_full = locationBlockRootDir + originalUrlPath;
            }
        }

        // -------------------------------------------------------------------------------------- DIRECTORY
        // TODO SEND THIS PART TO parsePath_dir
        // If it is a directory and has a / at the end, delete it, so it can be matched against the config file locations
        std::string originalUrlPath_NoSlash = originalUrlPath;
        if (originalUrlPath.back() == '/') {
            originalUrlPath_NoSlash = originalUrlPath.substr(0, originalUrlPath.size() - 1);
        }
        // Ex.: localhost:8080/test/ or localhost:8080/test
        if (originalUrlPath_NoSlash == locationBlockUriName) {
            if (parsePath_dir(originalUrlPath) == true) {
                std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
                std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
                std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
                URLPath_full = locationBlockRootDir + originalUrlPath_NoSlash + '/' + location->getIndexFile();
            }
        }
    }
    return URLPath_full;
}

std::string Request::parsePath_locationMatch(std::string const & originalUrlPath) {
    std::string URLPath_full = std::string();

    std::vector<ServerLocation>::const_iterator location = getServerData().getLocationBlocks().cbegin();
    for (; location != getServerData().getLocationBlocks().cend(); ++location) {
        std::cout << "⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻" << std::endl;
        std::cout << "locationBlockUriName:            [" << GRN_BG << location->getLocationUriName() << RES << "]" << std::endl;
        std::cout << "locationBlockRootDir:            [" << GRN_BG << location->getRootDirectory() << RES << "]" << std::endl;

        URLPath_full = parsePath_regularCase(originalUrlPath, location);
        if (originalUrlPath[0] == '/' && not URLPath_full.empty()) {
            break;
        }
        URLPath_full = parsePath_edgeCase(originalUrlPath, location);
        if (originalUrlPath[0] != '/' && not URLPath_full.empty()) {
            break;
        }
        std::cout << YEL << "The url path [" << originalUrlPath << "] did not match the current location block [";
        std::cout << location->getLocationUriName() << "] from the config file. ";
        std::cout << "Checking the next locationBlockUriName" << RES << std::endl;
    }
    return URLPath_full;
}

// In case error 404, file not found, it's probably good to not continue after parsePath(), and just close the connection
HttpStatus Request::parsePath(std::string  const & originalUrlPath) {
    std::string serverBlockDir = getServerData().getRootDirectory();

    std::cout << "originalUrlPath:               [" << GRN_BG << originalUrlPath << RES << "]" << std::endl;
    std::cout << "server block root directory:   [" << GRN_BG << getServerData().getRootDirectory() << RES << "]" << std::endl;
    std::cout << std::endl << GRN << "Starting parsePath() and searching for the correct location block on the config file:" << RES;
    std::cout << std::endl << std::endl;

	if (originalUrlPath.empty()) {
		return NO_CONTENT;// Todo: return maybe another status?
    }

    // When a request comes in, nginx will first try to match the URI to a specific location block.
	std::string URLPath_full = parsePath_locationMatch(originalUrlPath);
    if (URLPath_full.empty()) {
        std::cout << std::endl << RED << "As the UrlPath did not match any location block, ";
//        std::cout << "the server block root_directory [" << serverBlockDir << "] and configuration will be used" << RES << std::endl;
//        URLPath_full = serverBlockDir + '/' + getServerData().getIndexFile();
        std::cout << "the server cannot serve any file" << RES << std::endl << std::endl;
        setHttpStatus(NOT_FOUND);
        return NOT_FOUND;
    }
    std::cout << "⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻" << std::endl << std::endl;
	_data.setResponseContentType(_data.getFileExtention());
    storeURLPathParts(originalUrlPath, URLPath_full);// TODO: do that only if _data.getRequestMethod() != "POST" ?????

    printPathParts(_data);
    return checkIfPathExists(_data.getURLPath_full());
}
