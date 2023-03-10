#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <cctype> // tolower()
#include <string.h>	// for memset
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <ctime>

#include "Parser.hpp"
#include "ServerData.hpp"
#include "RequestData.hpp"
#include "ResponseData.hpp"
#include "ParseURLpath.hpp"
#include "CgiData.hpp"


	// std::string temp = path;
	// if (path[0] == '.')
	// 	temp = path.substr(1, std::string::npos);
	// std::size_t found = temp.find_last_of(".");
	// std::string extention = temp.substr(found, std::string::npos);
	
	//std::cout << GRN "Found Extension: [" << temp << "]\n" << RES;
	//std::cout << GRN "Found Extension: [" << extention << "]\n" RES;

class Request : public Parser {
    private:
		int _clientFd;
        RequestData _data;
        int 		appendLastChunkToBody(std::string::size_type it, int fdClient);
        int 		appendToBody(std::string request);

		ResponseData _answer;

		CgiData _cgi;

        bool		_headerDone;
        bool		_doneParsing;
        bool 		_errorRequest;
        bool 		_earlyClose;
        bool 		_hasBody;
        ServerData  _server;


    public:
        Request(int fd);
        virtual ~Request();

        /** Getters */
        RequestData & getRequestData();

        ResponseData & getResponseData();
		CgiData & getCgiData();
        // std::string const & getRequestBody() const;  // moved to RequestData

        /** Methods */
        void    							parseHeader(std::string header);
        int									parsePath(std::string str, int fdClient);
        void    							appendToRequest(const char *str, int fdClient);
        void								storeBody(std::istringstream &iss);
        // std::map<std::string, std::string>	storeFormData(std::string & pq);
        std::map<std::string, std::string>	storeFormData(std::string pq);
        void	storePathParts_and_FormData(std::string path);
        void	storePath_and_FolderName(std::string path);
        void 	callCGI(RequestData reqData, int fdClient);
        int		checkTypeOfFile();




    //	int     checkStoredVars();
        void    printStoredRequestData(Request &request); // Just for checking

        int     storeWordsFromFirstLine(std::string firstLine);
        int     storeWordsFromOtherLine(std::string otherLine);

        bool getDone();
        void setDone(bool val);
        bool getError();
        void setError(bool val);
		int getFdClient();

    //	std::string getTemp();	moved to RequestData
};

#endif // REQUEST_PARSER_HPP
