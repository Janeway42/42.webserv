#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>


#include <sys/stat.h>	// stat()



/*
	What happens if you dont have a form on your page, but you directly write ?city=aaa in the URL?
	In this case, no action file is specified ???
*/

// #include <sys/types.h>
#include <sys/wait.h>	// for wait() on Linux

#include "../includes/Parser.hpp" // for colors
#include "../includes/RequestParser.hpp"




void Request::runExecve(char *ENV[], char *args[], struct kevent event) {
	//std::cout << BLU << "START runExeve\n" << RES;
	(void)event;

	// Create pipes
	_cgi.createPipes(_data.getKqFd(), event);

	int ret = 0;
	pid_t		retFork;

	retFork = fork();

	if (retFork == 0) { // CHILD
		std::cout << YEL "Start CHILD execve()\n" << RES;
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
	std::cout << RED << "START CALL_CGI, cgi path: " << _data.getURLPath() << "\n" << RES;
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
	temp.push_back(server_name.append("default"));
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
	args[0] = (char *)"/usr/bin/python";
	std::string tempPath = _data.getURLPath();
	char *path = (char *)tempPath.c_str();	//  ie: "./resources/cgi//python_cgi_GET.py"
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
std::string removeDuplicateSlash(std::string pathOld) {
  
	char *temp = (char *)malloc(pathOld.length() * sizeof(char) + 1);
	if (temp == NULL)
	{ std::cout << "Error: removeDuplicate failed mallocing\n";  exit(-1); }  

	int beginOfQuery = false;
	size_t j = 0, i = 0;
	while (i < pathOld.length()) {
		if (pathOld[i] == '/' && pathOld[i - 1] == '/' && i != 0 && beginOfQuery == false) {
			i++;
			continue ;
		}
		if (pathOld[i] == '?')
			beginOfQuery = true;
		temp[j++] = pathOld[i++];
	}
	temp[j] = '\0';
	std::string pathNew(temp);
	free(temp);

	if (pathNew[0] == '/')	{
		std::string prefix = "."; // Not sure why this was necessary ???
		prefix.append(pathNew);
		return prefix;
	}
	return pathNew;
}

////////////////////////////////////////////////////////////////


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

	_data.setURLPath(path.substr(0, temp));// TODO change to accept not full path??
    _data.setFullPath(path);// todo keep?
	//std::cout << CYN "StorePathParts() " << _data.getURLPath() << "\n" << RES;
	int posLastSlash 		= tempStr.find_last_of("/");
	int	posFirstQuestMark	= path.find_first_of("?");
	std::string	queryString	= path.substr(temp, std::string::npos);

	_data.setPathFirstPart(tempStr.substr(0, posLastSlash));
	_data.setPathLastWord(path.substr(posLastSlash, posFirstQuestMark - posLastSlash));

	if (queryString[0] == '?') 	// Skip the '?' in the path
		queryString = &queryString[1];

	if (_data.getRequestMethod() == "GET") {
		_data.setQueryString(queryString);
		// _data.setBody(queryString);  // too early
	}

	std::cout << "Stored GET _queryString [\n" << BLU << _data.getQueryString() << RES << "]\n";
	std::cout << YEL "Body:\n" RES;
	std::copy(_data.getBody().begin(), _data.getBody().end(), std::ostream_iterator<uint8_t>(std::cout));  // just to print
	//std::cout << "Stored GET _body [\n"        << BLU << _data.getBody() << RES << "]\n" << RES;

	storeFormData(queryString);	// maybe not needed (the whole vector and map)
	// if the cgi script can handle the whole queryString
}



// !!! Not storing correctly the path part and file name!
// If last '/' is found in path, then this is a folder, not file
void Request::storePath_and_FolderName(std::string path) {
	size_t 	pos1	= 0;
	size_t	pos2	= 0;
	size_t 	count	= 0;

	std::cout << CYN "Start storePath_and_FolderName(). Path: [" << GRN_BG << path << RES << "]\n" << RES;

	// Check if there is query '?' and store path before it
	// Probably not needed searching for query here, because the method is POST, so query not possible
	_data.setURLPath(path);// TODO change to accept not full path??
    _data.setFullPath(path);// todo keep?
	pos1 = path.find_first_of("?");
	if (pos1 != std::string::npos)
		_data.setURLPath(path.substr(0, pos1));// TODO change to accept not full path??
        _data.setFullPath(path);// todo keep?

	pos1	= 0;
	pos2	= path.find_first_of("/");
//	pos2	= path.find_last_of("/");
	while (count < path.length()) {
		if ((count = path.find("/", count)) != std::string::npos) {
			pos1 = pos2;
			pos2 = count;
		}
		if (count == std::string::npos)
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

int Request::checkTypeOfFile() {
	std::cout << "Start checkTypeofFile(). Path: [" << GRN_BG << _data.getURLPath() << RES << "] " RES;

	std::string path = _data.getURLPath();
	std::string temp = _data.getURLPath();


	// CHECK IF THE PATH IS A FILE OR FOLDER, REGARDLES IF IT HAS ANY EXTENTION
    // TODO: USE THE FUNCTION INSIDE PARSER.CPP
	struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (S_ISDIR(s.st_mode)) {
            std::cout << CYN "is a directory\n";
			_data.setIsFolder(true);
			// return (0);
        } else if (S_ISREG(s.st_mode)) {
            std::cout << CYN "is a file\n" RES;
        } else {
            std::cout << CYN "is not a valid directory or file\n" RES;
        }
    } else {
        std::cerr << RED << "Error getting file/directory info: " << strerror(errno) << "\n" RES;
    }



	// IF IT IS A FILE, CHECK AND STORE THE SUFFIX
	if (path[0] == '.')
		temp = path.substr(1, std::string::npos);

	std::size_t found = temp.find_last_of(".");

	if (found != std::string::npos) {
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
	std::cout << "Path:            [" << PUR << reqData.getURLPath() << RES << "]\n";
	std::cout << "Path first part: [" << PUR << reqData.getURLPathFirstPart() << RES << "]\n";
	std::cout << "File/Folder:     [" << PUR << reqData.getURLPathLastWord() << RES << "]\n";
	std::cout << "File extention:  [" << PUR << reqData.getFileExtention() << RES << "]\n";
	std::cout << YEL "Body:\n" RES;
	//std::copy(reqData.getBody().begin(), reqData.getBody().end(), std::ostream_iterator<uint8_t>(std::cout));  // just to print
	// std::cout << "Body:            [" << PUR << reqData.getBody() << RES << "]\n";

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

static int checkIfPathExists(const std::string& path, struct kevent event) {
	
	(void)event;
	std::cout << "Start CheckIfFIleExists(), path [" << path << "] \n" << RES;

	
	std::ifstream file(path.c_str());

	if (not file.is_open()) {		// ??? what is this syntax? -> joyce for cpp we can use not in the pace of ! for readability :)
		std::cout << RED << "Error: File " << path << " not found\n" << RES;
		return (404);
	}
    std::cout << GRN << "File/folder " << RES << path << GRN << " exists\n" << RES;


	// CHECK IF PATH MATCHES THE SERVER ROOT FOLDER
	// Request *storage = (Request*)event.udata;
	// if (path == storage->getServerData().getRootDirectory()) {
	// 	std::cout << GRN << "Path is the root folder\n" << RES;
	// 	return (0);
	// }

	// LOOP THROUGH LOCATIONS AND CHECK IF THERE IS A MATCH, OTHERWISE ERROR 404
	// std::vector<ServerLocation> location_data_vector = storage->getServerData().getLocationBlocks();
	// size_t i;
	// for (i = 0; i < location_data_vector.size(); i++) {
	// 	std::cout << GRN "   ........ location uri: [" << location_data_vector[i].getLocationUriName()) << "]\n";
	// 	std::cout << GRN "   ... location root dir: [" << location_data_vector[i].getRootDirectory() << "]\n";
	// 	std::cout << GRN "   ....... _responsePath: [" << location_data_vector[i].getRootDirectory() << "]\n";
	// 	if (location_data_vector[i].getRootDirectory() == path) {// TODO here it should be getLocationUriName() ?? talk to joyce
	// 		path = location_data_vector[i].getIndexFile();
	// 		std::cout << BLU "   ....... FinalPath: [" << path << "]\n";
	// 	}
	// }
	// if (i == location_data_vector.size()) {
	// 	std::cout << RED "This path exists but does not match any location: [" << path << "]\n";
	// 	storage->setHttpStatus(NOT_FOUND);
	// }

	return 0;
}

/*
	For Joyce: I think there is no separate getter, just to get the path to the cgi location,
	without having to loop through all locations.
	Maybe it is usefull to have a separate getter() for the cgi path, or just folder.
	in line ***** a)
*/

// int Request::parsePath(std::string originalUrlPath) {
int Request::parsePath(std::string originalUrlPath, struct kevent event) {
//	std::string path			= removeDuplicateSlash(originalUrlPath);	// here error: read buffer overflow
	std::string urlPath			= originalUrlPath;
	size_t		ret				= 0;
//	std::string pathLastWord	= "";

	if (originalUrlPath.empty())
		return (-1);
//    if (originalUrlPath[0] == '/' && originalUrlPath == "/")
//        urlPath = getServerData().getRootDirectory();
//    if (originalUrlPath[0] != '/') {
//        if (originalUrlPath == "./") {
//            urlPath = getServerData().getRootDirectory();// TODO 	SHOULD BE PRE PENDED WITH THE ROOT DIRECTORY OF THE LOCATION, NOT THE PATH FROM THE REQUEST
//            std::cout << "Path is the root '/'    [" << GRN_BG << urlPath << RES << "]\n";
//        }
//    }

    std::cout << "originalUrlPath:               [" << GRN_BG << urlPath << RES << "]" << std::endl;
    std::cout << "server block root directory:   [" << GRN_BG << getServerData().getRootDirectory() << RES << "]" << std::endl;

    std::cout << std::endl << GRN << "Starting parsePath() and searching for the correct location block on the config file:" << RES << std::endl;

    std::string serverBlockDir = getServerData().getRootDirectory();
    std::vector<ServerLocation>::const_iterator location = getServerData().getLocationBlocks().cbegin();
    for (; location != getServerData().getLocationBlocks().cend(); ++location) {
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();

        std::cout << "locationBlockUriName:            [" << GRN_BG << locationBlockUriName << RES << "]" << std::endl;
        std::cout << "locationBlockRootDir:            [" << GRN_BG << locationBlockRootDir << RES << "]" << std::endl;

        // When a request comes in, nginx will first try to match the URI to a specific location block.
        if (originalUrlPath[0] != '/') {
            // Todo Ex.: ????
            if (originalUrlPath == "./") {// TODO WHEN IT CAN BE LIKE THIS ./ ???????????????
                urlPath = serverBlockDir;//
                std::cout << "Path is the root '/'    [" << GRN_BG << urlPath << RES << "]\n";
            }
        } else if (originalUrlPath[0] == '/') {
            // Ex.: localhost:8080
            if (originalUrlPath == "/") {
                if (originalUrlPath == locationBlockUriName) {
                    urlPath = locationBlockRootDir;
                } else {
                    urlPath = serverBlockDir;
                }
                break;
            } else if (originalUrlPath != "/" && originalUrlPath.find("?") == std::string::npos && _data.getRequestMethod() != "POST") {
                std::cout << BLU << "No '?' found, so no cgi path needed" << RES << std::endl;
                /* If the url is a file, the match will be done between the directory where the file is against the location uri
                 * ex: url localhost/cgi/test/cgi_index.html -> the /cgi/test part will be checked against a location uri */
                if (pathType(serverBlockDir + originalUrlPath) == REG_FILE) {
                    std::cout << "Path is a file. ";

                    // The if block down below is just for logging
                    if (not originalUrlPath.substr(0, originalUrlPath.find('/') + 1).empty()) {
                        std::cout << "Deleting file from it so it can be matched again the location block uri name. Path: ";
                        std::cout << GRN_BG << originalUrlPath.substr(0, originalUrlPath.find('/') + 1) << RES << std::endl;
                    }

                    if (originalUrlPath.substr(0, originalUrlPath.find('/') + 1) == locationBlockUriName) {
                        urlPath = locationBlockRootDir + originalUrlPath;
                        break;
                    }
                } else if (pathType(serverBlockDir + originalUrlPath) == DIRECTORY){
                    std::cout << YEL << "Path is a directory" << RES << std::endl;
                    if (originalUrlPath == locationBlockUriName) {
                        urlPath = locationBlockRootDir;
                        break;
                    }
                }
            }
        }
        std::cout << YEL << "UrlPath did not match the current locationBlockUriName. Checking the next locationBlockUriName" << RES << std::endl << std::endl;
    }

    /* the URI is "/", nginx will look for a file named "index.html" in the root directory of the server block.
     * If it finds the file, it will be served to the client as the response to the request.
     * If it doesn't find the file, nginx will return a "404 Not Found" error to the client.
     * So, if you have a file named "index.html" in the root directory of your server block and there is no explicit
     * "location /" block defined in your nginx configuration file, then nginx will serve that file by default when
     * someone requests the root URL of your site. */

//	if (urlPath[0] == '/' && urlPath != "/" && urlPath.find("?") == std::string::npos && _data.getRequestMethod() != "POST") {
//		std::cout << YEL << "Path is a file or directory. No '?' found, so no cgi path needed" << RES << std::endl;
//        urlPath = getServerData().getRootDirectory() + urlPath;
//	}
	if ((originalUrlPath[0] == '/' && originalUrlPath != "/" && originalUrlPath.find("?") != std::string::npos) || _data.getRequestMethod() == "POST") {
		std::cout << GRN << "Path is a script file or directory. '?' was found, so cgi path is needed" << RES << std::endl;
		// urlPath = getServerData().getRootDirectory() + "/cgi/" + urlPath;		//   ***** a) // TODO 	SHOULD BE PRE PENDED WITH THE ROOT DIRECTORY OF THE LOCATION, NOT THE PATH FROM THE REQUEST
        urlPath = getServerData().getRootDirectory() + originalUrlPath;		//   ***** a) // TODO 	SHOULD BE PRE PENDED WITH THE ROOT DIRECTORY OF THE LOCATION, NOT THE PATH FROM THE REQUEST
		getResponseData().setIsCgi(true);
	}

    std::cout << GRN << "Path with pre-pended root folder [" << GRN_BG << urlPath << RES << "]\n\n";

	if (urlPath.back() == '/'  && urlPath.find("?") == std::string::npos) {
		std::cout << GRN << "The urlPath has no GET-Form data. Last char is '/', it must be a folder.\n" << RES;
		storePath_and_FolderName(urlPath);
	}
		// if the last char is not slash /   then look for question mark
	else if (urlPath.find("?") == std::string::npos && _data.getRequestMethod() != "POST") {
		std::cout << YEL << "Simple GET (there is no FORM or POST method, also the '?' not found)\n" << RES;
		_data.setURLPath(urlPath);// TODO change to accept not full urlPath
        _data.setFullPath(urlPath);
		int pos			= 0;
		pos				= urlPath.find_last_of("/");
		_data.setPathFirstPart(urlPath.substr(0, pos));
		_data.setPathLastWord(urlPath.substr(pos, std::string::npos));
	} else if (urlPath.find("?") != std::string::npos) {			// Found '?' in the urlPath, maybe also check != "POST"
		std::cout << GRN << "There is GET Form data, the '?' is found\n" << RES;
		storePathParts_and_FormData(urlPath);
	} else if (_data.getRequestMethod() == "POST" || _data.getRequestMethod() == "DELETE") {
		std::cout << GRN << "There is POST Form data\n" << RES;
		storePath_and_FolderName(urlPath);	// Not sur if this good here ???
		
		// urlPath is not extracted correctly
		// _data.setQueryString(getRequestBody());
		//_data.setQueryString(_data.getBody());
	}

	ret = checkIfPathExists(_data.getURLPath(), event);
	if (ret != 0)	{ // What in case of root only "/"  ???
		std::cout << RED << "ret " << ret << ", file not found, should set error to 404)\n" << RES;
        setHttpStatus(NOT_FOUND);
		return (NOT_FOUND);
	}

	//std::cout << GRN "FD _kq: " << _data.getKqFd() << "\n" RES;

	// What in case of GET??
	checkTypeOfFile();
	_data.setResponseContentType(_data.getFileExtention());

	printPathParts(originalUrlPath, getRequestData());
	return (0);
}





