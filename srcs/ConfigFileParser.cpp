#include "includes/ConfigFileParser.hpp"

#include <fstream>

namespace data {
/** Default constructor */
ConfigFile::ConfigFile() {// todo make it unacceptable to construct?
}

ConfigFile::ConfigFile(std::string const & configFileName) {
//   _server_data = new Server();
    _server_data = Server();
    _location_data = Location(_server_data.getRootDirectory(), _server_data.getIndexFile());
    handleFile(configFileName);
}

/** Destructor */
ConfigFile::~ConfigFile() {
//   delete _data;
}

/** ########################################################################## */

bool ConfigFile::handleFile(std::string const & configFileName) {
    /** Opening the file */
    //std::ifstream destructor will close the file automatically, which is one of the perks of using this class.
    // The IDE may compile the program on a child directory and so the file to open would be one path behind
    std::ifstream configFile;
    configFile.open("./" + configFileName);

    if (configFile.is_open()) {
        ConfigFile::parseFileServerBlock(configFile);
    } else {
        std::cerr << "Not able to open the configuration file" << std::endl;
        return false;
    }
    configFile.close();
    return true;
}

void ConfigFile::parseFileServerBlock(std::ifstream & configFile) {
    std::string lineContent;

    /** Reading from the file (per word) */
    while (configFile) {
        std::getline(configFile, lineContent);
        std::cout << RES << "  Server lineContent: " << lineContent << BACK << std::endl;
        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        }
        if (lineContent.find('}') != std::string::npos) {
            _server_data.setHasServerBlock(false);
            continue;
        }
        if (lineContent == "server {") {
            _server_data.setHasServerBlock(true);
            std::cout << RED << "Has server block: " << std::boolalpha << _server_data.hasServerBlock() << BACK << std::endl;
        }
        else if (lineContent != "server {" && !_server_data.hasServerBlock()) {
            std::cerr << "A server block is needed in the configuration file" << std::endl;
            break;
        }

// TODO: find a way to check if a server block was already parsed so I would need to create another node of Server to send to the parseFileServerBlock recursively
// ex: Server next_server = _server_data._next_location; // malloc?
// ex: parseFileServerBlock(configFile, next_server);
// Also we have to know how many Server blocks we have to deal with

        //DataType datatype = getValueType(lineContent);
        /** Start reading and parsing the server block */
        std::string serverName = keyParser(lineContent, "server_name");
        if (!serverName.empty()) {
            _server_data.setServerName(serverName);
            std::cout << RED_BG << "JOYCE : " << _server_data.getServerName() << BACK << std::endl;
            continue;
        }
        std::string listensTo = keyParser(lineContent, "listens_to");
        if (!listensTo.empty()) {
            _server_data.setListensTo(std::strtol(listensTo.c_str(), nullptr, 10));
            continue;
        }
        std::string ipAddress = keyParser(lineContent, "ip_address");
        if (!ipAddress.empty()) {
            _server_data.setIpAddress(ipAddress);
            continue;
        }
        std::string rootDirectory = keyParser(lineContent, "root_directory");
        if (!rootDirectory.empty()) {
            _server_data.setRootDirectory(rootDirectory);
            continue;
        }
        std::string indexFile = keyParser(lineContent, "index_file");
        if (!indexFile.empty()) {
            _server_data.setIndexFile(indexFile);
            continue;
        }
        std::string clientMaxBodySize = keyParser(lineContent, "client_max_body_size");
        if (!clientMaxBodySize.empty()) {
            _server_data.setClientMaxBodySize(std::strtol(clientMaxBodySize.c_str(), nullptr, 10));
            continue;
        }
        std::string errorPage = keyParser(lineContent, "error_page");
        if (!errorPage.empty()) {
            _server_data.setErrorPage(errorPage);
            continue;
        }
        std::string portRedirection = keyParser(lineContent, "port_redirection");
        if (!portRedirection.empty()) {
            _server_data.setPortRedirection(std::strtol(portRedirection.c_str(), nullptr, 10));
            continue;
        }
        if (lineContent.find("location") != std::string::npos && lineContent.find('{') != std::string::npos ) {
//            _location_data.setHasLocationBlock(true);
            std::cout << RED << "Server block already have a location block? " << std::boolalpha << _location_data.hasLocationBlock() << BACK << std::endl;

// TODO: find a way to check if a location block was already parsed so I would need to create another node of Location to send to the parseFileLocationBlock recursively
// Also we have to know how many Location blocks we have to deal with (inside this Server block)

//            if (_location_data.hasLocationBlock()) {
//                _location_data.setNextBlock(_server_data);
//                parseFileLocationBlock(configFile);// returns the new created node, empty only with Server standards
//            } else {
                parseFileLocationBlock(configFile);// current location data
//            }
        }
    }
}

void ConfigFile::parseFileLocationBlock(std::ifstream & configFile) {
    std::string lineContent;

    /** Start reading and parsing a location block */
    while (configFile) {
        std::getline(configFile, lineContent);
        std::cout << RES << "Location lineContent: " << lineContent << BACK << std::endl;
        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        }
        if (lineContent.find('}') != std::string::npos) {
            _location_data.setHasLocationBlock(true);
            break;
        }
        std::string locationRootDirectory = keyParser(lineContent, "root_directory");
        if (!locationRootDirectory.empty()) {
            _location_data.setRootDirectory(locationRootDirectory);
            continue;
        }
        std::string locationAllow_methodsString = keyParser(lineContent, "allow_methods");
        if (!locationAllow_methodsString.empty()) {
            std::vector<AllowMethods> locationAllow_methods;
            if (locationAllow_methodsString.find("GET") != std::string::npos) {
                locationAllow_methods.push_back(GET);
            }
            if (locationAllow_methodsString.find("POST") != std::string::npos) {
                locationAllow_methods.push_back(POST);
            }
            if (locationAllow_methodsString.find("DELETE") != std::string::npos) {
                locationAllow_methods.push_back(DELETE);
            }
            _location_data.setAllowMethods(locationAllow_methods);
            continue;
        }
        std::string locationIndexFile = keyParser(lineContent, "index_file");
        if (!locationIndexFile.empty()) {
            _location_data.setIndexFile(locationIndexFile);
            continue;
        }
        std::string autoIndex = keyParser(lineContent, "auto_index");
        if (!autoIndex.empty()) {
            if (autoIndex.find("on") != std::string::npos) {
                _location_data.setAutoIndex(true);
            } else if (autoIndex.find("off") != std::string::npos) {
                _location_data.setAutoIndex(false);
            }
            continue;
        }
    }
}

//template<typename T>
std::string ConfigFile::keyParser(std::string & lineContent, std::string const & keyToFind) {
    if (lineContent.empty() || keyToFind.empty())
        return std::string();
    if (lineContent.find(keyToFind) != std::string::npos) {
        std::cout << YEL << "Found "<< keyToFind << " line [" << lineContent << "]" << BACK << std::endl;
        return getOneCleanValueFromKey(lineContent, keyToFind);
    }
    return std::string();
}

/** Getters */
Server const & ConfigFile::getServerData() const {
    return _server_data;
}

Location const & ConfigFile::getLocationData() const {
    return _location_data;
}
} // data

