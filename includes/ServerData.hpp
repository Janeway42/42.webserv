#ifndef WEBSERV_SERVERDATA_HPP
#define WEBSERV_SERVERDATA_HPP

#include "Parser.hpp"
#include "includes/LocationData.hpp"

/**
 * Directory and file (full/relative) path:
 *      ./ means $path_to_web_server_executable/
 *      a directory or file with no path (i.e.: test_index.html) will be searched on ./
 */

namespace data {
class Server {
    private:
        // todo number of server blocks on the config file
        bool _server_block;
        std::string _server_name;
        unsigned int _listens_to;
        /* std::uint8_t since each block of an ip address s form 0 to 255 */
        std::string _ip_address;// todo look for a better _data type?
        std::string _root_directory;
        std::string _index_file;
        unsigned int _client_max_body_size;
        std::string _error_page;
        unsigned int _port_redirection;
        /** As more than 1 server block can be added, a linked list makes more sense */
//        class server * _next_server;
//        std::unordered_map<Server, std::vector<Location> > _server_map;

    public:
        Server();
        virtual ~Server();

        /** server Getters */
        bool hasServerBlock() const;
        std::string getServerName() const;
        unsigned int getListensTo() const;
        std::string getIpAddress() const;
        std::string getRootDirectory() const;
        std::string getIndexFile() const;
        unsigned int getClientMaxBodySize() const;
        std::string getErrorPage() const;
        unsigned int getPortRedirection() const;

        /** server Setters */
        void setHasServerBlock(unsigned int server_block);
        void setServerName(std::string const & name);
        void setListensTo(unsigned int const & port);
        void setIpAddress(std::string const & ip);
        void setRootDirectory(std::string const & root_dir);
        void setIndexFile(std::string const & idx_file);
        void setClientMaxBodySize(unsigned int const & body_size);
        void setErrorPage(std::string const & err_page);
        void setPortRedirection(unsigned int const & port_redir);

};
} // data
#endif // WEBSERV_SERVERDATA_HPP
