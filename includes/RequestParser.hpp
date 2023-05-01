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
		std::string		_redirection;
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
		std::string		getRedirection();
		bool			getDone();
		int				getFdClient();
		int				getKq();

		// setters
		void			setDone(bool val);
		void			setHttpStatus(HttpStatus val);
		void			setRedirection(std::string const & redirection);

		// methods
		std::map<std::string, std::string>	storeFormData(std::string pq);
		void			    parseHeader(std::string header);
		void    		    parsePath(std::string const & originalUrlPath);
        void                checkRedirection(std::string getRedirection);
		std::string	        parsePath_locationMatch(std::string const & originalUrlPath);
        std::string		    parsePath_file(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location);
        std::string     	parsePath_cgi(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location, std::string const & file_cgi);
        std::string		    parsePath_dir(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location, std::string const & subdirectory);
        std::string         parsePath_regularCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location);
        std::string         parsePath_root(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location);
        std::string         parsePath_edgeCase(std::string const & originalUrlPath, std::vector<ServerLocation>::const_iterator & location);//todo mayne not needed?
		void		    	appendToRequest(const char str[], size_t len);
		void			    storeURLPathParts(std::string const & originalUrlPath, std::string const & newUrlPath);
		void                checkIfPathExists(std::string const & path);
		void 			    callCGI(struct kevent event);
		void			    runExecve(char *ENV[], char *args[], struct kevent event);
		int				    storeWordsFromFirstLine(std::string firstLine);
		int				    storeWordsFromOtherLine(std::string otherLine);

		std::string			getErrorPage();


		// cleanup ------------------------
		void    printStoredRequestData(Request &request); // Just for checking
		//void		storeBody(std::istringstream &iss);
		//	int     checkStoredVars();
		//	std::string getTemp();	moved to RequestData
};

#endif // REQUEST_PARSER_HPP
