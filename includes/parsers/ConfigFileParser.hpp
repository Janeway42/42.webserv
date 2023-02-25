#ifndef CONFIGFILEPARSER_HPP
#define CONFIGFILEPARSER_HPP

#include <fstream>
#include <iostream>

#include "Parser.hpp"
#include "ServerData.hpp"
#include "ServerLocation.hpp"

class ConfigFileParser : public Parser {
    private:
        ServerData _server_data;
        unsigned short _server_block_counter;
        ServerLocation _location_data;
        unsigned short _location_block_counter;
        /* As more than 1 server block can be added (with one or more location blocks inside) */
        std::vector<ServerData> _server_vector;

        ConfigFileParser();

        /** Private Methods */
        bool handleFile(std::string const & configFileName);
        void parseFileServerBlock(std::ifstream & configFile);
        void parseFileLocationBlock(std::ifstream & configFile);

    public:
        /* explicit keyword is here so the compiler won't do implicit conversions, which means implicitly calling this
         * constructor anytime a string is given to this type (e.g.: ConfigFileParser obj; obj = std::string("Test");) todo check this*/
        explicit ConfigFileParser(std::string const & configFileName);
        virtual ~ConfigFileParser();

        /** Getters */
        std::vector<ServerData> getServers() const;
        unsigned short numberOfServerBlocks() const;
        unsigned short numberOfLocationBlocks() const;
};
#endif //CONFIGFILEPARSER_HPP
