#include "ConfigFileParser.hpp"

/** Private Default Constructor */
//ConfigFileParser::ConfigFileParser()
//    : _server_data(ServerData()),
//    _location_data(ServerLocation()),
//    _server_block_counter(0),
//    _location_block_counter(0),
//    servers(std::vector<ServerData>()) {
//}

/** Overloaded constructor */
ConfigFileParser::ConfigFileParser()
    : _server_data(ServerData()),
      _server_location(ServerLocation(_server_data.getRootDirectory(), _server_data.getIndexFile())),
      _server_block_counter(0),
      _location_block_counter(0) {
    std::cout << BLU << "JOYCE CREATING ConfigFileParser " << BACK << std::endl;
//    handleFile(configFileName);
}

/** Destructor */
ConfigFileParser::~ConfigFileParser() {
    /** Cleaning default values */
    _server_block_counter = 0;
    _location_block_counter = 0;
}

/** #################################### Getters #################################### */

unsigned short ConfigFileParser::numberOfServerBlocks() const {
    return _server_block_counter;
}

unsigned short ConfigFileParser::numberOfLocationBlocks() const {
    return _location_block_counter;
}

/** #################################### Methods #################################### */

std::vector<ServerData> ConfigFileParser::handleFile(std::string const & configFileName) {
    // std::ifstream destructor will close the file automatically, which is one of the perks of using this class.
    // An IDE may compile the program on another directory and so the file to open would be on a different path.
    std::ifstream configFile;
    configFile.open("./" + configFileName);
    if (configFile.is_open()) {
        parseFileServerBlock(configFile);
        if (servers.empty()) {
            throw ParserException(CONFIG_FILE_ERROR("Configuration File", MISSING));
        }
    } else {
        std::cerr << "Not able to open the configuration file" << std::endl;// TODO: throw exception
        return std::vector<ServerData>();
    }
    configFile.close();
    return servers;
}

void ConfigFileParser::parseFileServerBlock(std::ifstream & configFile) {
    while (configFile) {
        std::string lineContent;
        std::getline(configFile, lineContent);

        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        } else if (lineContent.find('}') != std::string::npos) {
            /* Adding to the vector whatever was inserted into _server_data */
            std::cout << "####################" << std::endl;
//            _server_data.addToLocationVector(serverData);
            servers.push_back(_server_data);
            std::cout << "####################" << std::endl;


            /* Cleaning the _server_data and _location_data private members, so it can receive new data if a new
             * server block is found */
//            _server_data = ServerData();
//            _location_data = ServerLocation(_server_data.getRootDirectory(), _server_data.getIndexFile());
            break;
        } else if (lineContent == "server {") {
            std::cout << "server block:" << std::endl;
            _server_block_counter++;
        } else if (lineContent != "server {" && numberOfServerBlocks() == 0) {
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
            _server_location.setLocationAsCgi(true);
            _location_block_counter++;
            parseFileLocationBlock(configFile);
            continue;
        } else if (lineContent.find("location") != std::string::npos && lineContent.find('{') != std::string::npos) {
            std::cout << "location block:" << std::endl;
            _location_block_counter++;
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
            /* Adding a location block to the location vector */
            std::cout << "_server_location.isLocationCgi(): " << _server_location.isLocationCgi() << std::endl;
            _location_data_vector.push_back(_server_location);

            std::cout << "_server_location.isLocationCgi(): " << _server_location.isLocationCgi() << std::endl;

            /* Cleaning the _location_data private member, so it can receive new data if a new location
             * block is found inside this current server block */
//            _location_data = ServerLocation(_server_data.getRootDirectory(), _server_data.getIndexFile());

            /* Now we are going out of a possible cgi block */
//            _location_data.setLocationAsCgi(false);
            break;
        }
        /** Handling the location or cgi block key values */
        if (not rootDirectoryAlreadyChecked && _server_location.setRootDirectory(keyParser(lineContent, "root_directory"))) {
            rootDirectoryAlreadyChecked = true;
            continue;
        }
        if (_server_location.setAllowMethods(keyParser(lineContent, "allow_methods"))) {
            continue;
        }
        if (_server_location.setIndexFile(keyParser(lineContent, "index_file"))) {
            continue;
        }
        if (_server_location.setAutoIndex(keyParser(lineContent, "auto_index"))) {
            continue;
        }
        if (not interpreterPathAlreadyChecked &&
            _server_location.setInterpreterPath(keyParser(lineContent, "interpreter_path"))) {
            interpreterPathAlreadyChecked = true;
            continue;
        }
        if (not scriptExtensionAlreadyChecked &&
            _server_location.setScriptExtension(keyParser(lineContent, "script_extension"))) {
            scriptExtensionAlreadyChecked = true;
            continue;
        }
    }
}

