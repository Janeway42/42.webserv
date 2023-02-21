#ifndef WEBSERV_CONFIGFILEPARSER_HPP
#define WEBSERV_CONFIGFILEPARSER_HPP

#include "Parser.hpp"
#include "ServerData.hpp"
#include "LocationData.hpp"

#include <fstream>
#include <iostream>
#include <map>

class ConfigFileParser : public Parser {
    private:
        ServerData _server_data;
        unsigned short _server_block_counter;
        LocationData _location_data;
        unsigned short _location_block_counter;
        /* As more than 1 location block can be added */
        std::vector<LocationData> _location_data_vector;
        /* As more than 1 server block can be added (with one or more location blocks inside) */
        std::map<ServerData*, std::vector<LocationData> > _server_map;

        /** Private Methods */
//        std::string keyParser(std::string & lineContent, const std::string& keyToFind);
        bool handleFile(std::string const & configFileName);
        void parseFileServerBlock(std::ifstream & configFile);
        void parseFileLocationBlock(std::ifstream & configFile);

    public:
        ConfigFileParser();//= delete; if I want no one to access a type of constructor, destructor (not even me manually) -> If I make it private I will be able to access it inside the class implementation
        /* explicit keyword is here os the compiler won't do implicit conversions. Which means implicitly calling this
         * constructor anytime a string is given to this type (without explicitly calling this constructor) */
//        explicit ConfigFileParser(std::string const & configFileName);
        virtual ~ConfigFileParser();

        /** Methods */
        std::map<ServerData*, std::vector<LocationData> > parseFile(std::string const & configFileName);
        void serverBlockCounter();
        void locationBlockCounter();

        /** Getters */
        std::map<ServerData*, std::vector<LocationData> > const & getServerDataMap() const;
        unsigned short getServerBlockCounter() const;
        unsigned short getLocationBlockCounter() const;

};
#endif //WEBSERV_CONFIGFILEPARSER_HPP
