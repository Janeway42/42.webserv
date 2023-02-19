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
    // An IDE may compile the program on another directory and so the file to open would be on a different path.
    std::ifstream configFile;
    configFile.open("./" + configFileName);
    if (configFile.is_open()) {
        parseFileServerBlock(configFile);
    } else {
        std::cerr << "Not able to open the configuration file" << std::endl;// TODO: throw exception
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

        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        } else if (lineContent.find('}') != std::string::npos) {
            /* Adding to the map whatever was inserted into _server_data and _location_data_vector */
            //ServerData* serverBlock = new ServerData(_server_data);
            _server_map.insert(std::make_pair(new ServerData(_server_data), _location_data_vector));// TODO check if it leaks

            /* Cleaning the _server_data and _location_data private members so it can receive new data if a new
             * server block is found */
            _server_data = ServerData();
            _location_data = LocationData(_server_data.getRootDirectory(), _server_data.getIndexFile());
            break;
        } else if (lineContent == "server {") {
            std::cout << "server block:" << std::endl;
            serverBlockCounter();
        } else if (lineContent != "server {" && getServerBlockCounter() == 0) {
            std::cerr << RED << "A server block is needed in the configuration file" << BACK << std::endl;// TODO: throw exception
            break;
        }
        /** Handling the server block key values */
        if (_server_data.setServerName(keyParser(lineContent, "server_name"))) {
            continue;
        }
        if (_server_data.setListensTo(keyParser(lineContent, "listens_to"))) {
            continue;
        }
        if (_server_data.setIpAddress(keyParser(lineContent, "ip_address"))) {
            continue;
        }
        if (_server_data.setRootDirectory(keyParser(lineContent, "root_directory"))) {
            continue;
        }
        if (_server_data.setIndexFile(keyParser(lineContent, "index_file"))) {
            continue;
        }
        if (_server_data.setClientMaxBodySize(keyParser(lineContent, "client_max_body_size"))) {
            continue;
        }
        if (_server_data.setErrorPage(keyParser(lineContent, "error_page"))) {
            continue;
        }
        if (_server_data.setPortRedirection(keyParser(lineContent, "port_redirection"))) {
            continue;
        }
        /** Checking for cgi or location blocks */
        if (lineContent.find("cgi {") != std::string::npos) {
            std::cout << "cgi block:" << std::endl;
            _location_data.setLocationAsCgi(true);
            locationBlockCounter();
            parseFileLocationBlock(configFile);
            continue;
        } else if (lineContent.find("location") != std::string::npos && lineContent.find('{') != std::string::npos) {
            std::cout << "location block:" << std::endl;
            locationBlockCounter();
            parseFileLocationBlock(configFile);
            continue;
        }
    }
}

void ConfigFileParser::parseFileLocationBlock(std::ifstream & configFile) {
    std::string lineContent;
    bool rootDirectoryAlreadyChecked = false;
    bool scriptExtensionAlreadyChecked = false;
    bool interpreterPathAlreadyChecked = false;

    /** Handling the location or cgi block key values */
    while (configFile) {
        std::getline(configFile, lineContent);
        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        }
        if (lineContent.find('}') != std::string::npos) {
            /* Adding the location block to the vector, so it can be later on added to the server map */
            _location_data_vector.push_back(_location_data);

            /* Cleaning the _location_data private member, so it can receive new data if a new location
             * block is found inside this current server block */
            _location_data = LocationData(_server_data.getRootDirectory(), _server_data.getIndexFile());

            /* Now we are going out of a possible cgi block */
            _location_data.setLocationAsCgi(false);
            break;
        }
        /** Handling the location or cgi block key values */
        if (not rootDirectoryAlreadyChecked &&
                _location_data.setRootDirectory(keyParser(lineContent, "root_directory"))) {
            rootDirectoryAlreadyChecked = true;
            continue;
        }
        if (_location_data.setAllowMethods(keyParser(lineContent, "allow_methods"))) {
            continue;
        }
        if (_location_data.setIndexFile(keyParser(lineContent, "index_file"))) {
            continue;
        }
        if (_location_data.setAutoIndex(keyParser(lineContent, "auto_index"))) {
            continue;
        }
        if (not interpreterPathAlreadyChecked &&
                _location_data.setInterpreterPath(keyParser(lineContent, "interpreter_path"))) {
            interpreterPathAlreadyChecked = true;
            continue;
        }
        if (not scriptExtensionAlreadyChecked &&
            _location_data.setScriptExtension(keyParser(lineContent, "script_extension"))) {
            scriptExtensionAlreadyChecked = true;
            continue;
        }
    }
}

