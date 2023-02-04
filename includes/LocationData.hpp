#ifndef WEBSERV_LOCATIONDATA_HPP
#define WEBSERV_LOCATIONDATA_HPP

#include "Parser.hpp"
#include "ServerData.hpp"

#include <string>
#include <vector>

/**
 * Directory and file (full/relative) path:
 *      ./ means $path_to_web_server_executable/
 *      a directory or file with no path (i.e.: test_index.html) will be searched on ./
 */

namespace data {
class Location {
    private:
        bool _is_location_cgi;
        std::string _root_directory;
        std::vector<AllowMethods> _allow_methods;
        std::string _index_file;
        bool _auto_index;
        std::string _interpreter_path;
        std::string _script_extension;

    public:
        Location() = delete;// delete makes it impossible to be used (anywhere by anyone) todo is it c++98?
        Location(std::string const & server_root_directory, std::string const & server_index_file);
        virtual ~Location();

        /** Methods */
        bool is_location_cgi() const;

        /** Getters */
        std::string getRootDirectory() const;
        std::vector<AllowMethods> getAllowMethods() const;
        std::string getIndexFile() const;
        bool getAutoIndex() const;
        std::string getInterpreterPath() const;
        std::string getScriptExtension() const;

        /** Setters */
        void setLocationAsCgi(bool isCgi);
        void setRootDirectory(std::string const & rootDirectory);
        void setAllowMethods(const std::vector<AllowMethods> & allowMethods);
        void setIndexFile(std::string const & indexFile);
        void setAutoIndex(bool autoIndex);
        void setInterpreterPath(std::string const & interpreterPath);
        void setScriptExtension(std::string const & scriptExtension);
    };
} // data
#endif //WEBSERV_LOCATIONDATA_HPP
