#include "ServerData.hpp"
#include "WebServer.hpp"

// ---- kqueue ----
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>

#include <arpa/inet.h>
#include <sstream>

#include "ServerData.hpp"

/** Default constructor */
ServerData::ServerData()
	/** Initializing default values for the server block */
	: _server_name("localhost"),//127.0.0.1
	_listens_to("80"),
	_root_directory(_server_name),
	_index_file("index.html"),
	_client_max_body_size(1024),
	_error_page(std::vector<std::string>()),
	_listening_socket(-1),
	_addr(NULL){
	std::cout << PUR << "ServerData Default constructor" << RES << std::endl;
}

/** Copy constructor */
ServerData::ServerData(ServerData const & rhs)
	: _server_name(rhs._server_name),
	_listens_to(rhs._listens_to),
	_root_directory(rhs._root_directory),
	_index_file(rhs._index_file),
	_client_max_body_size(rhs._client_max_body_size),
	_error_page(rhs._error_page),
	_location_data_vector(rhs._location_data_vector),
	_listening_socket(rhs._listening_socket),
	_addr(rhs._addr) {
	std::cout << PUR << "ServerData Copy constructor" << RES << std::endl;
}

/** Destructor */
ServerData::~ServerData() {
	/** Cleaning default values for the server block */
	_server_name.clear();
	_listens_to.clear();
	_root_directory.clear();
	_index_file.clear();
	_client_max_body_size = 0;
	_error_page.clear();
	std::cout << PUR << "ServerData Destructor" << RES << std::endl;
	std::cout << "addr address: " << _addr << std::endl;
	if (_addr != NULL)
	{
		if (signalCall == true)
			freeaddrinfo(_addr);
		_addr = NULL;
	}
	// std::cout << "free addr has been run!\n";  // TO BE CLEANED
	// if (_addr != NULL)
	// 	std::cout << "_addr not null after free\n";
}

/** #################################### Getters #################################### */

std::string ServerData::getServerName() const {
	return _server_name;
}

std::string ServerData::getListensTo() const {
	return _listens_to;
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

std::vector<std::string> ServerData::getErrorPages() const {
	return _error_page;
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

/*
 * To add a hostname or IP address to your macOS machine, you can modify the "hosts" file located at /etc/hosts.
 * This file maps hostnames to IP addresses and is used by the operating system to resolve domain names to IP addresses locally, without querying a DNS server.
 * To edit the hosts file, you can follow these steps:
 * 1. Open /etc/hosts (it requires sudo rights to modify system files)
 * 2. Add a new line for the hostname or IP address you want to add. The format of the line should be: <IP address> <hostname>
 *    ex: 192.168.0.1 example.com
 *    2.1. Or add a new hostname to an existing IP address
 *         ex:127.0.0.1 localhost example.com.br
 * 3. Finally, flush the DNS cache by typing the following command in the Terminal: sudo dscacheutil -flushcache
 *    This will ensure that the updated hosts file is used by the operating system.
 * After making these changes, you should be able to use the hostname or IP address you added to access the associated network resource on your Mac.
 */
void ServerData::setServerName(std::string const & serverName) {
//    /* not mandatory | default: 127.0.0.1 */ todo it now can also be the ip address
//    if (not ip.empty()) {
//        struct sockaddr_in sockAddr;
//        if (inet_pton(AF_INET, ip.c_str(), &(sockAddr.sin_addr))) {
//            _ip_address = ip;// TODO _ip_address = inet_addr(ip.c_str()); ???
//
//        } else {
//            throw ParserException(CONFIG_FILE_ERROR("ip_address", NOT_SUPPORTED));
//        }
//    }
	/* not mandatory | default: localhost */
	if (not serverName.empty()) {
		if (std::all_of(serverName.begin(), serverName.end(), isServerNameValid)) {
			_server_name = serverName;
		} else {
			throw ParserException(CONFIG_FILE_ERROR("server_name", NOT_SUPPORTED));
		}
	}
}
//    WTF? LOL (I was trying to do my self first -  I will keep it comment out for now in case we can't use inet_pton
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
void ServerData::setListensTo(std::string const & port) {
	/* not mandatory | default 80 */
	if (not port.empty()) {
		try {
			unsigned short const & listensToPort = std::strtol(port.c_str(), nullptr, 10);
			if (listensToPort == 4242 || listensToPort == 4243 || listensToPort == 8080 || listensToPort >= 49152) {// todo:: add 65536 as acceptable? then change form short to int? -> WHAT ABOUT listensToPort == 80 || listensToPort == 591 ?????
				/* No need to check port < 65536 since port is an unsigned short already */
				_listens_to = port;
			} else {
				throw ParserException(CONFIG_FILE_ERROR("listens_to", NOT_SUPPORTED));
			}
		} catch (...) {
			throw ParserException(CONFIG_FILE_ERROR("listens_to", NOT_SUPPORTED));
		}
	}
}

void ServerData::setRootDirectory(std::string const & rootDirectory) {
	/* not mandatory | default: ./$server_name */
	if (not rootDirectory.empty() && rootDirectory != "/" && rootDirectory != "./") {
		PathType type = pathType(rootDirectory);
		if (type == DIRECTORY) {
			_root_directory = addCurrentDirPath(rootDirectory) + rootDirectory;
		} else if (type == PATH_TYPE_ERROR) {
			throw ParserException(CONFIG_FILE_ERROR("root_directory", MISSING));
		} else {
			throw ParserException(CONFIG_FILE_ERROR("root_directory", NOT_SUPPORTED));
		}
	}
}

void ServerData::setIndexFile(std::string const & indexFile) {
	/* not mandatory | default: index.html */
	if (indexFile.find('/') != std::string::npos) {
		throw ParserException(CONFIG_FILE_ERROR("index_file", NOT_SUPPORTED));
	}

	std::string index_file_copy = indexFile;
	if (indexFile.empty()) {
		index_file_copy = _index_file;
	}
	std::string index_file = addRootDirectoryPath(_root_directory, index_file_copy);
	// doesn't contain regexp (regular expressions), wildcards or full/relative path
	if (pathType(index_file) == REG_FILE) {
		_index_file = index_file_copy;
	}
}

void ServerData::setClientMaxBodySize(std::string const & bodySize) {
	/* not mandatory | default: 1024 (1KB) -> Max: INT_MAX (2GB) */
	if (not bodySize.empty()) {
		try {
			unsigned int const & body_size = std::strtol(bodySize.c_str(), nullptr, 10);
			if (body_size <= INT32_MAX) {
				_client_max_body_size = body_size;
			} else {
				throw ParserException(CONFIG_FILE_ERROR("client_max_body_size", NOT_SUPPORTED));
			}
		} catch (...) {
			throw ParserException(CONFIG_FILE_ERROR("client_max_body_size", NOT_SUPPORTED));
		}
	}
}

void ServerData::setErrorPages(std::string const & errorPage) {
    /* not mandatory | default: empty, no set error page, webserver will decide */
    if (not errorPage.empty()) {
        std::vector<std::string> error_page_vector;
        std::string error_page;
        std::stringstream ss(errorPage);
        while(getline(ss, error_page, ' ')){
            if (error_page.find(".html") != std::string::npos) {
                std::string path_error_page = addRootDirectoryPath(_root_directory, "error_pages/" + error_page);
                if (pathType(path_error_page) == REG_FILE) {
                    error_page_vector.push_back(path_error_page);
                } else {
                    throw ParserException(CONFIG_FILE_ERROR("error_page", MISSING));
                }
            } else {
                throw ParserException(CONFIG_FILE_ERROR("error_page", NOT_SUPPORTED));
            }
        }
        _error_page = error_page_vector;
    }
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

	// CHANGED:	This was causing the Siege error.
	// 			Apparently Siege wants the IP address, not ServerName 'localhost'
	//			Now Siege works.
	if (getaddrinfo(_server_name.c_str(), _listens_to.c_str(), &hints, &_addr) != 0) {
//	if (getaddrinfo(_ip_address.c_str(), _listens_to.c_str(), &hints, &_addr) != 0) {// deleted the ip_address now
		//freeaddrinfo(_addr); no need?
		throw ServerDataException("failed getaddrinfo");
	}

	// for testing memory leaks
	// int out = getaddrinfo(_server_name.c_str(), _listens_to.c_str(), &hints, &_addr);
	// out = 5;
	// if (out != 0)
	// {
	// 	std::cout << "------------------- failed addr!!!!\n";
	// 	throw ServerDataException("failed getaddrinfo");
	// }

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
