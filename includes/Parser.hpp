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
# define MAG   	 "\e[0;35m"
# define GRN     "\x1B[32m"
# define BACK    "\033[0m"

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

class Parser {
	public:
		virtual std::string getOneCleanValueFromKey(std::string & contentLine, std::string const & key);
		virtual DataType    getValueType(std::string & lineContent);
};

#endif // WEBSERV_PARSER_HPP
