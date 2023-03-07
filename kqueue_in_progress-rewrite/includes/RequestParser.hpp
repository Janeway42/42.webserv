#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <cctype>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <ctime>

#include "Parser.hpp"
#include "RequestData.hpp"
#include "ResponseData.hpp"
#include "ParseURLpath.hpp"
#include "CgiData.hpp"

// std::string temp = path;
// if (path[0] == '.')
// 	temp = path.substr(1, std::string::npos);
// std::size_t found = temp.find_last_of(".");
// std::string extention = temp.substr(found, std::string::npos);

//std::cout << GRN "Found Extension: [" << temp << "]\n" RES;
//std::cout << GRN "Found Extension: [" << extention << "]\n" RES;

class Request : public Parser {
    private:
		// --- request --- 
        RequestData _data;

		// --- response --
		ResponseData _answer;

		// --- CGI -------
		CgiData _cgi;

		// --- server utils -- 
		int			_fdClient;
        bool		_cgiScript;  // change inside parsing 
		bool		_doneParsing; // doesn't mean that the server can reply to the client
		bool		_allowWrite;  // only now the server can reply to the client 
        bool 		_errorRequest;
        std::time_t _startTime;



        bool		_headerDone;
        std::string _header;
        std::string _body;
        std::string _temp;
        //std::string keyParser(std::string & lineContent, std::string keyToFind); // I think its not being used
        int 		appendLastChunkToBody(std::string::size_type it);

    public:
        Request(int fd);
        virtual ~Request();

        /** Getters */
        RequestData getRequestData() const;
        ResponseData & getResponseData();
		std::string getRequestBody() const;
		CgiData & getCgiData();
		bool getDoneParsing();
		bool getAllowWrite();
		bool getCgiScript();
        int getError();
        std::time_t getTime();
        std::string getTemp();
		int getFdClinet();


        /** Setters */
        void setCgiScript(bool val);
		void setDoneParsing(bool val);
		void setAllowWrite(bool val);
        void setError(bool val);

        /** Methods */
        void    							parseHeader(std::string header);
        int									parsePath(std::string str);
        void    							appendToRequest(const char *str);
        void								storeBody(std::istringstream &iss);
        std::map<std::string, std::string>	storeFormData(std::string pq);
        int 		                        appendToBody(std::string request);
        void	storePathParts_and_FormData(std::string path);
        void	storePath_and_FolderName(std::string path);

        int     checkStoredVars();
        void    printStoredRequestData(Request &request); // Just for checking

        int     storeWordsFromFirstLine(std::string firstLine);
        int     storeWordsFromOtherLine(std::string otherLine);
};

#endif //REQUEST_PARSER_HPP
