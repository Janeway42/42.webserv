#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem> // temp, to test current directory

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

        // Change current working directory to the internal CGI directory
        // Best practice to ensure the script to find correct relative paths, if needed
        chdir("./resources/_cgi/");

	//	std::cerr << RED "Before execve in child\n" << RES;
		ret = execve(args[0], args, ENV);
		std::cerr << RED << "Error: Execve failed: " << ret << "\n" << RES;
        // TODO: handle error if execve failed
	}
	else {				// PARENT
		//wait(NULL);
		
		//std::cerr << "    Start Parent\n";
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
	std::string info_path		= "INFO_PATH=";
	std::string upload_path		= "UPLOAD_DIR=";

	// Convert length to string
	std::stringstream ssContLen;
	ssContLen << _data.getRequestContentLength();

	// Declare a vector and fill it with variables, with attached =values
	std::vector<std::string> temp;
	temp.push_back(request_method.append(_data.getRequestMethod()));
	temp.push_back(content_type.append(_data.getRequestContentType()));
	temp.push_back(content_length.append(ssContLen.str()));
	temp.push_back(query_string.append(_data.getQueryString()));
	temp.push_back(server_name.append(getServerData().getServerName()));
//  temp.push_back(server_name.append("defaultServerName"));// TODO add server name?
	temp.push_back(comspec.append(""));
	temp.push_back(info_path.append(""));           // todo: find out if info_path is mandatory and how to test it?
//	temp.push_back(upload_path.append(_data.get));  // todo: append the /upload folder name

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
    	args[0] = (char *)"/usr/local/bin/python3";
//	args[0] = (char *)"/usr/bin/python";    // TODO: add from config file -> /bin/bash -c python or /usr/bin/env python

//  std::string tempPath = _data.getURLPath_full();   // Changed Jaka:
    std::string tempPath = _data.getURLPathLastPart();  // must be only the script name, because the CWD is changed to CGI folder

	char *path = (char *)tempPath.c_str();	//  ie: "./resources/_cgi/python_cgi_GET.py"
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

void Request::storeURLPathParts(std::string const & originalUrlPath, std::string const & URLPath_full) {
    std::cout << CYN << "Start storeURLPathParts(). URLPath_full: [" << URLPath_full << "]\n" << RES;
    std::string::size_type posLastSlash = URLPath_full.find_last_of('/');// todo: can a / be inserted as a POST data? ex: city: ams/erdam. Then this would not work

    // todo joyce -> I will keep those line commented ouy for now but I Think we wont need them
//    std::string urlPath = originalUrlPath;
//    std::string urlPath_full = URLPath_full;
//    std::string::size_type lastPart = std::string::npos;

    // If there is query '?' string, store it
    std::string::size_type positionQuestionMark = originalUrlPath.find_first_of('?');
	if (positionQuestionMark != std::string::npos) {
        // todo joyce -> I will keep those line commented ouy for now but I Think we wont need them
//        urlPath = originalUrlPath.substr(0, positionQuestionMark);
//        urlPath_full = URLPath_full.substr(0, positionQuestionMark);
//        std::string::size_type lastSlash = originalUrlPath.find_last_of('/');
//        lastPart = positionQuestionMark - lastSlash;

        // Skip the '?' in the path and store the Query string
        std::string	queryString = originalUrlPath.substr(positionQuestionMark + 1);
        if (_data.getRequestMethod() == "GET") {
            _data.setQueryString(queryString);
            std::cout << "queryString:                   [" << GRN_BG << _data.getQueryString() << RES << "]" << std::endl;
            // _data.setBody(queryString);  // too early todo JAKA is it needed here?
        }
        storeFormData(queryString);	// maybe not needed (the whole vector and map) if the cgi script can handle the whole queryString todo JAKA is it needed?
    }

    _data.setURLPath(originalUrlPath);
    _data.setURLPath_full(URLPath_full);
	_data.setPathFirstPart(URLPath_full.substr(0, posLastSlash + 1));
	_data.setPathLastPart(URLPath_full.substr(posLastSlash + 1));
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


// Disabled by Jaka, new function is below
// void Request::checkIfPathExistsOLD(std::string const & URLPath_full) {
//     std::cout << CYN << "Start CheckIfPathExists(), URLPath_full [" << URLPath_full << "] \n" << RES;

//     // Here at the end URLPath_full will always be a file, either because the request was a file, or because
//     // the correct index file was appended to it (from location or server block, the config file parser decided already)
//     if (pathType(URLPath_full) != REG_FILE) {
//         std::cout << RED << std::endl << "Error: file [" << RES << URLPath_full << RED;
//         std::cout << "] was not found. Returning 404 NOT FOUND" << RES << std::endl << std::endl;
//         setHttpStatus(NOT_FOUND);
//     } else {
//         std::cout << GRN << "Path " << RES << URLPath_full << GRN << " exists" << RES << std::endl << std::endl;
//         if (getHttpStatus() != MOVE_PERMANENTLY) {
//             setHttpStatus(OK);
//         }
//     }
// }

void Request::checkIfPathExists(std::string const & URLPath_full) {
    std::cout << CYN << "Start CheckIfPathExists(), URLPath_full [" << URLPath_full << "] \n" << RES;

    // Here at the end URLPath_full will always be a file, either because the request was a file, or because
    // the correct index file was appended to it (from location or server block, the config file parser decided already)
    PathType type = pathType(URLPath_full);
    if (type != REG_FILE) {
        std::cout << RED << std::endl << "Error: file [" << RES << URLPath_full << RED;
        std::cout << "] was not found. Returning 404 NOT FOUND" << RES << std::endl << std::endl;
        setHttpStatus(NOT_FOUND);
    } else if (type == REG_FILE) {
        std::cout << GRN << "Path " << RES << URLPath_full << GRN << " exists" << RES << std::endl << std::endl;
        setHttpStatus(OK);
    }

    // If the path with a file does not exist, maybe the original URL is just a folder.
    // In this case, check if the original path is a folder, then check if this location has autoindex on or off.
    // If autindex is off, return 403 Forbidden, else return folder content.
    std::string urlPathFolder = getServerData().getRootDirectory();
    urlPathFolder.append(_data.getURLPath());               // Append root to URLpath, to get the correct path of the folder
    if (pathType(URLPath_full) != REG_FILE && pathType(urlPathFolder) == DIRECTORY) {
        std::cout << YEL "Path is " << "\n" RES;

        // TODO FIRST: it needs to iterate through all locations and look at the autindex status !!
        // if autindex is off, return 403, else the folder content
        //if (getServerData().getAutoindexStatus() == "off")
            setHttpStatus(FORBIDDEN);
    }

}

static void printPathParts(RequestData reqData) {
	std::cout << std::endl;
	std::cout << "URL Path:        [" << PUR << reqData.getURLPath() << RES << "]\n";
//	std::cout << "HTTP Path:       [" << PUR << reqData.getHttpPath() << RES << "]\n";  // todo, this is the same as getURLPath(), can be deleted from .hpp
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
    // Ex.: localhost:8080/favicon.ico or localhost:8080/_cgi/python_cgi_GET.py?street=test&city=test+city or localhost:8080/index.html
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

std::string Request::parsePath_cgi(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location, std::string const & file_cgi) {
    // localhost:8080/_cgi/python_cgi_GET.py?street=test&city=test+city
//    if (originalUrlPath.find('?') != std::string::npos) {
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();
        getCgiData().setIsCgi(true);

        std::cout << BLU << "'?' was found, so cgi root_directory is needed" << RES << std::endl;
        std::cout << "Path is a script file. ";

        // blocks down below are just for logging
        if (not _data.getFileExtention().empty()) {
            std::cout << "Deleting file name from it so the extension can be matched against the location block uri extension name. Extension: ";
            std::cout << GRN_BG << _data.getFileExtention() << RES << std::endl;
        }
        if (_data.getRequestMethod() == "GET") {
            std::cout << GRN << "There is GET Form data" << RES << std::endl;
        } else if (_data.getRequestMethod() == "POST") {
            std::cout << GRN << "There is POST Form data" << RES << std::endl;
        } else if (_data.getRequestMethod() == "DELETE") {
            std::cout << GRN << "There is DELETE Form data" << RES << std::endl;
        }

        /* If the url is a script file, the match will be done between the extension of it, against the location uri
         * ex: url localhost/_cgi/script.py -> the .py part will be checked against a location uri */
        if (_data.getFileExtention() == locationBlockUriName) {
            std::cout << BLU << "cgi location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
            std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
            std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
            return locationBlockRootDir + file_cgi;
        }
//    }
    return std::string();
}

std::string Request::parsePath_dir(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location, std::string const & subdirectory) {
    // Ex.: localhost:8080/test/ or localhost:8080/test/sub/
    // Obs.: A directory URI always have a / at the end (otherwise it is a file)
    if (originalUrlPath.find('?') == std::string::npos) {
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();

        std::cout << BLU << "No '?' found and originalUrlPath has no GET-Form data" << RES << std::endl;

        // Here we are matching the whole path with the whole locationBlockUriName + / (since it comes without it
        // from the config file)
        if (originalUrlPath == (locationBlockUriName + '/')) {
            std::cout << "Path is a directory." << std::endl << RES;
            _data.setIsFolder(true);
            _data.setAutoIndex(location->getAutoIndex());
            std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
            std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
            std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;

//            std::cout << RED << "JJOOYYCCEE subdirectory: " << subdirectory << RES << std::endl;
            //if (pathType(locationBlockRootDir + subdirectory) == DIRECTORY)// todo ????
            return locationBlockRootDir + (subdirectory == "/" ? "" : subdirectory) + '/' + location->getIndexFile();
            // TODO _data.setFileExtention(getExtension(originalUrlPath)) here too for .html??
        }
    }
    // if the last char is not slash / then look for question mark TODO this comment was on the code before
    // todo: does it mean that is there is no / at the end you were considering it was not a folder?
    // todo: and so if it is not a folder we ca look for a '?' but if it a folder no '?' is possible?
    // todo: otherwise, if '?' if possible with a url containing a folder, then we need to handler query here too
    return std::string();
}

std::string Request::parsePath_file(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    // Ex.: localhost:8080/favicon.ico or localhost:8080/_cgi/cgi_index.html
    if (originalUrlPath.find('?') == std::string::npos && _data.getRequestMethod() != "POST") {
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();
        std::string DirFromUrl = std::string();
        std::string file = originalUrlPath.substr(originalUrlPath.find_last_of('/'));

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

        /* If the url is a file, the match will be done between the directory where the file is, against the location uri
         * ex: url localhost/_cgi/cgi_index.html -> the /_cgi part will be checked against a location uri */
        if (DirFromUrl == locationBlockUriName) {
            std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
            std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
            std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;

//            if (pathType(locationBlockRootDir + file) == REG_FILE)// todo????
            return locationBlockRootDir + file;
//            else
//                return locationBlockRootDir;
        }
    }
    // Script file for POST, with no query ('?')
    // Ex.: localhost:8080/python_POST.py or localhost:8080/_cgi/python_cgi_POST_upload.py or localhost:8080/py/cgi_delete.py
    else if (originalUrlPath.find('?') == std::string::npos && _data.getRequestMethod() == "POST") {
        return parsePath_cgi(originalUrlPath, location, originalUrlPath.substr(originalUrlPath.find_last_of('/')));
    }
    return std::string();
}

std::string Request::parsePath_regularCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    std::string locationBlockRootDir = location->getRootDirectory();

    if (originalUrlPath != "/") {
        std::string::size_type lastSlash = originalUrlPath.find_last_of('/');

        // -------------------------------------------------------------------------------------------------- FILE
        if (lastSlash != std::string::npos) {
            std::string file = originalUrlPath.substr(lastSlash);
//            std::cout << RED << "joyce locationBlockRootDir + file: " << locationBlockRootDir + file << RES << std::endl;
            if (not file.empty() && (pathType(locationBlockRootDir + file) != DIRECTORY
                    && pathType(locationBlockRootDir + file) != PATH_TYPE_ERROR)) {
//                std::cout << RED << "joyce file: " << file << RES << std::endl;
                _data.setFileExtention(getExtension(originalUrlPath));
                std::string URLPath_full_file = parsePath_file(originalUrlPath, location);
                if (not URLPath_full_file.empty()) {
                    return URLPath_full_file;
                }
            }
        }
        // -------------------------------------------------------------------------------------------------- QUERY
        std::string::size_type firstQuestionMark = originalUrlPath.find_first_of('?');
        if (firstQuestionMark != std::string::npos) {
            std::string fileCgi = originalUrlPath.substr(lastSlash,  firstQuestionMark - lastSlash);
            _data.setFileExtention(getExtension(originalUrlPath));
            if (not fileCgi.empty() && pathType(locationBlockRootDir + fileCgi) == REG_FILE) {
                std::string URLPath_full_cgi = parsePath_cgi(originalUrlPath, location, fileCgi);
                if (not URLPath_full_cgi.empty()) {
                    return URLPath_full_cgi;
                }
            }
        }
        // -------------------------------------------------------------------------------------------------- DIRECTORY
        std::string locationBlockRootDir;
        if (lastSlash > 0) {
            // If lastSlash is not zero, then we are dealing with subdirectories (i.e.: we are dealing with /dir/dir1/ex/)
            // Which means we want to Match the locationBlockRootDir (without the location dir on it) to the path
            // to it. E.g.: path /dir/dir1/ex/ will be ./resources/dir/dir1/ex/
            std::string::size_type secondSlash = originalUrlPath.find_first_of('/', 1);
            if (secondSlash != std::string::npos) {
                subdirectoryFromUrl = originalUrlPath.substr(secondSlash);
            }
        }
//      std::cout << RED << "JOYCE locationBlockRootDir + subdirectoryFromUrl: " << locationBlockRootDir + subdirectoryFromUrl << RES << std::endl;
        if (pathType(locationBlockRootDir + subdirectoryFromUrl) == DIRECTORY) {
        //if (pathType(locationBlockRootDir + subdirectoryFromUrl) != REG_FILE
                //&& pathType(locationBlockRootDir + subdirectoryFromUrl) != PATH_TYPE_ERROR) {
            std::string URLPath_full_dir = parsePath_dir(originalUrlPath, location, subdirectoryFromUrl);
            if (not URLPath_full_dir.empty()) {
                return URLPath_full_dir;
            }
        } else {
            // Handling auto index errors, (in the case where the path is NOT a directory):
            // if auto index is on: return 403
            // if auto index is off: return 404
            if (_data.getAutoIndex()) {
                setHttpStatus(FORBIDDEN);
            } else {
                setHttpStatus(NOT_FOUND);
            }
        }
    }
    return std::string();
}

std::string Request::parsePath_edgeCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {//todo mayne not needed?
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();
    /* https://www.tutorialspoint.com/http/http_requests.htm
     * The most common form of Request-URI is that used to identify a resource on an origin server or gateway. Note that
     * the absolute path cannot be empty; if none is present in the original URI, it MUST be given as "/" (the server root) */

    /* The absoluteURI is used when an HTTP request is being made to a proxy.
     * The proxy is requested to forward the request or service from a valid cache, and return the response.
     * For example: GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1 */

    //if (originalUrlPath == "./") {// TODO WHEN IT CAN BE LIKE THIS ./ ???????????????
    //    std::cout << "Path is the root '/'    [" << GRN_BG << originalUrlPath << RES << "]\n";
    //    return serverBlockDir;// is being done down below if it does not match any location

    //Todo Ex.: ????
    if (originalUrlPath == "./" && locationBlockUriName == "/") {
        std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
        std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]" << std::endl;
        std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
        _data.setIsFolder(true);
        _data.setAutoIndex(location->getAutoIndex());
        return locationBlockRootDir + '/' + location->getIndexFile();
    }
    return std::string();
}

std::string Request::parsePath_root(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();
    /* Ex.: localhost:8080
     * If the URI is "/", nginx will look for a file named "index.html" in the root directory of the server block.
     * If it finds the file, it will be served to the client as the response to the request.
     * If it doesn't find the file, nginx will return a "404 Not Found" error to the client (at the end of parsePath()) */
    if (originalUrlPath == locationBlockUriName) {
        std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES BLU;
        std::cout << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES BLU << "]"
                  << std::endl;
        std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES
                  << std::endl;
        _data.setIsFolder(true);
        _data.setAutoIndex(location->getAutoIndex());
        // Checking if it needs to be redirected to another location, if so, it will run parsePath_locationMatch()
        // again looking to the new location match
        return locationBlockRootDir + '/' + location->getIndexFile();
    }
    return std::string();
}

// receiving a copy of originalUrlPath, so it can be modified in case we have a redirection on the location block
void Request::checkRedirection(std::string getRedirection) {
    if (not getRedirection.empty()) {
        std::cout << YEL << "Redirection found: " << getRedirection << RES << std::endl;
        std::cout << YEL << "A 301 response response will be sent, with the redirection url on it!" << RES << std::endl;
        setHttpStatus(MOVE_PERMANENTLY);
        setRedirection(getRedirection);
    }
}

std::string Request::parsePath_locationMatch(std::string const & originalUrlPath) {
    std::string URLPath_full = std::string();

    std::vector<ServerLocation>::const_iterator location = getServerData().getLocationBlocks().cbegin();
    for (; location != getServerData().getLocationBlocks().cend(); ++location) {
        // TODO -> Set 405 Method Not Allowed IF specific location allowed methods does not match with the method received
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();
        std::cout << "⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻" << std::endl;
        std::cout << "locationBlockUriName:            [" << GRN_BG << locationBlockUriName << RES << "]" << std::endl;
        std::cout << "locationBlockRootDir:            [" << GRN_BG << locationBlockRootDir << RES << "]" << std::endl;

        if (originalUrlPath.size() == 1) {// was: originalUrlPath == "/"
            URLPath_full = parsePath_root(originalUrlPath, location);
        } else if (originalUrlPath[0] == '/') {
            URLPath_full = parsePath_regularCase(originalUrlPath, location);
//            std::cout << RED << "JOYCEEEEEE URLPath_full: " << URLPath_full<< RES << std::endl;
        } else if (originalUrlPath[0] != '/') {
            URLPath_full = parsePath_edgeCase(originalUrlPath, location);
        }
        if (not URLPath_full.empty() || getHttpStatus() == FORBIDDEN) {
            checkRedirection(location->getRedirection());// TODO CHECK IF REDIRECTION IS WORKING and change name to Check
            break;
        }

        std::cout << YEL << "The url path [" << originalUrlPath << "] did not match the current location block [";
        std::cout << location->getLocationUriName() << "] from the config file. ";
        std::cout << "Checking the next locationBlockUriName" << RES << std::endl;
    }
    return URLPath_full;
}

// In case error 404, file not found, it's probably good to not continue after parsePath(), and just close the connection
void Request::parsePath(std::string  const & originalUrlPath) {
    std::string serverBlockDir = getServerData().getRootDirectory();

    std::cout << "originalUrlPath:               [" << GRN_BG << originalUrlPath << RES << "]" << std::endl;
    std::cout << "server block root directory:   [" << GRN_BG << getServerData().getRootDirectory() << RES << "]" << std::endl;
    std::cout << std::endl << GRN << "Starting parsePath() and searching for the correct location block on the config file:" << RES;
    std::cout << std::endl << std::endl;

	if (not originalUrlPath.empty()) {
        // When a request comes in, nginx will first try to match the URI to a specific location block.
        std::string URLPath_full = parsePath_locationMatch(originalUrlPath);
        std::cout << "⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻" << std::endl << std::endl;
        if (URLPath_full.empty()) {
            // Ex.:
            // localhost/_cgi [ok -> internal]
            // localhost/_cgi/file.php [ko -> 404] since the _cgi is not a location (and is internal)
            if (originalUrlPath.find("/_") != std::string::npos) {
                std::cout << YEL << "Internal directory requested, it will be server but no need to search for its"
                                    "location on the config file" << RES << std::endl << std::endl;
                _data.setFileExtention(getExtension(originalUrlPath));
                URLPath_full = serverBlockDir + originalUrlPath;
            } else {
                std::cout << RED << "As the UrlPath did not match any location block, ";
                std::cout << "the server cannot serve any file and will return 404 NOT_FOUND" << RES << std::endl << std::endl;
                if (getHttpStatus() == NO_STATUS) {// todo -> here I think we dont need to check this anymore
                    setHttpStatus(NOT_FOUND);
                }
                _data.setURLPath(originalUrlPath);
                return ;
            }
        }
        _data.setResponseContentType(_data.getFileExtention());
        storeURLPathParts(originalUrlPath, URLPath_full);// TODO: do that only if _data.getRequestMethod() != "POST" ?????

        printPathParts(_data);
        if (getHttpStatus() == NO_STATUS) {
            checkIfPathExists(URLPath_full);
        }
    }
}
