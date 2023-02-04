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

#include "./Parser.hpp"
#include "./RequestData.hpp"


namespace data {

	class Request : public Parser {
		private:
			RequestData _data;
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
	};

} // data
#endif // WEBSERV_REQUEST_PARSER_HPP