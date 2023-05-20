#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem> // temp, to test current directory
#include "Parser.hpp"
#include "RequestParser.hpp"

void Request::runExecve(char *ENV[], char *args[], struct kevent event) {
	//std::cout << BLU << "START runExeve\n" << RES;
	// (void)event;
	// (void)ENV;

	Request *storage = (Request *)event.udata;

	if (_cgi.createPipes(storage->getKq(), event) == 1)
		return ;

	int ret = 0;
	pid_t		retFork;

	retFork = fork();
    if (retFork < 0)
        std::cout << RED << "Error: Fork failed\n" << RES;
    else if (retFork == 0) { // CHILD
        std::cout << CYN << "Start CHILD execve()\n" << RES;
        std::cout << CYN << "Interpreter Path: " << storage->getInterpreterPath() << RES << std::endl;

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

        // Best practice to ensure the script to find correct relative paths, if needed
        chdir(storage->getRequestData().getURLPathFirstPart().c_str());
        args[0] = const_cast<char*>(storage->getInterpreterPath().c_str());
		ret = execve(args[0], args, ENV);
        // if (ret == -1) {
        std::cerr << RED << "Error: Execve failed: " << ret << "\n" << RES;
        delete storage;
        sleep(31);  // todo, can this be replaced with macro TIMEOUT?
        exit(1);
		// }
	}
	else {				// PARENT
		std::cerr << "    Start Parent\n";
		close(_cgi.getPipeCgiOut_1());
		close(_cgi.getPipeCgiIn_0());
	}
}

void Request::callCGI(struct kevent event) {
	std::cout << CYN << "Start callCGI, cgi path: " << _data.getURLPath_full() << "\n" << RES;

	// Declare all necessary variables
	std::string request_method	= "REQUEST_METHOD=";
	std::string server_protocol	= "SERVER_PROTOCOL=HTTP/1.1";
	std::string content_type	= "CONTENT_TYPE=";
	std::string content_length	= "CONTENT_LENGTH=";
	std::string query_string	= "QUERY_STRING=";
	std::string server_name		= "SERVER_NAME=";
	std::string comspec			= "COMSPEC=";
	std::string info_path		= "PATH_INFO=";
	std::string upload_path		= "UPLOAD_DIR=";
	std::string cookie		    = "COOKIE=";

	// Convert length to string
	std::stringstream ssContLen;
	ssContLen << _data.getRequestContentLength();

	// Declare a vector and fill it with variables, with attached =values
	std::vector<std::string> temp;
	temp.push_back(request_method.append(allowMethodsToString(_data.getRequestMethod())));
	temp.push_back(server_protocol);
	temp.push_back(content_type.append(_data.getRequestContentType()));
	temp.push_back(content_length.append(ssContLen.str()));
	temp.push_back(query_string.append(_data.getQueryString()));
	temp.push_back(server_name.append(getServerData().getServerName()));
	temp.push_back(comspec.append(""));
	temp.push_back(info_path.append(getServerData().getUploadDirectoryName()));
	temp.push_back(cookie.append(_data.getRequestCookie()));

    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) == NULL)
        std::cout << RED "Error in callCGI: getcwd() failed\n" RES;
    std::string str = getServerData().getUploadDirectory();
    std::string uploadDir = buffer;
    uploadDir.append(str, 1);
	temp.push_back(upload_path.append(uploadDir));

	char **env = new char*[temp.size() + 1];
	size_t i = 0;
	for (i = 0; i < temp.size(); i++) {
		env[i] = new char[temp[i].length() + 1];
		strcpy(env[i], temp[i].c_str());
	}
	env[i] = nullptr;

	// Just for printing
	std::cout << GRN "STORED ENV:\n" RES;
	for (i = 0; env[i]; i++) {
	  std::cout << "    " << i+1 << " " << env[i] << std::endl;
	}

	char *args[3];
    std::string tempPath = _data.getURLPathLastPart();  // must be only the script name, because the CWD is changed to CGI folder
	char *path = (char *)tempPath.c_str();	            //  ie: "./resources/cgi/python_cgi_GET.py"
	args[1] = path;
	args[2] = NULL;

	runExecve(env, args, event);

	//std::cout << "Stored body from CGI: [\n" << BLU << _data.getCgiBody() << RES << "]\n";
	for (size_t j = 0; j < temp.size(); j++) {      // Cleanup
		delete env[j];
	}
	delete[] env;
}

////////////////////////////////////////////////////////////////

/* 	Split string at '&' and store each line into vector<>
	Then split each line in vector into map<> key:value */
std::map<std::string, std::string> Request::storeFormData(std::string queryString)
{
	//std::cout << CYN << "Start store form data()\n" << RES;
	//std::cout << GRN << "    BODY:        ["   << _body << "]\n" << RES;
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
		std::stringstream iss2(*it);
		std::getline(iss2, key, '=') >> val;
		formDataMap[key] = val;
	}
	_data.setFormData(formDataMap);
	return (formDataMap);
}

void Request::storeURLPathParts(std::string const & originalUrlPath, std::string const & URLPath_full) {
    std::cout << CYN << "Start storeURLPathParts(). URLPath_full: [" << URLPath_full << "]\n" << RES;
    std::string::size_type posLastSlash = URLPath_full.find_last_of('/');

    // If there is query '?' string, store it
    std::string::size_type positionQuestionMark = originalUrlPath.find_first_of('?');
	if (positionQuestionMark != std::string::npos) {
        // Skip the '?' in the path and store the Query string
        std::string	queryString = originalUrlPath.substr(positionQuestionMark + 1);
        if (_data.getRequestMethod() == GET) {
            _data.setQueryString(queryString);
            std::cout << "queryString:                   [" << GRN << _data.getQueryString() << RES << "]" << std::endl;
        }
        storeFormData(queryString);	// the key:value map is now only used to print them out, 
    }                               // but the cgi script handles the queryString automatically

    _data.setURLPath(originalUrlPath);
    _data.setURLPath_full(URLPath_full);
	_data.setPathFirstPart(URLPath_full.substr(0, posLastSlash + 1));
	_data.setPathLastPart(URLPath_full.substr(posLastSlash + 1));
}



void Request::checkIfPathExists(std::string const & URLPath_full) {
    std::cout << CYN << "Start CheckIfPathExists(), URLPath_full [" << URLPath_full << "] \n" << RES;

    // Here at the end URLPath_full will always be a file, either because the request was a file, or because
    // the correct index file was appended to it (from location or server block, the config file parser decided already)
    PathType type = pathType(URLPath_full);

    if (type == PATH_TYPE_ERROR) {   // added jaka
        setHttpStatus(FORBIDDEN);    // added jaka
        return;
    }
    if (type != REG_FILE) {
        std::cout << RED << std::endl << "Error: file [" << RES << URLPath_full << RED;
        std::cout << "] was not found. Returning 404 NOT FOUND" << RES << std::endl << std::endl;
        setHttpStatus(NOT_FOUND);
    } else if (type == REG_FILE) {
        std::cout << GRN << "Path " << RES << URLPath_full << GRN << " exists" << RES << std::endl << std::endl;
        setHttpStatus(OK);
    }
}


static void printPathParts(RequestData reqData) {
	std::cout << std::endl;
	std::cout << "URL Path:        [" << PUR << reqData.getURLPath() << RES << "]\n";
	std::cout << "Full URI Path:   [" << PUR << reqData.getURLPath_full() << RES << "]\n";
	std::cout << "Path first part: [" << PUR << reqData.getURLPathFirstPart() << RES << "]\n";
	std::cout << "Path LAST part:  [" << PUR << reqData.getURLPathLastPart() << RES << "]\n";
	std::cout << "File extension:  [" << PUR << reqData.getFileExtension() << RES << "]\n";

    // To log if necessary
    //std::cout << YEL "Body:\n" RES;
	//std::copy(reqData.getBody().begin(), reqData.getBody().end(), std::ostream_iterator<uint8_t>(std::cout));
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
        std::cout  << "There is no extension in the file" << std::endl;
    }
    return extension;
}

std::string Request::parsePath_cgi(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location, std::string const & file_cgi) {
    // Ex.: localhost:8080/cgi/python_cgi_GET.py?street=test&city=test+city
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();
    setInterpreterPath(location->getInterpreterPath());

    // blocks down below are just for logging
    if (not _data.getFileExtension().empty()) {
        std::cout << "Deleting file name from it so the extension can be matched against the location block uri extension name. Extension: ";
        std::cout << GRN << _data.getFileExtension() << RES << std::endl;
    }
    if (_data.getRequestMethod() == GET) {
        std::cout << GRN << "There is GET Form data" << RES << std::endl;
    } else if (_data.getRequestMethod() == POST) {
        std::cout << GRN << "There is POST Form data" << RES << std::endl;
    } else if (_data.getRequestMethod() == DELETE) {
        std::cout << GRN << "There is DELETE Form data" << RES << std::endl;
    }

    /* If the url is a script file, the match will be done between the extension of it, against the location uri
     * ex: url localhost/cgi/script.py -> the .py part will be checked against a location uri */
    if (_data.getFileExtension() == locationBlockUriName) {
        std::cout << GRN << "setIsCgi(true)" << RES << std::endl;
        getCgiData().setIsCgi(true);
        std::cout << BLU << "cgi location block for [" << RES BLU_BG << originalUrlPath << RES;
        std::cout << BLU << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES << "]" << std::endl;
        std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;
        return locationBlockRootDir + file_cgi;
    }
    return std::string();
}

std::string Request::parsePath_dir(std::string const & originalUrlPath, std::string const & firstDirectoryFromUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    // Ex.: localhost:8080/test/ or localhost:8080/test/sub/
    // Obs.: A directory URI always have a / at the end (otherwise it is a file)
    if (originalUrlPath.find('?') == std::string::npos) {
        std::cout << BLU << "No '?' found and originalUrlPath has no GET-Form data" << RES << std::endl;
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();

        // Here we are matching the first directory of the URI with the locationBlockUriName + / (since it comes
        // without it from the config file)
        if (firstDirectoryFromUrlPath == (locationBlockUriName + '/') && pathType(locationBlockRootDir + originalUrlPath) == DIRECTORY) {
            std::cout << "Path is a directory." << std::endl << RES;
            _data.setIsFolder(true);
            
            std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES;
            std::cout << BLU << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES << "]" << std::endl;
            std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;

            // If autoindex is off and no index file, return 403 Forbidden, else return folder content
            if (pathType(locationBlockRootDir + originalUrlPath + '/' + location->getIndexFile()) != REG_FILE) {
                _data.setAutoIndex(location->getAutoIndex());
                if (not location->getAutoIndex()) {
                    std::cout << GRY << "Auto index is off and no index file found on the location [";
                    std::cout << locationBlockRootDir + originalUrlPath << "]. Returning 403 Forbidden" << RES << std::endl;
                    setHttpStatus(FORBIDDEN);
                } else {
                    std::cout << GRY << "Auto index is on and no index file found on the location [";
                    std::cout << locationBlockRootDir + originalUrlPath << "]. Folder content will be served" << RES << std::endl;
                }
            } else {
                _data.setFileExtension(getExtension(location->getIndexFile()));
            }
            return locationBlockRootDir + originalUrlPath + '/' + location->getIndexFile();
        } else {
            std::cout << locationBlockRootDir + originalUrlPath << " is NOT a directory"<< std::endl;
        }
    }
    return std::string();
}

std::string Request::parsePath_file(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    // Ex.: localhost:8080/favicon.ico or localhost:8080/cgi/cgi_index.html
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();
    if (originalUrlPath.find('?') == std::string::npos && _data.getRequestMethod() == GET
        && locationBlockUriName != ".py" && locationBlockUriName != ".php") {

        std::cout << BLU << "No '?' found, so no cgi root_directory needed" << RES << std::endl;
        std::cout << "Path is a file. ";

        std::string firstDirectoryFromUrlPath = std::string();
        std::string::size_type secondSlash = originalUrlPath.find_first_of('/', 1);
        if (secondSlash != std::string::npos) {
            firstDirectoryFromUrlPath = originalUrlPath.substr(0, secondSlash + 1);
        } else {
            // If there is only one / then it has to keep it and tricky it to match the / location block (if it exists)
            firstDirectoryFromUrlPath = "//";
        }

        // The if block down below is just for logging
        if (not firstDirectoryFromUrlPath.empty()) {
            std::cout << "Deleting file from it so it can be matched against the location block uri name. Path: ";
            std::cout << firstDirectoryFromUrlPath << std::endl;
        }

        /* If the url is a file, the match will be done between the directory where the file is, against the location uri
         * ex: url localhost/cgi/cgi_index.html -> the /cgi part will be checked against a location uri */
        if (firstDirectoryFromUrlPath == (locationBlockUriName + '/')) {
            std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES;
            std::cout << BLU << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES << "]" << std::endl;
            std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;

            return locationBlockRootDir + originalUrlPath;
        }
    }
    // Script file for POST, with no query ('?')
    // Ex.: localhost:8080/python_POST.py or localhost:8080/cgi/python_cgi_POST_upload.py or localhost:8080/py/cgi_delete.py
    else if (originalUrlPath.find('?') == std::string::npos) {
        std::cout << "Path is a script file.\n";
        return parsePath_cgi(originalUrlPath, location, originalUrlPath.substr(originalUrlPath.find_last_of('/')));
    }
    return std::string();
}

std::string Request::parsePath_regularCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();

    if (originalUrlPath != "/") {
        std::string::size_type lastSlash = originalUrlPath.find_last_of('/');

        // -------------------------------------------------------------------------------------------------- FILE
        if (lastSlash != std::string::npos) {
            std::string scriptFile = originalUrlPath.substr(lastSlash);
            if (not scriptFile.empty() &&
                    (scriptFile.find(".py") == std::string::npos
                    && scriptFile.find(".php") == std::string::npos)) {
                scriptFile = "";
            }
            std::cout << RED << "scriptFile (if it is a script): " << scriptFile << RES << std::endl << RES;
            std::cout << RED << "path to be searched: " << locationBlockRootDir + (scriptFile == "" ? originalUrlPath : scriptFile) << RES << std::endl << RES;

            if (pathType(locationBlockRootDir + (scriptFile == "" ? originalUrlPath : scriptFile)) == REG_FILE) {
                _data.setFileExtension(getExtension(originalUrlPath));
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
            _data.setFileExtension(getExtension(originalUrlPath));
            if (not fileCgi.empty() && pathType(locationBlockRootDir + fileCgi) == REG_FILE) {
                std::string URLPath_full_cgi = parsePath_cgi(originalUrlPath, location, fileCgi);
                if (not URLPath_full_cgi.empty()) {
                    return URLPath_full_cgi;
                }
            }
        }
        // -------------------------------------------------------------------------------------------------- DIRECTORY
        //if (originalUrlPath.at(originalUrlPath.size() - 1) == '/') {
            std::string firstDirectoryFromUrlPath;
            if (lastSlash > 0) {
                // If lastSlash is not zero, then we are dealing with subdirectories (i.e.: /dir/dir1/ex/), which means
                // we want to match the first directory from the path to the first directory of locationBlockRootDir
                // E.g.: path /dir/dir1/ will be /dir/ and will have to match $location.root_directory/dir location name
                std::string::size_type secondSlash = originalUrlPath.find_first_of('/', 1);
                if (secondSlash != std::string::npos) {
                    firstDirectoryFromUrlPath = originalUrlPath.substr(0, secondSlash + 1);
                }
            }
            std::string URLPath_full_dir = parsePath_dir(originalUrlPath, firstDirectoryFromUrlPath, location);
            if (not URLPath_full_dir.empty()) {
                return URLPath_full_dir;
            }
        //}
    }
    return std::string();
}

std::string Request::parsePath_root(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location) {
    std::string locationBlockUriName = location->getLocationUriName();
    std::string locationBlockRootDir = location->getRootDirectory();
    /* Ex.: localhost:8080
     * If the URI is "/", nginx will look for a file named "index.html" in the root directory of the server block.
     * If it finds the file, it will be served to the client as the response to the request. If it doesn't find
     * the file, nginx will return a "404 Not Found" error to the client (at the end of checkIfPathCanBeServed()) */
    if (originalUrlPath == locationBlockUriName) {
        std::cout << "Path is a directory." << std::endl << RES;
        _data.setIsFolder(true);
        std::cout << BLU << "location block for [" << RES BLU_BG << originalUrlPath << RES;
        std::cout << BLU << "] exists on config file as [" << RES BLU_BG << locationBlockUriName << RES << "]" << std::endl;
        std::cout << BLU << "Its root_directory [" << locationBlockRootDir << "] and configuration will be used" << RES << std::endl;

        // If autoindex is off and no index file, return 403 Forbidden, else return folder content
        if (pathType(locationBlockRootDir + locationBlockUriName + '/' + location->getIndexFile()) != REG_FILE) {
            _data.setAutoIndex(location->getAutoIndex());
            if (not location->getAutoIndex() ) {
                std::cout << RED << "Auto index is off and no file found on the location [";
                std::cout << locationBlockRootDir + locationBlockUriName << "]. Returning 403 Forbidden" << RES << std::endl;
                setHttpStatus(FORBIDDEN);
            }
        } else {
            _data.setFileExtension(getExtension(location->getIndexFile()));
        }
        return locationBlockRootDir + locationBlockUriName + location->getIndexFile();
    }
    return std::string();
}

// receiving a copy of originalUrlPath, so it can be modified in case we have a redirection on the location block
void Request::checkRedirection(std::string getRedirection) {
    if (not getRedirection.empty()) {
        std::cout << BLU << "Redirection found: " << getRedirection << RES << std::endl;
        std::cout << BLU << "A 301 response response will be sent, with the redirection url on it!" << RES << std::endl;
        setHttpStatus(MOVE_PERMANENTLY);
        setRedirection(getRedirection);
    }
}

void Request::checkMethods(std::vector<AllowMethods> const & methods) {
    std::vector<AllowMethods>::const_iterator config_file_method;
    std::cout << GRY << "Method allowed from the config file: " << RES;
    bool methodsMatched = false;
    for (config_file_method = methods.cbegin(); config_file_method != methods.cend(); ++config_file_method) {
        std::cout << GRY << allowMethodsToString(*config_file_method) << " " << RES;
        if (*config_file_method == DELETE) {
            setDeleteIsAllowed(true);
        }
        if (_data.getRequestMethod() == *config_file_method) {
            methodsMatched = true;
        }
    }
    if (not methodsMatched) {
        setHttpStatus(METHOD_NOT_ALLOWED);
        std::cout << std::endl << RED << "Location does not accept the Method - 405 Method not allowed will be returned"
            << RES << std::endl;
    } else {
        std::cout << std::endl << GRN << "Location accepts the ";
        std::cout << allowMethodsToString(_data.getRequestMethod()) << " method, continuing..." << RES << std::endl;
    }
}

std::string Request::parsePath_locationMatch(std::string const & originalUrlPath) {
    std::string URLPath_full = std::string();

    std::vector<ServerLocation>::const_iterator location = getServerData().getLocationBlocks().cbegin();
    for (; location != getServerData().getLocationBlocks().cend(); ++location) {
        std::string locationBlockUriName = location->getLocationUriName();
        std::string locationBlockRootDir = location->getRootDirectory();
        std::cout << "⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻" << std::endl;
        std::cout << "locationBlockUriName:            [" << PUR << locationBlockUriName << RES << "]" << std::endl;
        std::cout << "locationBlockRootDir:            [" << PUR << locationBlockRootDir << RES << "]" << std::endl;

        if (originalUrlPath.size() == 1) {// was: originalUrlPath == "/"
            URLPath_full = parsePath_root(originalUrlPath, location);
        } else if (originalUrlPath[0] == '/') {
            URLPath_full = parsePath_regularCase(originalUrlPath, location);
        }
        if (not URLPath_full.empty() || getHttpStatus() == FORBIDDEN) {
			if (not location->getLocationCookie().empty()) {
                _data.setRequestSetCookie(location->getLocationCookie());
                // std::cout << "Cookies from the location: " << location->getLocationSetCookie() << std::endl;
                std::cout << "Cookies from the location: " << location->getLocationCookie() << std::endl;
            }
            std::cout << "Method from the request: " << allowMethodsToString(_data.getRequestMethod()) << std::endl;
            checkMethods(location->getAllowMethods());
            checkRedirection(location->getRedirection());
            break;
        }
        // if (not location->getRedirection().empty()) {
        //     //checkRedirection(location->getRedirection());
        //     break;
        // }

        std::cout << YEL << "The url path [" << originalUrlPath << "] did not match the current location block [";
        std::cout << locationBlockUriName << "] from the config file. ";
        std::cout << "\nChecking the next locationBlockUriName" << RES << std::endl;
    }
    return URLPath_full;
}

// In case error 404, file not found, it's probably good to not continue after checkIfPathCanBeServed(), and just close the connection
void Request::checkIfPathCanBeServed(std::string  const & originalUrlPath) {
    std::string serverBlockDir = getServerData().getRootDirectory();

    std::cout << "originalUrlPath:               [" << PUR << originalUrlPath << RES << "]" << std::endl;
    std::cout << "server block root directory:   [" << PUR << getServerData().getRootDirectory() << RES << "]" << std::endl;
    std::cout << std::endl << GRN << "Starting checkIfPathCanBeServed() and searching for the correct location block on the config file:" << RES;
    std::cout << std::endl << std::endl;

	if (not originalUrlPath.empty()) {
        // When a request comes in, nginx will first try to match the URI to a specific location block.
        std::string URLPath_full = parsePath_locationMatch(originalUrlPath);
        std::cout << "⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻ ⎻" << std::endl << std::endl;
        if (URLPath_full.empty()) {
            // Ex.: localhost/_server_default_status [ok -> internal]
            //      localhost/cgi/file.php [ko -> 404] since the cgi can be a location (and is not internal)
            if (originalUrlPath.find("/_") != std::string::npos) {
                std::cout << YEL << "Internal directory requested, it will be served but no need to search for its "
                                    "location on the config file" << RES << std::endl << std::endl;
                _data.setFileExtension(getExtension(originalUrlPath));
                URLPath_full = serverBlockDir + originalUrlPath;
            } else {
                std::cout << RED << "As the UrlPath did not match any location block, ";
                std::cout << "the server cannot serve any file and will return 404 NOT_FOUND" << RES << std::endl << std::endl;
                if (getHttpStatus() == NO_STATUS) {
                    setHttpStatus(NOT_FOUND);
                }
                _data.setURLPath(originalUrlPath);
                return ;
            }
        }
        _data.setResponseContentType(_data.getFileExtension());
        storeURLPathParts(originalUrlPath, URLPath_full);// TODO: do that only if _data.getRequestMethod() != POST ?????

        printPathParts(_data);
        if (getHttpStatus() == NO_STATUS) {
            checkIfPathExists(URLPath_full);
        }
        std::cout << "HTTP status after checkIfPathCanBeServed(): " << getHttpStatus() << std::endl;
    }
}
