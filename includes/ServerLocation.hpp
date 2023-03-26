#ifndef SERVERLOCATION_HPP
#define SERVERLOCATION_HPP

#include <vector>

#include "Parser.hpp"

/**
 * Directory and file (full/relative) path:
 *      ./ means $path_to_web_server_executable/
 *      a directory or file with no path (i.e.: test_index.html) will be searched on ./
 */

class ServerLocation : public Parser {
    private:
        bool _is_location_cgi;
        std::string _location_uri_name;
        std::string _location_cgi_extension;
        std::string _root_directory;
        std::vector<AllowMethods> _allow_methods;
        std::string _index_file;
        bool _auto_index;
        std::string _interpreter_path;
        std::string _script_extension;

    public:
//        ServerLocation() {    std::cout << BLU << "CREATING DEFAULT ServerLocation " << RES << std::endl;}
        explicit ServerLocation(std::string const & server_root_directory, std::string const & server_index_file);
        virtual ~ServerLocation();

        /** Public variables */
        bool useServerBlockIndexFile;

        /** Methods */
        bool isLocationCgi() const;

        /** Getters */
        std::string getLocationUriName() const;
        std::string getLocationCgiExtension() const;
        std::string getRootDirectory() const;
        std::vector<AllowMethods> getAllowMethods() const;
        std::string getIndexFile() const;
        bool getAutoIndex() const;
        std::string getInterpreterPath() const;
        std::string getScriptExtension() const;

        /** Setters */
        void setLocationAsCgi(bool isCgi);
        void setLocation(std::string const & location);
        void setRootDirectory(std::string const & rootDirectory);
        void setAllowMethods(std::string const & allowMethods);
        void setIndexFile(std::string const & indexFile);
        void setAutoIndex(std::string const & autoIndex);
        void setInterpreterPath(std::string const & interpreterPath);
        void setScriptExtension(std::string const & scriptExtension);
};
#endif //SERVERLOCATION_HPP
