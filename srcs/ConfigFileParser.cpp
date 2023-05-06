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
//    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻ Config File ⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
//    std::cout << BLU << "ConfigFileParser Overloaded constructor" << RES << std::endl;
    try {
        handleFile(configFileName);
    } catch (std::exception const & e) {
        std::cout << RED << e.what() << RES << std::endl;
        throw e;
    }
}

/** Destructor */
ConfigFileParser::~ConfigFileParser() {
    /** Cleaning default values */
    _server_block_counter = 0;
    _location_block_counter = 0;
//    std::cout << BLU << "ConfigFileParser Destructor" << RES << std::endl;
//    std::cout << "⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻⎻" << std::endl;
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
                throw ParserException(CONFIG_FILE_ERROR("At least one server block", MANDATORY));
            } else if (lineContent == "server {") {
                _server_block_counter++;
                parseFileServerBlock(configFile);

                continue;
            }
        }
//        if (servers.empty()) {
//            throw ParserException(CONFIG_FILE_ERROR("Configuration File", MISSING));
//        }
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
//        std::cout << RED << lineContent << RES << std::endl;

        if (lineContent.find('#') != std::string::npos || !lineContent[0]) {
            continue;
        } else if (lineContent.find('}') != std::string::npos) {
            /* Adding to the vector whatever was inserted into _server_data */
            servers.push_back(_server_data);
            break;
        }
        /** Handling the server block key values */
        if (lineContent.find("server_name") != std::string::npos) {
            _server_data.setServerName(keyParser(lineContent, "server_name"));
            continue;
        }
        if (lineContent.find("listens_to") != std::string::npos) {
            _server_data.setListensTo(keyParser(lineContent, "listens_to"));
            continue;
        }
        if (lineContent.find("root_directory") != std::string::npos) {
            _server_data.setRootDirectory(keyParser(lineContent, "root_directory"));
            continue;
        }
        if (lineContent.find("index_file") != std::string::npos) {
            _server_data.setIndexFile(keyParser(lineContent, "index_file"));
            continue;
        }
        if (lineContent.find("client_max_body_size") != std::string::npos) {
            _server_data.setClientMaxBodySize(keyParser(lineContent, "client_max_body_size"));
            continue;
        }
        if (lineContent.find("error_page") != std::string::npos) {
            _server_data.setErrorPages(keyParser(lineContent, "error_page"));
            continue;
        }
        if (lineContent.find("upload_directory") != std::string::npos) {
            _server_data.setUploadDirectory(keyParser(lineContent, "upload_directory"));
            continue;
        }
        /** Checking for cgi or location/cgi blocks */
        if (lineContent.find("location") != std::string::npos && lineContent.find('{') != std::string::npos) {
            ServerLocation _server_location(ServerLocation(_server_data.getRootDirectory(), _server_data.getIndexFile()));
            std::string locationValue = keyParser(lineContent, "location");
            if (not locationValue.empty() && locationValue[0] == '.') {
                _server_location.setLocationAsCgi(true);
            }
            _server_location.setLocation(locationValue);
            parseFileLocationBlock(configFile, _server_data, _server_location);
            continue;
        }
    }
}

void ConfigFileParser::parseFileLocationBlock(std::ifstream & configFile, ServerData & _server_data, ServerLocation & _server_location) {
    std::string lineContent;

    _location_block_counter++;
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
        if (lineContent.find("root_directory") != std::string::npos) {
            _server_location.setRootDirectory(keyParser(lineContent, "root_directory"));
            continue;
        }
        if (lineContent.find("allow_methods") != std::string::npos) {
            _server_location.setAllowMethods(keyParser(lineContent, "allow_methods"));
            continue;
        }
        if (lineContent.find("index_file") != std::string::npos) {
            _server_location.setIndexFile(keyParser(lineContent, "index_file"));
            if (_server_location.getIndexFile().empty()) {
                /* In a server configuration file, the default index file specified in the server block applies to all
                 * locations within that server block unless it is explicitly overridden in a location block.
                 * I.e. if the location index file is missing, the server block index file can be used */
                _server_location.setIndexFile(_server_data.getIndexFile());
            }
            /* If the index file specified in the server block has a different name than the index file specified
             * in the location block, both files will be used for requests that match that location */
            else if (_server_location.getIndexFile() != _server_data.getIndexFile()) {
                _server_location.useServerBlockIndexFile = true;// todo delete? not needed since the response class will search on server block anyway if it does not find the index file on the location block
            }
            continue;
        }
        if (lineContent.find("auto_index") != std::string::npos) {
            _server_location.setAutoIndex(keyParser(lineContent, "auto_index"));
            continue;
        }
        if (lineContent.find("interpreter_path") != std::string::npos) {
            _server_location.setInterpreterPath(keyParser(lineContent, "interpreter_path"));
            continue;
        }
        if (lineContent.find("redirection") != std::string::npos) {
            _server_location.setRedirection(keyParser(lineContent, "redirection"));
            continue;
        }
    }
}

