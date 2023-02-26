#include "ServerLocation.hpp"

/** Default constructor */
//ServerLocation::ServerLocation()// todo private???
//    /** Initializing default values for the location block */
//    : _is_location_cgi(false),
//    _root_directory(std::string()),
//    _allow_methods(),
//    _index_file(std::string()),
//    _auto_index(false),
//    _interpreter_path(std::string()),
//    _script_extension(std::string()) {
//    std::cout << CYN << "JOYCE CREATING ServerLocation: " << BACK << std::endl;
//}

/** Overloaded constructor */
ServerLocation::ServerLocation(std::string const & server_root_directory, std::string const & server_index_file)
    /** Initializing default values for the location block */
    : _is_location_cgi(false),
    _root_directory(server_root_directory),
    _index_file(server_index_file),
    _auto_index(false),
    _interpreter_path(std::string()),
    _script_extension(std::string()) {
    _allow_methods.push_back(GET);
    std::cout << CYN << "JOYCE CREATING Overloaded ServerLocation" << BACK << std::endl;
}

/** Destructor */
ServerLocation::~ServerLocation() {
    /** Cleaning default values for the location block */
    _is_location_cgi = false;
    _root_directory = std::string();
    _allow_methods = std::vector<AllowMethods>(NONE);
    _index_file = std::string();
    _auto_index = false;
    _interpreter_path = std::string();
    _script_extension = std::string();
    std::cout << CYN << "JOYCE deleting ServerLocation" << BACK << std::endl;
}

/** #################################### Methods #################################### */

bool ServerLocation::isLocationCgi() const {
    return _is_location_cgi;
}

/** #################################### Getters #################################### */

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

std::string ServerLocation::getInterpreterPath() const {
    return _interpreter_path;
}

std::string ServerLocation::getScriptExtension() const {
    return _script_extension;
}

/** #################################### Setters #################################### */

void ServerLocation::setLocationAsCgi(bool isCgi) {
    /* not mandatory | default: python cgi with a default index.html inside */
    _is_location_cgi = isCgi;
}

bool ServerLocation::setRootDirectory(std::string const & rootDirectory) {
    /* location -> not mandatory | default: $server.root_directory */
    if (not rootDirectory.empty()) {
        PathType type = pathType(rootDirectory);
        if (type == DIRECTORY) {
            _root_directory = addCurrentDirPath(rootDirectory) + rootDirectory;
            return true;
        } else if (type == PATH_TYPE_ERROR) {
            throw ParserException(CONFIG_FILE_ERROR("root_directory", MISSING));
        } else {
            throw ParserException(CONFIG_FILE_ERROR("root_directory", NOT_SUPPORTED));
        }
    } else {
        /* cgi -> mandatory */
        if (isLocationCgi()) {
            throw ParserException(CONFIG_FILE_ERROR("root_directory", MANDATORY));
        }
    }
    return false;
}

bool ServerLocation::setAllowMethods(std::string const & allowMethods) {
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
        return true;
    }
    return false;
}

bool ServerLocation::setIndexFile(std::string const & indexFile) {
    /* location -> not mandatory | default: $server.index_file */
    /* cgi -> not mandatory | default: stays in the same page */
    if (not indexFile.empty()) {
        std::string index_file = isPath(_root_directory, indexFile);

        if (pathType(index_file) == REG_FILE) {
            _index_file = addCurrentDirPath(index_file) + index_file;
            return true;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("index_file", NOT_SUPPORTED));
        }
    }
    return false;
}

bool ServerLocation::setAutoIndex(std::string const & autoIndex) {
    /* not mandatory | default: off */
    if (not autoIndex.empty()) {
        if (autoIndex.find("on") != std::string::npos) {
            _auto_index = true;
        } else if (autoIndex.find("off") != std::string::npos) {
            _auto_index = false;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("auto_index", NOT_SUPPORTED));
        }
        return true;
    }
    return false;
}

bool ServerLocation::setInterpreterPath(std::string const & interpreterPath) {
    /* mandatory */
    if (not interpreterPath.empty()) {
        if (interpreterPath[0] != '/') {
            throw ParserException(CONFIG_FILE_ERROR("interpreter_path", NOT_SUPPORTED));
        } else {
            _interpreter_path = interpreterPath;
            return true;
        }
    } else {
        if (isLocationCgi()) {
            throw ParserException(CONFIG_FILE_ERROR("interpreter_path", MANDATORY));
        }
    }
    return false;
}

bool ServerLocation::setScriptExtension(std::string const & scriptExtension) {
    /* mandatory */
    if (not scriptExtension.empty()) {
        if (scriptExtension == ".py") {
            _script_extension = scriptExtension;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("script_extension", NOT_SUPPORTED));
        }
    } else {
        if (isLocationCgi()) {
            throw ParserException(CONFIG_FILE_ERROR("script_extension", MANDATORY));
        }
    }
    return false;
}
