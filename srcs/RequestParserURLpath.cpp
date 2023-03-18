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




static std::string runExecve(char *ENV[], char *args[], int fdClient) {
	//std::cout << BLU << "START runExeve\n" << RES;
	//std::cout << "ENV: " << ENV[0] << "\n";

	int    		fd[2];

	//int    		fdSendBody[2];

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
		std::cout << RED "Start CHILD execve()\n" RES;
		if (retFork < 0)
			std::cout << "Error: Fork failed\n";

		dup2(fd[1], 1);
		(void)fdClient;
		//dup2(fdClient, fd[1]);
		close(fd[0]);

		// BIG BODY NEEDS TO BE SENT TO THE CGI BY WRITE TO PIPE
		//close(fdSendBody[0]);	// close stdout reading from
		//dup2(fdSendBody[1], 1);
		//int ret2 = write(fdSendBody[1], "Something ..." , 13);
		//std::cout << YEL << "ret from write to CGI : " << ret2 << "\n" << RES;


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

		//close(fdSendBody[1]);
		//close(fdSendBody[0]);
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


int Request::checkTypeOfFile() {
	std::cout << GRN << "Start checkTypeofFile(), path [" << _data.getPath() << "] " RES;

	std::string path = _data.getPath();
	std::string temp = _data.getPath();


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

int checkIfPathExists(const std::string& path, struct kevent event) {
	
	(void)event;
	std::cout << GRN << "Start CheckIfFIleExists(), path [" << path << "] \n" << RES;

	
	std::ifstream file(path.c_str());

	if (not file.is_open()) {		// ??? what is this syntax? -> joyce for cpp we can use not in the pace of ! for readability :)
		std::cout << RED << "Error: File " << path << " not found\n" << RES;
		return (404);
	}
	std::cout << GRN << "File/folder " << path << " exists\n" << RES;


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
	// 	std::cout << GRE "   ........ location uri: [" << location_data_vector[i].getLocationPath() << "]\n";
	// 	std::cout << GRE "   ... location root dir: [" << location_data_vector[i].getRootDirectory() << "]\n";
	// 	std::cout << GRE "   ....... _responsePath: [" << location_data_vector[i].getRootDirectory() << "]\n";
	// 	if (location_data_vector[i].getRootDirectory() == path) {// TODO here it should be getLocationPath() ?? talk to joyce
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






// int Request::parsePath(std::string str) {
int Request::parsePath(std::string str, struct kevent event) {

	std::cout << GRN "Start parse path: [" << str << "]\n";	// sleep(1);
//	std::string path			= removeDuplicateSlash(str);	// here error: read buffer overflow
	std::string path			= str;
	size_t		ret				= 0;
//	std::string pathLastWord	= "";

	if (path == "")
		return (-1);
	if (path[0] == '/' && path != "/")
		path = getServerData().getRootDirectory();// + path; -> JOyce: I have commented out the + path part since it was not matching ./resources/server_root/ with ./resources/server_root for example
	if (path[0] == '/' && path == "/")
		path = getServerData().getRootDirectory();
	if (path[0] != '/')
	if (path == "./") {
		path = getServerData().getRootDirectory();
		//path = path + "resources"; // !!! How to grab the server root name? The path should contain the root folder, ie: ./sources 
		std::cout << GRN << "Path is the root '/'    [" << path << "]\n" RES;
	}
	std::cout << GRN << "Path with pre-pended root folder [" << path << "]\n" RES;

	// Pre-pend the name of the root folder


	if (path.back() == '/'  && (path.find("?") == std::string::npos)) {
		std::cout << GRN << "The path has no GET-Form data. Last char is '/', it must be a folder.\n" << RES;
		storePath_and_FolderName(path);
	}

		// if the last char is not slash /   then look for question mark
		// else if ((ret = path.find("?")) == std::string::npos ) {
	else if ((ret = path.find("?")) == std::string::npos && _data.getRequestMethod() != "POST") {
		std::cout << YEL << "Simple GET (there is no FORM or POST method, also the '?' not found)\n" << RES;
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

	else if (_data.getRequestMethod() == "POST" || _data.getRequestMethod() == "DELETE") {
		std::cout << GRN << "There is POST Form data\n" << RES;
		storePath_and_FolderName(path);	// Not sur if this good here ???
		// path is not extracted correctly
		// _data.setQueryString(getRequestBody());
		_data.setQueryString(_data.getBody());
	}

	ret = checkIfPathExists(_data.getPath(), event);
	if (ret != 0)	{ // What in case of root only "/"  ???
		std::cout << RED << "ret " << ret << ", file not found, should set error to 404)\n" << RES;
        setHttpStatus(NOT_FOUND);
		return (NOT_FOUND);
	}

	//Request *storage = (Request *)event.udata;	
	


	// What in case of GET??
	checkTypeOfFile();
	_data.setRequestContentType(_data.getFileExtention());

	printPathParts(str, getRequestData());
	return (0);
}





