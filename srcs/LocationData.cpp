#include "includes/LocationData.hpp"

//namespace data {
/** Default constructor */
LocationData::LocationData()// todo private???
    /** Initializing default values for the location block */
    : _is_location_cgi(false),
    _root_directory(std::string()),
    _allow_methods(std::vector<AllowMethods>()),
    _index_file(std::string()),
    _auto_index(false) {
}

/** Overloaded constructor */
LocationData::LocationData(std::string const & server_root_directory, std::string const & server_index_file)
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

bool LocationData::is_location_cgi() const {
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
        //todo add error handling
    _is_location_cgi = isCgi;
}

void LocationData::setRootDirectory(std::string const & rootDirectory) {
    //todo add error handling
    _root_directory = rootDirectory;
}

void LocationData::setAllowMethods(const std::vector<AllowMethods> & allowMethods) {
    //todo add error handling
    _allow_methods = allowMethods;
}

void LocationData::setIndexFile(std::string const & indexFile) {
    //todo add error handling
    _index_file = indexFile;
}

void LocationData::setAutoIndex(bool autoIndex) {
    //todo add error handling
    _auto_index = autoIndex;
}

void LocationData::setInterpreterPath(std::string const & interpreterPath) {
    //todo add error handling
    _interpreter_path = interpreterPath;
}

void LocationData::setScriptExtension(std::string const & scriptExtension) {
    //todo add error handling
    _script_extension = scriptExtension;
}
//} // data

