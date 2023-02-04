#include "includes/LocationData.hpp"

namespace data {
/** Default constructor */
//Location::Location()// todo private??? I made it deleted so it cant be used
//    /** Initializing default values for the location block */
//    : _is_location_cgi(false),
//    _root_directory(std::string()),
//    _allow_methods(std::vector<AllowMethods>()),
//    _index_file(std::string()),
//    _auto_index(false) {
//}

/** Overloaded constructor */
Location::Location(std::string const & server_root_directory, std::string const & server_index_file)
    /** Initializing default values for the location block */
    : _is_location_cgi(false),
    _root_directory(server_root_directory),
    _allow_methods(std::vector<AllowMethods>(GET)),
    _index_file(server_index_file),
    _auto_index(false),
    _interpreter_path(std::string()),
    _script_extension(std::string()) {
}

/** Destructor */
Location::~Location() {
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

bool Location::is_location_cgi() const {
    return _is_location_cgi;
}

/** #################################### Getters #################################### */

std::string Location::getRootDirectory() const {
    return _root_directory;
}

std::vector<AllowMethods> Location::getAllowMethods() const {
    return _allow_methods;
}

std::string Location::getIndexFile() const {
    return _index_file;
}

bool Location::getAutoIndex() const {
    return _auto_index;
}

std::string Location::getInterpreterPath() const {
    return _interpreter_path;
}

std::string Location::getScriptExtension() const {
    return _script_extension;
}

/** #################################### Setters #################################### */

void Location::setLocationAsCgi(bool isCgi) {
        //todo add error handling
    _is_location_cgi = isCgi;
}

void Location::setRootDirectory(std::string const & rootDirectory) {
    //todo add error handling
    _root_directory = rootDirectory;
}

void Location::setAllowMethods(const std::vector<AllowMethods> & allowMethods) {
    //todo add error handling
    _allow_methods = allowMethods;
}

void Location::setIndexFile(std::string const & indexFile) {
    //todo add error handling
    _index_file = indexFile;
}

void Location::setAutoIndex(bool autoIndex) {
    //todo add error handling
    _auto_index = autoIndex;
}

void Location::setInterpreterPath(std::string const & interpreterPath) {
    //todo add error handling
    _interpreter_path = interpreterPath;
}

void Location::setScriptExtension(std::string const & scriptExtension) {
    //todo add error handling
    _script_extension = scriptExtension;
}
} // data

