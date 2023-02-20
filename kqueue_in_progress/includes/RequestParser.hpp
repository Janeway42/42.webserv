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
#include "./RequestResponse.hpp"

namespace data {

	class Request : public Parser {
		private:
			// request:
			RequestData _data;

			// response
			Response _answer;

			// server utils
			bool _doneParsing;
			int _errorRequest;
			bool _earlyClose;
			std::time_t _startTime;

			// to be relocated
			std::string _header;
			std::string _temp;
			bool _headerDone;
			std::string _body;
        	std::string keyParser(std::string & lineContent, std::string keyToFind);
			int appendLastChunkToBody(std::string::size_type it);
			int appendToBody(std::string request);



			


		public:
			Request();
			virtual ~Request();

			/** Getters */
			RequestData const & getRequestData() const;
			std::string const & getRequestBody() const;

			/** Methods */
			void    parseHeader(std::string header);
			void    appendToRequest(const char *str);

			int     checkStoredVars();
			void    printStoredRequestData(data::Request &request); // Just for checking

			int     storeWordsFromFirstLine(std::string firstLine);
			int     storeWordsFromOtherLine(std::string otherLine);

			// Getters ----------------------------------------------------------------

			Response getAnswer(void);

			std::string getTemp();

			bool getDone();
			void setDone(bool val);
			int getError();
			bool getEarlyClose();
			std::time_t getTime();

			// Setters ----------------------------------------------------------------
			void setError(int val);
			void setEarlyClose(bool val);
	};

	

} // data
#endif // WEBSERV_REQUEST_PARSER_HPP