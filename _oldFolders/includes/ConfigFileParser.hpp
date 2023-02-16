#ifndef WEBSERV_CONFIGFILEPARSER_HPP
#define WEBSERV_CONFIGFILEPARSER_HPP

#include "Parser.hpp"
#include "ServerData.hpp"
#include "LocationData.hpp"

#include <iostream>
#include <map>

namespace data {
class ConfigFile : public Parser {
    private:
        Server _server_data;
        unsigned short _server_block_counter;
        Location _location_data;
        unsigned short _location_block_counter;
        /* As more than 1 location block can be added */
        std::vector<Location> _location_data_vector;
        /* As more than 1 server block can be added (with one or more location blocks inside) */
        std::map<Server*, std::vector<Location> > _server_map;

        /** Private Methods */
        //template<typename T>
        std::string keyParser(std::string & lineContent, const std::string& keyToFind);
        bool handleFile(std::string const & configFileName);
        void parseFileServerBlock(std::ifstream & configFile);
        void parseFileLocationBlock(std::ifstream & configFile);

    public:
        ConfigFile();
//        explicit ConfigFile(std::string const & configFileName);// todo what is it: Clang-Tidy: Single-argument constructors must be marked explicit to avoid unintentional implicit conversions
        virtual ~ConfigFile();

        /** Methods */
        std::map<Server*, std::vector<Location> > parseFile(std::string const & configFileName);
        void serverBlockCounter();
        void locationBlockCounter();

        /** Getters */
        std::map<Server*, std::vector<Location> > const & getServerDataMap() const;
        unsigned short getServerBlockCounter() const;
        unsigned short getLocationBlockCounter() const;

    };
} // data
#endif // WEBSERV_CONFIGFILEPARSER_HPP
