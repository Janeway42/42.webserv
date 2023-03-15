#ifndef CONFIGFILEPARSER_HPP
#define CONFIGFILEPARSER_HPP

#include <fstream>
#include <iostream>

#include "Parser.hpp"
#include "ServerData.hpp"
#include "ServerLocation.hpp"

class ConfigFileParser : public Parser {
    private:
//        ServerData _server_data;
//        std::vector<ServerLocation> serverData;
//        ServerLocation _server_location;
//        std::vector<ServerLocation> _location_data_vector;
        unsigned short _server_block_counter;
        unsigned short _location_block_counter;

//        ConfigFileParser() = default;

        /** Private Methods */
        void handleFile(std::string const & configFileName);
        void parseFileServerBlock(std::ifstream & configFile);
        void parseFileLocationBlock(std::ifstream & configFile, ServerData & _server_data, ServerLocation & _server_location);

    public:
        ConfigFileParser(std::string const & configFileName);
        /* explicit keyword is here so the compiler won't do implicit conversions, which means implicitly calling this
         * constructor anytime a string is given to this type (e.g.: ConfigFileParser obj; obj = std::string("Test");) todo check this*/
//        ConfigFileParser(std::string const & configFileName);
        virtual ~ConfigFileParser();

        /* As more than 1 server block can be added (with one or more location blocks inside) */
        std::vector<ServerData> servers;

        /** Getters */
        unsigned short numberOfServerBlocks() const;
        unsigned short numberOfLocationBlocks() const;
};
#endif //CONFIGFILEPARSER_HPP
