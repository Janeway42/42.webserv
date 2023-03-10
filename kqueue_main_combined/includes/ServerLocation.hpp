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

        /** Methods */
        bool isLocationCgi() const;

        /** Getters */
        std::string getRootDirectory() const;
        std::vector<AllowMethods> getAllowMethods() const;
        std::string getIndexFile() const;
        bool getAutoIndex() const;
        std::string getInterpreterPath() const;
        std::string getScriptExtension() const;

        /** Setters */
        void setLocationAsCgi(bool isCgi);
        bool setRootDirectory(std::string const & rootDirectory);
        bool setAllowMethods(std::string const & allowMethods);
        bool setIndexFile(std::string const & indexFile);
        bool setAutoIndex(std::string const & autoIndex);
        bool setInterpreterPath(std::string const & interpreterPath);
        bool setScriptExtension(std::string const & scriptExtension);
};
#endif //SERVERLOCATION_HPP
