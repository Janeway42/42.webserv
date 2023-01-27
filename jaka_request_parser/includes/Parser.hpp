//
// Created by Joyce Macksuele on 1/22/23.
//

#ifndef WEBSERV_PARSER_HPP
#define WEBSERV_PARSER_HPP

#include <string>
#include <iostream>


/** The Parser has no access to the data namespace so it does nt have access to any data */

enum AllowMethods {
    GET = 0,
    POST,
    DELETE,
    NONE
};

enum DataType {
    STRING = 0,
    PORT,
    IP_ADDRESS,
    ABSOLUTE_PATH,
    RELATIVE_PATH,
    ABSOLUTE_PATH_HTML,
    RELATIVE_PATH_HTML,
    BYTES,
    HTTP_METHOD,
    BOOL,
    EXTENSION,
    HTTP_VERSION,
    HOST,
    BODY
};

class Parser {
    public:
        virtual std::string getOneCleanValueFromKey(std::string & contentLine, std::string const & key);
        virtual DataType getValueType(std::string & lineContent);
};

#endif //WEBSERV_PARSER_HPP
