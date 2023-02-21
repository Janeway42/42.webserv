#include "includes/LocationData.hpp"

/** Default constructor */
LocationData::LocationData()// todo private???
    /** Initializing default values for the location block */
    : _is_location_cgi(false),
    _root_directory(""),
    _allow_methods(),
    _index_file(""),
    _auto_index(false),
    _interpreter_path(""),
    _script_extension("") {
}

/** Overloaded constructor */
LocationData::LocationData(std::string const & server_root_directory, std::string const & server_index_file)
    /** Initializing default values for the location block */
    : _is_location_cgi(false),
    _root_directory(server_root_directory),
    _index_file(server_index_file),
    _auto_index(false),
    _interpreter_path(""),
    _script_extension("") {
    _allow_methods.push_back(GET);
}

/** Destructor */
LocationData::~LocationData() {
    /** Cleaning default values for the location block */
    _is_location_cgi = false;
    _root_directory = std::string();
    _allow_methods = std::vector<AllowMethods>(NONE);
    _index_file = std::string();
    _auto_index = false;
    _interpreter_path = std::string();
    _script_extension = std::string();
}

/** #################################### Methods #################################### */

bool LocationData::isLocationCgi() const {
    return _is_location_cgi;
}

/** #################################### Getters #################################### */

std::string LocationData::getRootDirectory() const {
    return _root_directory;
}

std::vector<AllowMethods> LocationData::getAllowMethods() const {
    return _allow_methods;
}

std::string LocationData::getIndexFile() const {
    return _index_file;
}

bool LocationData::getAutoIndex() const {
    return _auto_index;
}

std::string LocationData::getInterpreterPath() const {
    return _interpreter_path;
}

std::string LocationData::getScriptExtension() const {
    return _script_extension;
}

/** #################################### Setters #################################### */

void LocationData::setLocationAsCgi(bool isCgi) {
    /* not mandatory | default: python cgi with a default index.html inside */
    _is_location_cgi = isCgi;
}

bool LocationData::setRootDirectory(std::string const & rootDirectory) {
    /* location -> not mandatory | default: $server.root_directory */
    std::cout << "JOYCE rootDirectory: " << rootDirectory << std::endl;

    if (not rootDirectory.empty()) {
        PathType type = pathType(rootDirectory);
        if (type == DIRECTORY) {
            _root_directory = addCurrentDirPath(rootDirectory) + rootDirectory;
            std::cout << "JOYCE _root_directory: " << _root_directory << std::endl;

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

bool LocationData::setAllowMethods(std::string const & allowMethods) {
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

bool LocationData::setIndexFile(std::string const & indexFile) {
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

bool LocationData::setAutoIndex(std::string const & autoIndex) {
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

bool LocationData::setInterpreterPath(std::string const & interpreterPath) {
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

bool LocationData::setScriptExtension(std::string const & scriptExtension) {
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
