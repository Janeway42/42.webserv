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
        std::string _root_directory;
        std::string _index_file;
        unsigned int _client_max_body_size;
        std::vector<std::string> _error_page;
        /* As more than 1 location block can be added to a server block */
        std::vector<ServerLocation> _location_data_vector;

        int _listening_socket;
        struct addrinfo *_addr;

    public:
        ServerData();
        virtual ~ServerData();
        ServerData(ServerData const & rhs);

        /** Getters */
        std::string getServerName() const;
        std::string getListensTo() const;
        std::string getRootDirectory() const;
        std::string getIndexFile() const;
        unsigned int getClientMaxBodySize() const;
        std::vector<std::string> getErrorPages() const;
        std::vector<ServerLocation> & getLocationBlocks();
        int getListeningSocket() const;
        struct addrinfo* getAddr() const;

        /** Setters */
        void setServerName(std::string const & serverName);
        void setListensTo(std::string const & port);
        void setRootDirectory(std::string const & rootDirectory);
        void setIndexFile(std::string const & indexFile);
        void setClientMaxBodySize(std::string const & bodySize);
        void setErrorPages(std::string const & errorPage);
        void setListeningSocket();

        class ServerDataException: public std::exception {
            private:
                std::string _errorMessage;
            public:
                ServerDataException(std::string const & errorMessage) throw() {
                    _errorMessage = "ServerData error: " + errorMessage;
                }
                virtual const char* what() const throw() {
                    return (_errorMessage.c_str());
                }
                virtual ~ServerDataException() throw() {}
        };
};
#endif //SERVERDATA_HPP
