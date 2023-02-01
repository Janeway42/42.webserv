#include "includes/LocationData.hpp"

namespace data {
/** Default constructor */ //private???
Location::Location() {
    /** Initializing default values for the location block */
    _location_block_counter = 0;
    _root_directory = std::string();
    _allow_methods = std::vector<AllowMethods>();
    _index_file = std::string();
    _auto_index = false;
}

/** Overloaded constructor */
Location::Location(std::string const & server_root_directory, std::string const & server_index_file) {
    /** Initializing default values for the location block */
    _location_block_counter = 0;
    _root_directory = server_root_directory;
    _allow_methods = std::vector<AllowMethods>(GET);
    _index_file = server_index_file;
    _auto_index = false;
    _interpreter_path = std::string();
    _script_extension = std::string();
}

/** Destructor */
Location::~Location() {
    /** Cleaning default values for the location block */
    _location_block_counter = 0;
    _root_directory = std::string();
    _allow_methods = std::vector<AllowMethods>(NONE);
    _index_file = std::string();
    _auto_index = false;
    _interpreter_path = std::string();
    _script_extension = std::string();
}

/** #################################### Methods #################################### */

unsigned short Location::locationBlockCounter() {
    return _location_block_counter + 1;
}

/** #################################### Getters #################################### */

unsigned short Location::getLocationBlockCounter() const {
    return _location_block_counter;
}

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

void Location::setRootDirectory(std::string const & rootDirectory) {
    _root_directory = rootDirectory;
}

void Location::setAllowMethods(const std::vector<AllowMethods> & allowMethods) {
    _allow_methods = allowMethods;
}

void Location::setIndexFile(std::string const & indexFile) {
    _index_file = indexFile;
}

void Location::setAutoIndex(bool autoIndex) {
    _auto_index = autoIndex;
}

void Location::setInterpreterPath(std::string const & interpreterPath) {
    _interpreter_path = interpreterPath;
}

void Location::setScriptExtension(std::string const & scriptExtension) {
    _script_extension = scriptExtension;
}
} // data

