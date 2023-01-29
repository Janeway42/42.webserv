#include "includes/LocationData.hpp"

namespace data {
/** Default constructor */ //private???
Location::Location() {
    /** Initializing default values for the location block */
    _location_block = 0;
    _root_directory = std::string();
    _allow_methods = std::vector<AllowMethods>();
    _index_file = std::string();
    _auto_index = false;
    /* Only one location block by default */
    _next_location = nullptr;
}

/** Overloaded constructor */
Location::Location(std::string const & server_root_directory, std::string const & server_index_file) {
    /** Initializing default values for the location block */
    _location_block = false;
    _root_directory = server_root_directory;
    _allow_methods = std::vector<AllowMethods>(GET);
    _index_file = server_index_file;
    _auto_index = false;
    /* Only one location block by default */
    _next_location = nullptr;
}

/** Destructor */
Location::~Location() {
    /** Cleaning default values for the location block */
    _location_block = false;
    _root_directory = std::string();
    _allow_methods = std::vector<AllowMethods>(NONE);
    _index_file = std::string();
    _auto_index = false;
    _next_location = nullptr;
}

/** ########################################################################## */

/** Location Methods */
//Location Location::nextLocationBlock(Location & currentLocation) {
//    Server server_data = Server();
//    Location nextLocation = Location(server_data.getRootDirectory(), server_data.getIndexFile());
//    currentLocation._next_location = &nextLocation;
//    return nextLocation;
//}

/** Location Getters */
bool Location::hasLocationBlock() const {
    return _location_block;
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

Location *Location::getNextBlock() const {
    return _next_location;
}

/** ########################################################################## */

/** Location Setters */
void Location::setHasLocationBlock(bool next_server_block) {
    _location_block = next_server_block;
}

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

void Location::setNextBlock(Server standardServerData) {
//    _next_location = next_location_block;
    Location nextLocation = Location(standardServerData.getRootDirectory(), standardServerData.getIndexFile());
    _next_location = &nextLocation;
}
} // data

