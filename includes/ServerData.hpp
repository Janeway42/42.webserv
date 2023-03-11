#ifndef SERVERDATA_HPP
#define SERVERDATA_HPP

#include "Parser.hpp"
#include "ServerLocation.hpp"

/**
 * An IP address identifies a machine in an IP network and is used to determine the destination of a data packet.
 * Port numbers identify a particular application or service on a system.
 *
 * Directory and file (full/relative) path:
 *      ./ means $path_to_web_server_executable/
 *      a directory or file with no path (i.e.: test_index.html) will be searched on ./
 */

class ServerData : public Parser {
    private:
        std::string _server_name;
        std::string _listens_to;
        std::string _ip_address;
        std::string _root_directory;
        std::string _index_file;
        unsigned int _client_max_body_size;
        std::string _error_page;
        unsigned int _port_redirection;
        /* As more than 1 location block can be added to a server block */
        std::vector<ServerLocation> _location_data_vector;

        int _listening_socket;
        struct addrinfo *_addr;

    public:
        ServerData();
        virtual ~ServerData();
        ServerData(ServerData const & rhs);

        /** Methods */

        /** Getters */
        std::string getServerName() const;
        std::string getListensTo() const;
        std::string getIpAddress() const;
        std::string getRootDirectory() const;
        std::string getIndexFile() const;
        unsigned int getClientMaxBodySize() const;
        std::string getErrorPage() const;
        unsigned int getPortRedirection() const;
        std::vector<ServerLocation> & getLocationBlocks();
        int getListeningSocket() const;
        struct addrinfo* getAddr() const;

        /** Setters */
        bool setServerName(std::string const & name);
        bool setListensTo(std::string const & port);
        bool setIpAddress(std::string const & ip);
        bool setRootDirectory(std::string const & root_dir);
        bool setIndexFile(std::string const & idx_file);
        bool setClientMaxBodySize(std::string const & body_size);
        bool setErrorPage(std::string const & err_page);
        bool setPortRedirection(std::string const & port_redir);
//        void addToLocationVector(std::vector<ServerLocation> const & location_data);
        void setListeningSocket();

        class ServerDataException: public std::exception {
        private:
            std::string _errorMessage;
        public:
            explicit ServerDataException(std::string const & errorMessage) throw() {
                _errorMessage = "ServerData error: " + errorMessage;// todo: maybe add a number to it? and make it an array, std::pair map?
            }
            virtual const char* what() const throw() {
                return (_errorMessage.c_str());
            }
            virtual ~ServerDataException() throw() {}
        };
};
#endif //SERVERDATA_HPP
