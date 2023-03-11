#include <arpa/inet.h>

#include "ServerData.hpp"
// ---- kqueue ----
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>

/** Default constructor */
ServerData::ServerData()
    /** Initializing default values for the server block */
    : _server_name("localhost"),
    _listens_to("80"),
    _ip_address("127.0.0.1"),
    _root_directory("./" + _server_name),
    _index_file(_root_directory + "/" + "index.html"),
    _client_max_body_size(1024),
    _error_page(""),
    _port_redirection(0) {
    std::cout << PUR << "ServerData Default constructor" << RES << std::endl;
//    _location_data_vector.push_back(ServerLocation(_root_directory, _index_file));//TODO WILL IT CALL THE ServerLocation CONSTRUCTOR???
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
    _port_redirection(rhs._port_redirection),
    _location_data_vector(rhs._location_data_vector) {
    std::cout << PUR << "ServerData Copy constructor" << RES << std::endl;
}

/** Destructor */
ServerData::~ServerData() {
    /** Cleaning default values for the server block */
    _server_name = "";
    _listens_to = "";
    _ip_address = std::string();
    _root_directory = std::string();
    _index_file = std::string();
    _client_max_body_size = 0;
    _error_page = std::string();
    _port_redirection = 0;
//    _location_data_vector = std::vector<ServerLocation>();//TODO WILL IT CALL THE ServerLocation CONSTRUCTOR???
    std::cout << PUR << "ServerData Destructor" << RES << std::endl;
}

/** #################################### Methods #################################### */



/** #################################### Getters #################################### */

std::string ServerData::getServerName() const {
    return _server_name;
}

std::string ServerData::getListensTo() const {
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

std::vector<ServerLocation> & ServerData::getLocationBlocks() {
    return _location_data_vector;
}

int ServerData::getListeningSocket() const {
    return _listening_socket;
}

struct addrinfo* ServerData::getAddr() const {
    return _addr;
}

/** #################################### Setters #################################### */

static bool isServerNameValid(int ch) {
    if (isalpha(ch) != 0 || ch == '.') {
        return true;
    }
    return false;
}

bool ServerData::setServerName(std::string const & name) {
    /* not mandatory | default: localhost */
    if (not name.empty()) {
        if (std::all_of(name.begin(), name.end(), isServerNameValid)) {
            _server_name = name;
            return true;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("server_name", NOT_SUPPORTED));
        }
    }
    return false;
}

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
bool ServerData::setListensTo(std::string const & port) {
    /* not mandatory | default 80 */
    if (not port.empty()) {
        try {
            unsigned short const & listensToPort = std::strtol(port.c_str(), nullptr, 10);
            if (listensToPort == 80 || listensToPort == 591 || listensToPort == 8008 || listensToPort == 8080 ||
                    listensToPort >= 49152) {// todo:: add 65536 as acceptable? then change form short to int?
                /* No need to check port < 65536 since port is an unsigned short already */
                _listens_to = port;
                return true;
            } else {
                throw ParserException(CONFIG_FILE_ERROR("listens_to", NOT_SUPPORTED));
            }
        } catch (...) {
            throw ParserException(CONFIG_FILE_ERROR("listens_to", NOT_SUPPORTED));
        }
    }
    return false;
}


void ServerConfig::setHost(std::string parametr)
{
    if (parametr == "localhost")
        parametr = "127.0.0.1";
    if (!isValidHost(parametr))
        throw ErrorException("Wrong syntax: host");
    this->_host = inet_addr(parametr.data());
}

/*
 * To add a hostname or IP address to your macOS machine, you can modify the "hosts" file located at /etc/hosts. This file maps hostnames to IP addresses and is used by the operating system to resolve domain names to IP addresses locally, without querying a DNS server.

To edit the hosts file, you can follow these steps:

Open the Terminal app on your Mac.

Type the following command to open the hosts file in the nano text editor:

sudo nano /etc/hosts

This will prompt you to enter your password, as the sudo command requires administrative privileges to modify system files.

Use the arrow keys to navigate to the end of the file, and add a new line for the hostname or IP address you want to add. The format of the line should be:

<IP address> <hostname>

For example, if you want to add the hostname "example.com" with IP address "192.168.0.1", the line would look like this:

192.168.0.1 example.com

Press Ctrl+O to save the file, then press Ctrl+X to exit the nano editor.

Finally, flush the DNS cache by typing the following command in the Terminal:

sudo dscacheutil -flushcache

This will ensure that the updated hosts file is used by the operating system.

After making these changes, you should be able to use the hostname or IP address you added to access the associated network resource on your Mac.
 */
bool ServerData::setIpAddress(std::string const & ip) {
    /* not mandatory | default: 127.0.0.1 */
    if (not ip.empty()) {
        struct sockaddr_in sockAddr;
        if (inet_pton(AF_INET, ip.c_str(), &(sockAddr.sin_addr))) {
            _ip_address = ip;// TODO _ip_address = inet_addr(ip.c_str()); ???
            return true;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("ip_address", NOT_SUPPORTED));
        }
    }
    return false;
//    WTF? LOL
//    std::string::size_type it;
//    int dots_quantity = 0;
//    int ip_chunk = 0;
//    std::string copy_ip = ip;
//
//    while (not copy_ip.empty()) {
//        std::cout << RED_BG << "copy_ip: " << copy_ip << RES << std::endl;
//
//        it = copy_ip.find('.');
//        try {
//            ip_chunk = std::stoi(copy_ip.substr(0, it));
//        } catch (...) {
//            throw ParserException(IP_ERROR);
//        }
//        if (ip_chunk < 0 || ip_chunk > 255) {
//            throw ParserException(IP_ERROR);
//        }
//        if (it != std::string::npos) {
//            dots_quantity++;
//        } else {
//            if (dots_quantity == 3) {
//                break;
//            } else if (dots_quantity > 3 || dots_quantity < 3) {
//                throw ParserException(IP_ERROR);
//            }
//        }
//        copy_ip = copy_ip.substr(it + 1);
//        if (copy_ip.empty() && dots_quantity == 3) {
//            throw ParserException(IP_ERROR);
//        }
//    }
//    _ip_address = ip;
}

bool ServerData::setRootDirectory(std::string const & root_dir) {
    /* not mandatory | default: ./$server_name */
    if (not root_dir.empty()) {
        PathType type = pathType(root_dir);
        if (type == DIRECTORY) {
            _root_directory = addCurrentDirPath(root_dir) + root_dir;
            return true;
        } else if (type == PATH_TYPE_ERROR) {
            throw ParserException(CONFIG_FILE_ERROR("root_directory", MISSING));
        } else {
            throw ParserException(CONFIG_FILE_ERROR("root_directory", NOT_SUPPORTED));
        }
    }
    return false;
}

bool ServerData::setIndexFile(std::string const & idx_file) {
    /* not mandatory | default: $root_directory/index.html */
    if (not idx_file.empty()) {
        std::string indexFile = isPath(_root_directory, idx_file);
        if (pathType(indexFile) == REG_FILE) {
            _index_file = addCurrentDirPath(indexFile) + indexFile;
            return true;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("index_file", NOT_SUPPORTED));
        }
    }
    return false;
}

bool ServerData::setClientMaxBodySize(std::string const & body_size) {
    /* not mandatory | default: 1024 (1KB) -> Max: INT_MAX (2GB) */
    if (not body_size.empty()) {
        try {
            unsigned int const & bodySize = std::strtol(body_size.c_str(), nullptr, 10);
            if (bodySize <= INT32_MAX) {
                _client_max_body_size = bodySize;
                return true;
            } else {
                throw ParserException(CONFIG_FILE_ERROR("client_max_body_size", NOT_SUPPORTED));
            }
        } catch (...) {
            throw ParserException(CONFIG_FILE_ERROR("client_max_body_size", NOT_SUPPORTED));
        }
    }
    return false;
}

bool ServerData::setErrorPage(std::string const & err_page) {
    /* not mandatory | default: empty, no set error page, webserver will decide */
    if (not err_page.empty()) {
        std::string errorPage = isPath(_root_directory, err_page);
        if (pathType(errorPage) == REG_FILE) {
            _error_page = addCurrentDirPath(errorPage) + errorPage;
            return true;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("error_page", NOT_SUPPORTED));
        }
    }
    return false;
}

bool ServerData::setPortRedirection(std::string const & port_redir) {
    /* not mandatory | default: zero, no redirection */
    if (not port_redir.empty()) {
        unsigned int portRedirection = std::strtol(port_redir.c_str(), nullptr, 10);
        if (port_redir != _listens_to &&
            (portRedirection == 80 || portRedirection == 591 || portRedirection == 8008 || portRedirection == 8080 ||
                    portRedirection >= 49152)) {// todo:: add 65536 as acceptable? then change form short to int?
            /* No need to check port < 65536 since port is an unsigned short already */
            _port_redirection = portRedirection;
            return true;
        } else {
            throw ParserException(CONFIG_FILE_ERROR("port_redirection", NOT_SUPPORTED));
        }
    }
    return false;
}

void ServerData::setListeningSocket() {
    _addr = new struct addrinfo();
    struct addrinfo hints = addrinfo();

    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    std::cout  << "IP ADDRESS: " << _server_name << std::endl;
    std::cout  << "PORT: " << _listens_to << std::endl;
    // hostname: is either a valid host name or a numeric host address string consisting of a dotted decimal IPv4 address or an IPv6 address.
    // servname: is either a decimal port number or a service name listed in services(5).
    if (getaddrinfo(_server_name.c_str(), _listens_to.c_str(), &hints, &_addr) != 0) {
        freeaddrinfo(_addr);
        throw ServerDataException("failed addr");
    }

    _listening_socket = socket(_addr->ai_family, _addr->ai_socktype, _addr->ai_protocol);
    if (_listening_socket < 0)
        throw ServerDataException(("failed socket"));
    fcntl(_listening_socket, F_SETFL, O_NONBLOCK);

    int socket_on = 1;
    setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
    if (bind(_listening_socket, _addr->ai_addr, _addr->ai_addrlen) == -1)
        throw ServerDataException(("failed bind: " + std::to_string(errno)));
    if (listen(_listening_socket, SOMAXCONN) == -1)  // max nr of accepted connections
        throw ServerDataException(("failed listen"));
}
