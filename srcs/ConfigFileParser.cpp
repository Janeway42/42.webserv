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
ConfigFileParser::ConfigFileParser(std::string const & configFileName)
    : //_server_data(ServerData()),
      //_server_location(ServerLocation(_server_data.getRootDirectory(), _server_data.getIndexFile())),
      _server_block_counter(0),
      _location_block_counter(0) {
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Config File ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
    std::cout << BLU << "ConfigFileParser Overloaded constructor" << RES << std::endl;
    handleFile(configFileName);
}

/** Destructor */
ConfigFileParser::~ConfigFileParser() {
    /** Cleaning default values */
    _server_block_counter = 0;
    _location_block_counter = 0;
    std::cout << BLU << "ConfigFileParser Destructor" << RES << std::endl;
    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
}

/** #################################### Getters #################################### */

unsigned short ConfigFileParser::numberOfServerBlocks() const {
    return _server_block_counter;
}

unsigned short ConfigFileParser::numberOfLocationBlocks() const {
    return _location_block_counter;
}

/** #################################### Methods #################################### */

void ConfigFileParser::handleFile(std::string const & configFileName) {
    // std::ifstream destructor will close the file automatically, which is one of the perks of using this class.
    // An IDE may compile the program on another directory and so the file to open would be on a different path.
    std::ifstream configFile;
    configFile.open("./" + configFileName);
    if (configFile.is_open()) {
        while (configFile) {
            std::string lineContent;
            std::getline(configFile, lineContent);
            if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
                continue;
            } else if (lineContent != "server {" && numberOfServerBlocks() == 0) {
                std::cerr << RED << "A server block is needed in the configuration file" << RES << std::endl;// TODO: throw exception
                break;
            } else if (lineContent == "server {") {
                std::cout << "server block:" << std::endl;
                _server_block_counter++;
                parseFileServerBlock(configFile);
                continue;
            }
        }
        if (servers.empty()) {
            throw ParserException(CONFIG_FILE_ERROR("Configuration File", MISSING));
        }
    } else {
        std::cerr << "Not able to open the configuration file" << std::endl;// TODO: throw exception
        return ;
    }
    configFile.close();
}

void ConfigFileParser::parseFileServerBlock(std::ifstream & configFile) {
    ServerData _server_data = ServerData();
    while (configFile) {
        std::string lineContent;
        std::getline(configFile, lineContent);

        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        } else if (lineContent.find('}') != std::string::npos) {
            /* Adding to the vector whatever was inserted into _server_data */
            servers.push_back(_server_data);
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
        /** Checking for cgi or location/cgi blocks */
        if (lineContent.find("location") != std::string::npos && lineContent.find('{') != std::string::npos) {
            ServerLocation _server_location(ServerLocation(_server_data.getRootDirectory(), _server_data.getIndexFile()));
            if (_server_location.setLocationPath(keyParser(lineContent, "location"))) {
                parseFileLocationBlock(configFile, _server_data, _server_location);
            }
            continue;
        }
    }
}

void ConfigFileParser::parseFileLocationBlock(std::ifstream & configFile, ServerData & _server_data, ServerLocation & _server_location) {
    std::string lineContent;
    bool rootDirectoryAlreadyChecked = false;
    bool scriptExtensionAlreadyChecked = false;
    bool interpreterPathAlreadyChecked = false;

    _location_block_counter++;
    /** Handling the location or cgi block key values */
    while (configFile) {
        std::getline(configFile, lineContent);
        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        }
        if (lineContent.find('}') != std::string::npos) {
            /* Adding a location block to the location vector */
            _server_data.getLocationBlocks().push_back(_server_location);
            break;
        }
        /** Handling the location or cgi block key values */
        if (not rootDirectoryAlreadyChecked &&
                (rootDirectoryAlreadyChecked = _server_location.setRootDirectory(keyParser(lineContent, "root_directory")))) {
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
                (interpreterPathAlreadyChecked = _server_location.setInterpreterPath(keyParser(lineContent, "interpreter_path")))) {
            _server_location.setLocationAsCgi(true);
            continue;
        }
        if (not scriptExtensionAlreadyChecked &&
                (scriptExtensionAlreadyChecked = _server_location.setScriptExtension(keyParser(lineContent, "script_extension")))) {
            continue;
        }
    }
}

