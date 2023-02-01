#include "includes/ConfigFileParser.hpp"

#include <fstream>

namespace data {
/** Default  */
ConfigFile::ConfigFile() {// todo make it unacceptable to construct?
}

ConfigFile::ConfigFile(std::string const & configFileName) {
//   _server_data = new Server();??
    _server_data = Server();
    _location_data = Location(_server_data.getRootDirectory(), _server_data.getIndexFile());
    _location_data_vector = std::vector<Location>();
    handleFile(configFileName);
}

/** Destructor */
ConfigFile::~ConfigFile() {
//   delete _data;
}

/** #################################### Getters #################################### */

std::map<Server*, std::vector<Location> > const & ConfigFile::getServerDataMap() const {
    return _server_map;
}

/** #################################### Methods #################################### */
bool ConfigFile::handleFile(std::string const & configFileName) {
    /** Opening the file */
    // std::ifstream destructor will close the file automatically, which is one of the perks of using this class.
    // The IDE may compile the program on a child directory and so the file to open would be one path behind
    std::ifstream configFile;
    configFile.open("./" + configFileName);
    if (configFile.is_open()) {
        ConfigFile::parseFileServerBlock(configFile);
        _server_map.insert(std::make_pair(&_server_data, _location_data_vector));

//        //testing
//        std::map<data::Server*, std::vector<data::Location> >::const_iterator it_location = _server_map.begin();
//        for (; it_location != _server_map.end(); it_location++) {
//            std::cout << RED_BG << "JOYCE server name: " << it_location->first->getServerName() << BACK << std::endl;
//        }

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
            // TODO we have to know how many Server blocks we have to deal with
            // checking size of map is ok?
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

        /** Start reading and parsing the server block */
        std::string serverName = keyParser(lineContent, "server_name");
        if (!serverName.empty()) {
            _server_data.setServerName(serverName);
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
            //_location_data.setHasLocationBlock(true);
            std::cout << RED << "Server block already have a location block? " << std::boolalpha << _location_data.hasLocationBlock() << BACK << std::endl;
            parseFileLocationBlock(configFile);// current location data

            // TODO we have to know how many Location blocks we have to deal with (inside this Server block)
            // check size of vector instead?
        }
        // TODO SET CGI VALUES
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
            /** Adding the location block to the vector so it can be later on added to the server map */
            _location_data_vector.push_back(_location_data);

//            //testing
//            std::vector<data::Location>::const_iterator it_location = _location_data_vector.begin();
//            for (; it_location != _location_data_vector.end(); it_location++) {
//                std::cout << RED_BG << "JOYCE location root directory: " << it_location->getRootDirectory() << BACK << std::endl;
//            }

            /** Cleaning the _location_data private member so it can receive new data if a new location
             * block is found inside this current server block
             */
            _location_data = Location(_server_data.getRootDirectory(), _server_data.getIndexFile());
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
} // data

