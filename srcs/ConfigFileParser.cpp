#include "includes/ConfigFileParser.hpp"

/** Default Constructor */
ConfigFileParser::ConfigFileParser()
    : _server_data(ServerData()),
    _server_block_counter(0),
    _location_data(LocationData(_server_data.getRootDirectory(), _server_data.getIndexFile())),
    _location_block_counter(0),
    _location_data_vector(std::vector<LocationData>()) {
}

//ConfigFileParser::ConfigFileParser(std::string const & configFileName) {
//    /** Initializing default values for the config file */
//   _server_data = new ServerData();??
//    _server_data = ServerData();
//    _server_block_counter = 0;
//    _location_data = LocationData(_server_data.getRootDirectory(), _server_data.getIndexFile());
//    _location_block_counter = 0;
//    _location_data_vector = std::vector<LocationData>();
//}

/** Destructor */
ConfigFileParser::~ConfigFileParser() {
    /** Cleaning default values */
    _server_block_counter = 0;
    _location_block_counter = 0;
    _location_data_vector = std::vector<LocationData>();
//    _server_map = std::map<ServerData*, std::vector<LocationData> >();
//   delete _data;
}

/** #################################### Getters #################################### */

std::map<ServerData*, std::vector<LocationData> > const & ConfigFileParser::getServerDataMap() const {
    return _server_map;
}

unsigned short ConfigFileParser::getServerBlockCounter() const {
    return _server_block_counter;
}

unsigned short ConfigFileParser::getLocationBlockCounter() const {
    return _location_block_counter;
}

/** #################################### Methods #################################### */

void ConfigFileParser::serverBlockCounter() {
    _server_block_counter++;
}

void ConfigFileParser::locationBlockCounter() {
    _location_block_counter++;
}

std::map<ServerData*, std::vector<LocationData> > ConfigFileParser::parseFile(std::string const & configFileName) {
    handleFile(configFileName);
    return _server_map;
}

bool ConfigFileParser::handleFile(std::string const & configFileName) {
    /** Opening the file */
    // std::ifstream destructor will close the file automatically, which is one of the perks of using this class.
    // The IDE may compile the program on a child directory and so the file to open would be one path behind
    std::ifstream configFile;
    configFile.open("./" + configFileName);
    if (configFile.is_open()) {
        parseFileServerBlock(configFile);
    } else {
        std::cerr << "Not able to open the configuration file" << std::endl;
        return false;
    }
    configFile.close();
    return true;
}


// TODO CHECK FOR MANDATORY / NON MANDATORY FIELDS
void ConfigFileParser::parseFileServerBlock(std::ifstream & configFile) {
    while (configFile) {
        std::string lineContent;
        std::getline(configFile, lineContent);
        std::cout << RES << "  ServerData lineContent: " << lineContent << BACK << std::endl;

        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        } else if (lineContent.find('}') != std::string::npos) {
            /** Adding to the map whatever was inserted into _server_data and _location_data_vector */

            //            ServerData* serverBlock = new ServerData(_server_data);
            _server_map.insert(
                    std::make_pair(new ServerData(_server_data), _location_data_vector));// TODO check if it leaks

            /** Cleaning the _server_data and _location_data private members so it can receive new data if a new
             * server block is found
             */
            _server_data = ServerData();
            _location_data = LocationData(_server_data.getRootDirectory(), _server_data.getIndexFile());
            break;
        } else if (lineContent == "server {") {
            serverBlockCounter();
            std::cout << RED << "Number of server block(s): " << getServerBlockCounter() << BACK << std::endl;
        } else if (lineContent != "server {" && getServerBlockCounter() == 0) {
            std::cerr << RED << "A server block is needed in the configuration file" << BACK << std::endl;
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
        if (lineContent.find("cgi {") != std::string::npos) {
            _location_data.setLocationAsCgi(true);
            locationBlockCounter();

            parseFileLocationBlock(configFile);
            std::cout << RED << "(cgi) Number of location block(s)? " << getLocationBlockCounter() << BACK
                      << std::endl;
            continue;
        } else if (lineContent.find("location") != std::string::npos &&
                   lineContent.find('{') != std::string::npos) {
            locationBlockCounter();

            parseFileLocationBlock(configFile);
            std::cout << RED << "Number of location block(s)? " << getLocationBlockCounter() << BACK << std::endl;

            continue;
        }
    }
}

void ConfigFileParser::parseFileLocationBlock(std::ifstream & configFile) {
    std::string lineContent;

    /** Start reading and parsing a location block */
    while (configFile) {
        std::getline(configFile, lineContent);
        std::cout << RES << "LocationData lineContent: " << lineContent << BACK << std::endl;
        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        }
        if (lineContent.find('}') != std::string::npos) {
            /** Adding the location block to the vector so it can be later on added to the server map */
            _location_data_vector.push_back(_location_data);

            /** Cleaning the _location_data private member so it can receive new data if a new location
             * block is found inside this current server block
             */
            _location_data = LocationData(_server_data.getRootDirectory(), _server_data.getIndexFile());

            /** Now we are going out if a possible cgi block */
            _location_data.setLocationAsCgi(false);
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
        std::string interpreterPath = keyParser(lineContent, "interpreter_path");
        if (!interpreterPath.empty()) {
            if (interpreterPath[0] != '/') {
                std::cerr << RED << "interpreter_path has to have a full path" << BACK << std::endl;
            }
            _location_data.setInterpreterPath(interpreterPath);
            continue;
        }
        std::string scriptExtension = keyParser(lineContent, "script_extension");
        if (!scriptExtension.empty()) {
            _location_data.setScriptExtension(scriptExtension);
            continue;
        }
    }
}

//template<typename T>
//std::string ConfigFileParser::keyParser(std::string & lineContent, std::string const & keyToFind) {
//    if (lineContent.empty() || keyToFind.empty())
//        return std::string();
//    if (lineContent.find(keyToFind) != std::string::npos) {
//        std::cout << YEL << "Found "<< keyToFind << " line [" << lineContent << "]" << BACK << std::endl;
//        return getOneCleanValueFromKey(lineContent, keyToFind);
//    }
//    return std::string();
//}
