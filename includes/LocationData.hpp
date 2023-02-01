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
        // todo number of location blocks inside the current server block
        bool _location_block;
        std::string _root_directory;
        std::vector<AllowMethods> _allow_methods;
        std::string _index_file;
        bool _auto_index;
        /** As more than 1 location block can be added, a linked list makes more sense */
//        class Location * _next_location;
        std::vector<Location> locationVector;


    public:
        Location();// todo make it unacceptable to construct?
        explicit Location(std::string const & server_root_directory, std::string const & server_index_file);// todo what is it: Clang-Tidy: Single-argument constructors must be marked explicit to avoid unintentional implicit conversions
        virtual ~Location();

        /** Location Methods */
//        Location nextLocationBlock(Location & currentLocation);

        /** Location Getters */
        bool hasLocationBlock() const;
        std::string getRootDirectory() const;
        std::vector<AllowMethods> getAllowMethods() const;
        std::string getIndexFile() const;
        bool getAutoIndex() const;
//        class Location * getNextBlock() const;

        /** Location Setters */
        void setHasLocationBlock(bool next_server_block);
        void setRootDirectory(std::string const & rootDirectory);
        void setAllowMethods(const std::vector<AllowMethods> & allowMethods);
        void setIndexFile(std::string const & indexFile);
        void setAutoIndex(bool autoIndex);
//        void setNextBlock(Server standardServerData);
    };
} // data
#endif // WEBSERV_LOCATIONDATA_HPP
