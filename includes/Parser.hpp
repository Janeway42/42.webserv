#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <iostream>
#include <unistd.h>

# define RED_BG  "\033[41m"
# define GRN_BG  "\033[42m"
# define YEL_BG  "\033[43m"
# define BLU_BG  "\033[44m"
# define PUR_BG  "\033[45m"
# define CYN_BG  "\033[46m"
# define GRY_BG  "\033[47m"

# define RED     "\033[31m"
# define GRN     "\033[32m"
# define YEL     "\033[33m"
# define BLU     "\033[34m"
# define PUR     "\033[35m"
# define CYN     "\033[36m"
# define GRY     "\033[37m"

# define RES     "\033[0m"

// We can find more status here: https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
// I put the ones I found interesting here, but we can delete the ones we don't use later if wanted/needed
enum HttpStatus {
    NO_STATUS = 0,
    /** Information responses */
    CONTINUE = 100, // This interim response indicates that the client should continue the request or ignore the response if the request is already finished.
    PROCESSING = 102, // This code indicates that the server has received and is processing the request, but no response is available yet.
    /** Successful responses */
    OK = 200, // The request succeeded. The result meaning of "success" depends on the HTTP method.
    CREATED = 201, // The request succeeded, and a new resource was created as a result. This is typically the response sent after POST requests, or some PUT requests.
    NO_CONTENT = 204, // There is no content to send for this request, but the headers may be useful. The user agent may update its cached headers for this resource with the new ones.
    /** Redirection messages */
    MOVE_PERMANENTLY = 301, // The URL of the requested resource has been changed permanently. The new URL is given in the response.
    /** Client error responses */
    BAD_REQUEST = 400, // The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
    UNAUTHORIZED = 401, // Although the HTTP standard specifies "unauthorized", semantically this response means "unauthenticated". That is, the client must authenticate itself to get the requested response.
    FORBIDDEN = 403, // The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. Unlike 401 Unauthorized, the client's identity is known to the server.
    NOT_FOUND = 404, // The server cannot find the requested resource. In the browser, this means the URL is not recognized.
    METHOD_NOT_ALLOWED = 405, // The request method is known by the server but is not supported by the target resource. For example, an API may not allow calling DELETE to remove a resource.
    REQUEST_TIMEOUT = 408, // This response is sent on an idle connection by some servers, even without any previous request by the client. It means that the server would like to shut down this unused connection. This response is used much more since some browsers, like Chrome, Firefox 27+, or IE9, use HTTP pre-connection mechanisms to speed up surfing.
    LENGTH_REQUIRED = 411, // Server rejected the request because the Content-Length header field is not defined and the server requires it.
    CONTENT_TOO_LARGE = 413, // Request entity is larger than limits defined by server. The server might close the connection or return a Retry-After header field.
    URI_TOO_LONG = 414, // The URI requested by the client is longer than the server is willing to interpret.
    UNSUPPORTED_MEDIA_TYPE = 415, // The media format of the requested data is not supported by the server, so the server is rejecting the request.
    I_AM_A_TEAPOT = 418, // The server refuses the attempt to brew coffee with a teapot. LOL
    TOO_MANY_REQUESTS = 429, // The user has sent too many requests in a given amount of time ("rate limiting").
    HEADER_FIELDS_TOO_LARGE = 431, // The server is unwilling to process the request because its header fields are too large. The request may be resubmitted after reducing the size of the request header fields.
    /** Server error responses */
    INTERNAL_SERVER_ERROR = 500, // The server has encountered a situation it does not know how to handle (preventing the server from handling the request entirely).
    SERVICE_UNAVAILABLE = 503, // Indicates that the server is still functioning properly and can process the request, but has opted to return the 503 response code.
	GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505 // The HTTP version used in the request is not supported by the server.
};

inline const std::string httpStatusToString(HttpStatus status) {
    switch (status)
    {
        case NO_STATUS:                     return "No Status";
        case CONTINUE:                      return "Continue";
        case PROCESSING:                    return "Processing";
        case OK:                            return "OK";
        case CREATED:                       return "Created";
        case NO_CONTENT:                    return "No Content";
        case MOVE_PERMANENTLY:              return "Move Permanently";
        case BAD_REQUEST:                   return "Bad Request";
        case UNAUTHORIZED:                  return "Unauthorized";
        case FORBIDDEN:                     return "Forbidden";
        case NOT_FOUND:                     return "Not Found";
        case METHOD_NOT_ALLOWED:            return "Method Not Allowed";
        case REQUEST_TIMEOUT:               return "Request Timeout";
        case LENGTH_REQUIRED:               return "Length Required";
        case CONTENT_TOO_LARGE:             return "Content Too Long";
        case URI_TOO_LONG:                  return "URI Too Long";
        case UNSUPPORTED_MEDIA_TYPE:        return "Unsupported Media Type";
        case I_AM_A_TEAPOT:                 return "I Am A Teapot";
        case TOO_MANY_REQUESTS:             return "Too Many Requests";
        case HEADER_FIELDS_TOO_LARGE:       return "Header Fields Too Large";
        case INTERNAL_SERVER_ERROR:         return "Internal Server Error";
		case SERVICE_UNAVAILABLE:			return "Service Unavailable";
		case GATEWAY_TIMEOUT:				return "Gateway Timeout";
        case HTTP_VERSION_NOT_SUPPORTED:    return "HTTP Version Not Supported";
        default:                            return "No Status";
    }
}

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

/** Parsing error Macros */
#define NOT_SUPPORTED 0
#define MANDATORY 1
#define MISSING 2
#define CONFIG_FILE_ERROR(key, error_message) ((error_message == MANDATORY) ? (RES key RED " is mandatory") \
                                                : (error_message == NOT_SUPPORTED) ? (RES key RED" is not supported") \
                                                : (RES key RED" is missing"))

class Parser {
    private:
//        Parser() {};
    public:
        virtual std::string keyParser(std::string & lineContent, std::string const & keyToFind);
        virtual std::string getOneCleanValueFromKey(std::string & contentLine, std::string const & key);
        virtual DataType getValueType(std::string & lineContent);
        virtual PathType pathType(std::string const & path);
        virtual std::string addCurrentDirPath(std::string const & fileOrDir) const;
        virtual std::string addRootDirectoryPath(std::string const & rootDirectory, std::string const & possiblePath);
        virtual bool isSpace(char ch);

        class ParserException: public std::exception {
            private:
                std::string _errorMessage;
            public:
                explicit ParserException(std::string const & errorMessage) throw() {
                    _errorMessage = "Config file error: " + errorMessage;
                }
                virtual const char* what() const throw() {
                    return (_errorMessage.c_str());
                }
                virtual ~ParserException() throw() {}
        };
};

#endif //PARSER_HPP
