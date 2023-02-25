#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <iostream>
#include <unistd.h>

# define RED_BG  "\033[41m"
# define RED     "\033[31m"
# define GRE     "\033[32m"
# define YEL     "\033[33m"
# define BLU     "\033[34m"
# define PUR     "\033[35m"
# define CYN     "\033[36m"
# define RES     "\033[37m"
# define BACK     "\033[0m"

enum PathType {
    DIRECTORY = 0,
    REG_FILE,
    SYM_LINK,
    OTHER_PATH_TYPE,
    PATH_TYPE_ERROR
};

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
//struct parser_error {
//    unsigned short responseStatusCode;// todo change it just to errorCode?
//    std::string errorMessage;
//} parserError[] = {
//        {   0,     "Key is not supported"},
//        {   0,     "Value is not a string"},
//        {   0,     "Value is not a relative or full path"},
//        {   0,     "Value is not a full path"},
//        {   0,     "Value is not a .html file"},
//        {   0,     "Value is not an allowed port"},
//        {   0,     "Value is not a string"},
//        {   0,     "Value is not a string"},
//        {   0,     "Value is not a string"},
//        {   400,   "Value is not a string"},
//        {  404,   "Key ServerLocation is not supported"},
//};

/** Error Macros */
#define NOT_SUPPORTED 0
#define MANDATORY 1
#define MISSING 2
#define CONFIG_FILE_ERROR(key, error_message) ((error_message == MANDATORY) ? (BACK key RED " is mandatory") \
                                                : (error_message == NOT_SUPPORTED) ? (BACK key RED" is not supported") \
                                                : (BACK key RED" is missing"))

class Parser {
    public:
//        Parser();
        virtual std::string keyParser(std::string & lineContent, std::string const & keyToFind);
        virtual std::string getOneCleanValueFromKey(std::string & contentLine, std::string const & key);
        virtual DataType getValueType(std::string & lineContent);
        virtual PathType pathType(std::string const & path);
        virtual std::string addCurrentDirPath(std::string const & fileOrDir) const;
        virtual std::string isPath(std::string const & rootDirectory, std::string const & possiblePath);
        virtual bool isSpace(char ch);

        class ParserException: public std::exception {
            private:
                std::string _errorMessage;
            public:
                explicit ParserException(std::string const & errorMessage) throw() {
                    _errorMessage = "Config file error: " + errorMessage;// todo: maybe add a number to it? and make it an array, std::pair map?
                }
                virtual const char* what() const throw() {
                    return (_errorMessage.c_str());
                }
                virtual ~ParserException() throw() {}
        };
};

#endif //PARSER_HPP
