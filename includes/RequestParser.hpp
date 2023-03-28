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
#include "RequestParserURLpath.hpp"
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
		RequestData	_data;
		ResponseData _answer;
		CgiData     _cgi;

		bool		_headerDone;
		bool		_doneParsing;
		HttpStatus 	_httpStatus;
		bool 		_hasBody;
		ServerData  _server;	


		void        parseHeaderAndPath(std::string & tmpHeader, struct kevent event, std::string::size_type it);
		int 		appendLastChunkToBody(std::string::size_type it);
		int 		appendToBody(std::string request);
		void	    chooseMethod_StartAction(struct kevent event);

	public:
		Request();
		Request(int fd, ServerData * specificServer);
		virtual ~Request();

		/** Getters */
		ServerData & 	getServerData();  // added jaka
		RequestData &	getRequestData();
		ResponseData &	getResponseData();
		CgiData &		getCgiData();
		// std::string const & getRequestBody() const;  // moved to RequestData

		/** Methods */
		void		parseHeader(std::string header);
		int			parsePath(std::string str, struct kevent event);
		void		appendToRequest(const char *str, struct kevent event);
		void		storeBody(std::istringstream &iss);
		std::map<std::string, std::string>	storeFormData(std::string pq);
		void		storePathParts_and_FormData(std::string path);
		void		storePath_and_FolderName(std::string path);
		int			checkTypeOfFile();
		void 		callCGI(struct kevent event);
		void		runExecve(char *ENV[], char *args[], struct kevent event);





	//	int     checkStoredVars();
		void    printStoredRequestData(Request &request); // Just for checking

		int     storeWordsFromFirstLine(std::string firstLine);
		int     storeWordsFromOtherLine(std::string otherLine);

		// getters 
		bool getDone();
		HttpStatus getHttpStatus();
		int getFdClient();

		// setters
		void setDone(bool val);
		void setHttpStatus(HttpStatus val);


	//	std::string getTemp();	moved to RequestData
};

#endif // REQUEST_PARSER_HPP
