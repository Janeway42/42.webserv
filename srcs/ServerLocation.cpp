#include "ServerLocation.hpp"

/** Default constructor */
//ServerLocation::ServerLocation()// todo private???
//    /** Initializing default values for the location block */
//    : _is_location_cgi(false),
//    _location_uri_name(std::string()),
//    _location_cgi_extension(std::string()),
//    _root_directory(std::string()),
//    _allow_methods(),
//    _index_file(std::string()),
//    _auto_index(false),
//    _redirection(std::string()),
//    _interpreter_path(std::string()),
//    useServerBlockIndexFile(false) {
//    std::cout << CYN << "ServerLocation Default constructor" << RES << std::endl;
//}

/** Overloaded constructor */
ServerLocation::ServerLocation(std::string const & server_root_directory, std::string const & server_index_file)
    /** Initializing default values for the location block */
    : _is_location_cgi(false),
    _location_uri_name(std::string()),
    _location_cgi_extension(std::string()),
    _root_directory(server_root_directory),
    _index_file(server_index_file),
    _auto_index(false),
    _redirection(std::string()),
    _interpreter_path(std::string()),
    useServerBlockIndexFile(false) {
    _allow_methods.push_back(NONE);
	_locationCookie = "";
//    std::cout << CYN << "ServerLocation Overloaded constructor" << RES << std::endl;
}

/** Destructor */
ServerLocation::~ServerLocation() {
    /** Cleaning default values for the location block */
    _is_location_cgi = false;
    _location_uri_name.clear();
    _location_cgi_extension.clear();
    _root_directory.clear();
    _allow_methods = std::vector<AllowMethods>(NONE);
    _index_file.clear();
    _auto_index = false;
    _redirection.clear();
    _interpreter_path.clear();
    useServerBlockIndexFile = false;
	_locationCookie.clear();
//    std::cout << CYN << "ServerLocation Destructor" << RES << std::endl;
}

/** #################################### Methods #################################### */

bool ServerLocation::isLocationCgi() const {
    return _is_location_cgi;
}

/** #################################### Getters #################################### */

std::string ServerLocation::getLocationUriName() const {
    return _location_uri_name;
}

std::string ServerLocation::getLocationCgiExtension() const {
    return _location_cgi_extension;
}

std::string ServerLocation::getRootDirectory() const {
    return _root_directory;
}

std::vector<AllowMethods> ServerLocation::getAllowMethods() const {
    return _allow_methods;
}

std::string ServerLocation::getIndexFile() const {
    return _index_file;
}

bool ServerLocation::getAutoIndex() const {
    return _auto_index;
}

std::string ServerLocation::getRedirection() const {
    return _redirection;
}

std::string ServerLocation::getInterpreterPath() const {
    return _interpreter_path;
}

std::string ServerLocation::getLocationCookie() const {
	return _locationCookie;
 }
/** #################################### Setters #################################### */

void ServerLocation::setLocationAsCgi(bool isCgi) {
    /* not mandatory */
    _is_location_cgi = isCgi;
}

void ServerLocation::setLocation(std::string const & location) {
    /* mandatory | if request contains an uri directory path, it can be made accessible by making it a location block */
    if (not location.empty()) {
        std::string locationName = location;
        // deleting the last / in case it is added
        if (location[location.size() - 1] == '/' && location.size() > 1) {
            locationName = location.substr(0, location.size() - 1);
        }
        if (isLocationCgi()) {
            if (location[0] == '.') {
                _location_cgi_extension = locationName;// todo maybe not needed
                _location_uri_name = locationName;
                return ;
            } else {
                throw ParserException(CONFIG_FILE_ERROR("location block cgi extension", NOT_SUPPORTED));
            }
        } else {
            std::string ::const_iterator it;
            for (it = location.cbegin(); it != location.cend(); it++) {
                if (*it < 47 || *it > 126) {
                    throw ParserException(CONFIG_FILE_ERROR("location block value", NOT_SUPPORTED));
                }
            }
            _location_uri_name = locationName;
            return ;
        }
    }
    throw ParserException(CONFIG_FILE_ERROR("location block value", MANDATORY));
}

void ServerLocation::setRootDirectory(std::string const & rootDirectory) {
    /* mandatory | default: $server.$root_directory.$uri_path */
    /* CGI: mandatory | default: $server.root_directory */
    if (not rootDirectory.empty() && rootDirectory != "/" && rootDirectory != "./") {
        std::string rootDirectoryTemp = rootDirectory;
        if (rootDirectory.at(rootDirectory.size() - 1) == '/') {
            rootDirectoryTemp = rootDirectory.substr(0, rootDirectory.size() - 1);
        }
        if (isLocationCgi()) {
            if (pathType(addCurrentDirPath(rootDirectoryTemp) + rootDirectoryTemp) == DIRECTORY) {
                _root_directory = addCurrentDirPath(rootDirectoryTemp) + rootDirectoryTemp;
            } else {
                throw ParserException(CONFIG_FILE_ERROR("cgi root_directory", MISSING));
            }
        } else {
            std::string root_directory = addRootDirectoryPath(_root_directory, rootDirectoryTemp);
            if (pathType(root_directory) == DIRECTORY) {
                _root_directory = addCurrentDirPath(rootDirectoryTemp) + rootDirectoryTemp;
            } else {
                throw ParserException(CONFIG_FILE_ERROR("root_directory", MISSING));
            }
        }
    }
    // If it is an empty location block or just a / or ./ (that is not a cgi one): default is $server.$root_directory/$uri_path
//    else {//} if (rootDirectory == "/" || rootDirectory == "./") {
//        // If it's a cgi script location, the default is $server.$root_directory (which is already set on constructor)
//        if (not isLocationCgi()) {
//            std::string locationRootDir = _root_directory;
//            if (pathType(locationRootDir) != PATH_TYPE_ERROR) {
//                _root_directory = locationRootDir;
//            } else {
//                throw ParserException(CONFIG_FILE_ERROR("root_directory", NOT_SUPPORTED));
//            }
//        }
//    }
//    else {
//        throw ParserException(CONFIG_FILE_ERROR("root_directory", MANDATORY));
//    }
}

void ServerLocation::setAllowMethods(std::string const & allowMethods) {
    /* not mandatory | default: GET */
    if (not allowMethods.empty()) {
        std::vector<AllowMethods> locationAllowMethods;
        if (allowMethods.find("GET") != std::string::npos) {
            locationAllowMethods.push_back(GET);
        }
        if (allowMethods.find("POST") != std::string::npos) {
            locationAllowMethods.push_back(POST);
        }
        if (allowMethods.find("DELETE") != std::string::npos) {
            locationAllowMethods.push_back(DELETE);
        }
        std::string::size_type numberOfSpaces = 0;
        for (std::string::size_type it = 0; allowMethods[it] ; it++) {
            if (isSpace(allowMethods[it])) {
                numberOfSpaces++;
            }
        }
        if (locationAllowMethods.size() == numberOfSpaces) {
            throw ParserException(CONFIG_FILE_ERROR("allow_methods", NOT_SUPPORTED));
        }
        _allow_methods = locationAllowMethods;
    }
}

void ServerLocation::setIndexFile(std::string const & indexFile) {
    /* not mandatory | default: index_file */
    /* cgi -> not mandatory | default: stays in the same html page */
    if (not indexFile.empty()) {
        std::string index_file = addRootDirectoryPath(_root_directory, _location_uri_name.substr(1) + "/" + indexFile);
        // doesn't contain regexp (regular expressions), wildcards or full/relative path
        if (indexFile.find('/') == std::string::npos) {
            if (pathType(index_file) == REG_FILE) {
                /* if the index_file (with the root_directory added to it) exists and is a regular file, it can be added
                 * to the _index_file private variable (without the root_directory added to it, since for a location, if
                 * auto_index is on, this same index_file would be used for the subdirectories) */
                _index_file = indexFile;
            }
        } else {
            throw ParserException(CONFIG_FILE_ERROR("index_file", NOT_SUPPORTED));
        }
    }
}

void ServerLocation::setAutoIndex(std::string const & autoIndex) {
    /* not mandatory | default: off */
    if (not autoIndex.empty()) {
        if (autoIndex.find("on") != std::string::npos) {
            _auto_index = true;
        } else if (autoIndex.find("off") != std::string::npos) {
            _auto_index = false;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("auto_index", NOT_SUPPORTED));
        }
    }
}

void ServerLocation::setRedirection(std::string const & redirection) {
    /* not mandatory | default: no redirection (empty) */
    if (not redirection.empty()) {
        if (redirection.find("http://") != std::string::npos) {
            _redirection = redirection;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("redirection", NOT_SUPPORTED));
        }
    }
}

void ServerLocation::setInterpreterPath(std::string const & interpreterPath) {
    /* mandatory */
    if (not interpreterPath.empty()) {
        if (interpreterPath[0] != '/') {
            throw ParserException(CONFIG_FILE_ERROR("interpreter_path", NOT_SUPPORTED));
        } else {
            _interpreter_path = interpreterPath;
        }
    } else {
        if (isLocationCgi()) {
            throw ParserException(CONFIG_FILE_ERROR("interpreter_path", MANDATORY));
        }
    }
}

void ServerLocation::setLocationCookie(std::string const & cookie)
{
	if (not cookie.empty()){
		if (cookie.find("cookie=") != std::string::npos)
		{
			std::string temp = cookie.substr(cookie.find("cookie="));
			_locationCookie = temp;
		}
	}
}
