#include "includes/ServerData.hpp"
#include "includes/Parser.hpp"
#include <arpa/inet.h>

/** Default constructor */
ServerData::ServerData()
    /** Initializing default values for the server block */
    : _server_name("localhost"),
    _listens_to(80),
    _ip_address("127.0.0.1"),
    _root_directory("./localhost"),
    _index_file("index.html"),
    _client_max_body_size(1024),
    /* default: $root_directory/error_pages/$status_code.html -> $status_code.html will be set later */
    _error_page(_root_directory + "/error_pages"),
    _port_redirection(_listens_to) {
}

/** Copy constructor */
ServerData::ServerData(ServerData const & rhs)
    : _server_name(rhs._server_name),
    _listens_to(rhs._listens_to),
    _ip_address(rhs._ip_address),
    _root_directory(rhs._root_directory),
    _index_file(rhs._index_file),
    _client_max_body_size(rhs._client_max_body_size),
    _error_page(rhs._error_page),
    _port_redirection(rhs._port_redirection) {
}

/** Destructor */
ServerData::~ServerData() {
    /** Cleaning default values for the server block */
    _server_name = "";
    _listens_to = 0;
    _ip_address = std::string();
    _root_directory = std::string();
    _index_file = std::string();
    _client_max_body_size = 0;
    _error_page = std::string();
    _port_redirection = 0;
}

/** #################################### Methods #################################### */



/** #################################### Getters #################################### */

std::string ServerData::getServerName() const {
    return _server_name;
}

unsigned int ServerData::getListensTo() const {
    return _listens_to;
}

std::string ServerData::getIpAddress() const {
    return _ip_address;
}

std::string ServerData::getRootDirectory() const {
    return _root_directory;
}

std::string ServerData::getIndexFile() const {
    return _index_file;
}

unsigned int ServerData::getClientMaxBodySize() const {
    return _client_max_body_size;
}

std::string ServerData::getErrorPage() const {
    return _error_page;
}

unsigned int ServerData::getPortRedirection() const {
    return _port_redirection;
}

/** #################################### Setters #################################### */

static bool isServerNameValid(int ch) {
    if (isalpha(ch) != 0 || ch == '.') {
        return true;
    }
    return false;
}

void ServerData::setServerName(std::string const & name) {
    if (std::all_of(name.begin(), name.end(), isServerNameValid)) {
        _server_name = name;
    } else {
        throw Parser::ParserException(NAME_ERROR);
    }
}

void ServerData::setListensTo(unsigned short const & port) {
    /* Available ports:
     * - Port 80 (standard): a well-known system ports (they are assigned and controlled by IANA).
     * - Port 591 (): a well-known system ports (they are assigned and controlled by IANA).
     * - Port 8008: a user or registered port (they are not assigned and controlled but registered by IANA only).
     * - Port 8080 (second most used): a user or registered port (they are not assigned and controlled but registered by IANA only).
     * - Ports ranging from 49152 to 65536: are available for anyone to use.
     *
     * E.g.: If a web server is already running on the default port (80) and another web server needs to be hosted on
     * the HTTP service, it's best practice to host it on port 8080 (but not mandatory, any other alternative or custom
     * port can be used instead).
     * https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml?search=http-alt
     */
    if (port == 80 || port == 591 || port == 8008 || port == 8080 || port >= 49152) {// todo:: add 65536 as acceptable? then change form short to int?
        /* No need to check port < 65536 since port is an unsigned short already */
        _listens_to = port;
    } else {
        throw Parser::ParserException(PORT_ERROR);
    }
}

void ServerData::setIpAddress(std::string const & ip) {
    struct sockaddr_in sockAddr = {};
    if (inet_pton(AF_INET, ip.c_str(), &(sockAddr.sin_addr))) {
        _ip_address = ip;
    } else {
        throw Parser::ParserException(IP_ERROR);
    }
//    WTF? LOL
//    std::string::size_type it;
//    int dots_quantity = 0;
//    int ip_chunk = 0;
//    std::string copy_ip = ip;
//
//    while (not copy_ip.empty()) {
//        std::cout << RED_BG << "copy_ip: " << copy_ip << BACK << std::endl;
//
//        it = copy_ip.find('.');
//        try {
//            ip_chunk = std::stoi(copy_ip.substr(0, it));
//        } catch (...) {
//            throw Parser::ParserException(IP_ERROR);
//        }
//        if (ip_chunk < 0 || ip_chunk > 255) {
//            throw Parser::ParserException(IP_ERROR);
//        }
//        if (it != std::string::npos) {
//            dots_quantity++;
//        } else {
//            if (dots_quantity == 3) {
//                break;
//            } else if (dots_quantity > 3 || dots_quantity < 3) {
//                throw Parser::ParserException(IP_ERROR);
//            }
//        }
//        copy_ip = copy_ip.substr(it + 1);
//        if (copy_ip.empty() && dots_quantity == 3) {
//            throw Parser::ParserException(IP_ERROR);
//        }
//    }
//    _ip_address = ip;
}

void ServerData::setRootDirectory(std::string const & root_dir) {
    if (pathType(root_dir) == DIR) {
        _root_directory = root_dir;
    } else {
        throw Parser::ParserException(ROOT_PATH_ERROR);
    }
}

void ServerData::setIndexFile(std::string const & idx_file) {
    //todo add error handling
    _index_file = idx_file;
}

void ServerData::setClientMaxBodySize(unsigned int const & body_size) {
    //todo add error handling
    _client_max_body_size = body_size;
}

void ServerData::setErrorPage(std::string const & err_page) {
    //todo add error handling
    _error_page = err_page;
}

void ServerData::setPortRedirection(unsigned int const & port_redir) {
    //todo add error handling
    _port_redirection = port_redir;
}
