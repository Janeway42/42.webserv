#ifndef WEBSERV_SERVERDATA_HPP
#define WEBSERV_SERVERDATA_HPP

#include "Parser.hpp"

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
        /* Port numbers consist of 16-bit numbers */
        unsigned short _listens_to;
        /* std::vector<std::uint8_t> since each block of an ip address is from 0 to 255. uint8_t to specify that the
         * data type is unsigned 8bits (better than writing unsigned char knowing we will receive a number) */
        std::string _ip_address;// todo: look for a better _data type?
        std::string _root_directory;
        std::string _index_file;
        unsigned int _client_max_body_size;
        std::string _error_page;
        unsigned int _port_redirection;

    public:
        ServerData();
        virtual ~ServerData();
        ServerData(ServerData const & rhs);

        /** Methods */

        /** Getters */
        std::string getServerName() const;
        unsigned int getListensTo() const;
        std::string getIpAddress() const;
        std::string getRootDirectory() const;
        std::string getIndexFile() const;
        unsigned int getClientMaxBodySize() const;
        std::string getErrorPage() const;
        unsigned int getPortRedirection() const;

        /** Setters */
        bool setServerName(std::string const & name);
        bool setListensTo(std::string const & port);
        bool setIpAddress(std::string const & ip);
        bool setRootDirectory(std::string const & root_dir);
        bool setIndexFile(std::string const & idx_file);
        bool setClientMaxBodySize(std::string const & body_size);
        bool setErrorPage(std::string const & err_page);
        bool setPortRedirection(std::string const & port_redir);
};
#endif //WEBSERV_SERVERDATA_HPP
