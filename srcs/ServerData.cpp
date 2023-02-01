#include "includes/ServerData.hpp"

namespace data {
/** Default constructor */
Server::Server() {
    /** Initializing default values for the server block */
    _server_block = false;
    _server_name = "localhost";
    _listens_to = 0;
    _ip_address = "127.0.0.1";
    _root_directory = "./localhost";
    _index_file = "index.html";
    _client_max_body_size = 1024;
    /* default: $root_directory/error_pages/$status_code.html -> $status_code.html will be set later */
    _error_page = _root_directory + "/error_pages";
    _port_redirection = _listens_to;
    /* Only one server block by default */
//    _next_server = nullptr;
}

/** Destructor */
Server::~Server() {
    /** Cleaning default values for the server block */
    _server_block = false;
    _server_name = "";
    _listens_to = 0;
    _ip_address = std::string();
    _root_directory = std::string();
    _index_file = std::string();
    _client_max_body_size = 0;
    _error_page = std::string();
    _port_redirection = 0;
//    _next_server = nullptr;
}

/** #################################### Getters #################################### */

bool Server::hasServerBlock() const {
    return _server_block;
}

std::string Server::getServerName() const {
    return _server_name;
}

unsigned int Server::getListensTo() const {
    return _listens_to;
}

std::string Server::getIpAddress() const {
    return _ip_address;
}

std::string Server::getRootDirectory() const {
    return _root_directory;
}

std::string Server::getIndexFile() const {
    return _index_file;
}

unsigned int Server::getClientMaxBodySize() const {
    return _client_max_body_size;
}

std::string Server::getErrorPage() const {
    return _error_page;
}

unsigned int Server::getPortRedirection() const {
    return _port_redirection;
}

/** #################################### Setters #################################### */

void Server::setHasServerBlock(unsigned int server_block) {
    _server_block = server_block;
}

void Server::setServerName(const std::string &name) {
    _server_name = name;
}

void Server::setListensTo(const unsigned int &port) {
    _listens_to = port;
}

void Server::setIpAddress(const std::string &ip) {
    _ip_address = ip;
}

void Server::setRootDirectory(const std::string &root_dir) {
    _root_directory = root_dir;
}

void Server::setIndexFile(const std::string &idx_file) {
    _index_file = idx_file;
}

void Server::setClientMaxBodySize(const unsigned int &body_size) {
    _client_max_body_size = body_size;
}

void Server::setErrorPage(const std::string &err_page) {
    _error_page = err_page;
}

void Server::setPortRedirection(const unsigned int &port_redir) {
    _port_redirection = port_redir;
}
} // data
