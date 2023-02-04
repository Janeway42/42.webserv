#ifndef WEBSERV_PARSER_HPP
#define WEBSERV_PARSER_HPP

#include <string>
#include <iostream>

# define RED_BG  "\033[41m"
# define RED     "\033[31m"
# define GRE     "\033[32m"
# define YEL     "\033[33m"
# define BLU     "\033[34m"
# define PUR     "\033[35m"
# define CYN     "\033[36m"
# define RES     "\033[37m"
# define BACK     "\033[0m"

/** The Parser has no access to the data namespace so it does nt have access to any data */

enum AllowMethods {
    GET = 0,
    POST,
    DELETE,
    NONE
};

enum DataType {// todo: Maybe not used
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

/** If the error message will be useful for the http response then a $responseStatusCode will be set.
 * Otherwise $responseStatusCode will be set to zero.
 */
struct parser_error {
    unsigned short responseStatusCode;// todo change it just to errorCode?
    std::string errorMessage;
} parser_error[] = {
        {   0,     "Key is not supported"},
        {   0,     "Value is not a string"},
        {   0,     "Value is not a relative or full path"},
        {   0,     "Value is not a full path"},
        {   0,     "Value is not a .html file"},
        {   0,     "Value is not an allowed port"},
        {   0,     "Value is not a string"},
        {   0,     "Value is not a string"},
        {   0,     "Value is not a string"},
        {   400,   "Value is not a string"},
        {  404,   "Key Location is not supported"},
};

class Parser {
    public:
        virtual std::string getOneCleanValueFromKey(std::string & contentLine, std::string const & key);
        virtual DataType getValueType(std::string & lineContent);

        class ParserException: public std::exception {
            private:
                std::string _errorMessage;
            public:
                explicit ParserException(struct parser_error const & error) throw() {
                    _errorMessage = std::string(std::to_string(error.responseStatusCode) + error.errorMessage);;
                }
                virtual const char* what() const throw() {
                    return (_errorMessage.c_str());
                }
                virtual ~ParserException() throw() {}
        };

//        std::pair<std::string, unsigned short> parserError(struct parser_error const error);
};

#endif //WEBSERV_PARSER_HPP
