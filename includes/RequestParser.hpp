//
// Created by Joyce Macksuele on 18/01/2023.
// Adapted Jaka
//

#ifndef WEBSERV_REQUEST_PARSER_HPP
#define WEBSERV_REQUEST_PARSER_HPP

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

#include "./Parser.hpp"
#include "./RequestData.hpp"


	// std::string temp = path;
	// if (path[0] == '.')
	// 	temp = path.substr(1, std::string::npos);
	// std::size_t found = temp.find_last_of(".");
	// std::string extention = temp.substr(found, std::string::npos);
	
	//std::cout << GRN "Found Extension: [" << temp << "]\n" RES;
	//std::cout << GRN "Found Extension: [" << extention << "]\n" RES;
	

namespace data {

	class Request : public Parser {
		private:
			RequestData _data;
			std::string _header;
			std::string _body;
			std::string _temp;
        	std::string keyParser(std::string & lineContent, std::string keyToFind);
			int 		appendLastChunkToBody(std::string::size_type it);
			int 		appendToBody(std::string request);

			bool		_headerDone;
			bool		_doneParsing;
			bool 		_errorRequest;
			bool 		_earlyClose;
			std::time_t _startTime;


		public:
			Request();
			virtual ~Request();

			/** Getters */
			RequestData const & getRequestData() const;
			std::string const & getRequestBody() const;

			/** Methods */
			void    							parseHeader(std::string header);
			int									parsePath(std::string str);
			void    							appendToRequest(const char *str);
			void								storeBody(std::istringstream &iss);
			std::map<std::string, std::string>	storeFormData(std::string &pq);
			void	storePathParts_and_FormData(std::string path);
			void	storePath_and_FolderName(std::string path);


			int     checkStoredVars();
			void    printStoredRequestData(data::Request &request); // Just for checking

			int     storeWordsFromFirstLine(std::string firstLine);
			int     storeWordsFromOtherLine(std::string otherLine);

			bool getDone();
			void setDone(bool val);
			bool getError();
			void setError(bool val);
			bool getEarlyClose();
			void setEarlyClose(bool val);
			std::time_t getTime();

			std::string getTemp();
	};


} // data
#endif // WEBSERV_REQUEST_PARSER_HPP