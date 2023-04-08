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
		int _kq;
		int				_clientFd;
		RequestData		_data;
		ResponseData	_answer;
		CgiData     	_cgi;

		bool			_headerDone;
		bool			_doneParsing;
		bool 			_hasBody;
		HttpStatus		_httpStatus;
		ServerData  	*_server;

		void        parseHeaderAndPath(std::string & tmpHeader, std::string::size_type it);
		int 		appendLastChunkToBody2(const char *str, size_t len);
		int 		appendToBody(const char *str, size_t len);

		// cleanup ---------------------------
		// void	    chooseMethod_StartAction(struct kevent event);  // Doesn't seem to be in use anymore - to be cleaned
		//int 		appendLastChunkToBody(std::string::size_type it);
		// int 		appendToBody(std::string request);

	public:
		Request();
		Request(int kq, int fd, ServerData * specificServer);
		virtual ~Request();

		// getters
		ServerData & 	getServerData();
		RequestData &	getRequestData();
		ResponseData &	getResponseData();
		CgiData &		getCgiData();
		HttpStatus		getHttpStatus();
		bool			getDone();
		int				getFdClient();
		int				getKq();

		// setters
		void			setDone(bool val);
		void			setHttpStatus(HttpStatus val);

		// methods
		std::map<std::string, std::string>	storeFormData(std::string pq);
		void			parseHeader(std::string header);
		HttpStatus		parsePath(std::string str);
		void			appendToRequest(const char str[], size_t len);
		void			storePathParts_and_FormData(std::string path);
		void			storePath_and_FolderName(std::string const & originalUrlPath, std::string const & newUrlPath);
		HttpStatus      checkIfPathExists(std::string const path);
		void 			callCGI(struct kevent event);
		void			runExecve(char *ENV[], char *args[], struct kevent event);
		int				storeWordsFromFirstLine(std::string firstLine);
		int				storeWordsFromOtherLine(std::string otherLine);


		// cleanup ------------------------
		void    printStoredRequestData(Request &request); // Just for checking
		//void		storeBody(std::istringstream &iss);
		//	int     checkStoredVars();
		//	std::string getTemp();	moved to RequestData
};

#endif // REQUEST_PARSER_HPP
