//
// Created by Joyce Macksuele on 18/01/2023.
// Adapted Jaka
//

#ifndef WEBSERV_REQUESTFILEPARSER_HPP
#define WEBSERV_REQUESTFILEPARSER_HPP

#include "./Parser.hpp"
#include "./RequestData.hpp"

// #include <iostream>

# define REDB  "\033[41m" 
# define BACK  "\033[0m"


# define RED  "\033[31m"   // added jaka
# define GRE  "\033[32m"
# define YEL  "\033[33m"
# define BLU  "\033[34m"
# define PUR  "\033[35m"
# define CYN  "\033[36m"
# define RES  "\033[37m"

namespace data {

	class RequestFile : public Parser {
		private:
			HttpRequest _data;
        	std::string keyParser(std::string & lineContent, std::string keyToFind);


		public:
			RequestFile();
			virtual ~RequestFile();

			/** Getters */
			HttpRequest const & getRequestData() const;

			/** Methods */
			bool handleFile(std::string const & configFileName);
			void parseFile(std::ifstream & configFile);

			int storeWordsFromFirstLine(std::string firstLine);
	};


} // data
#endif // WEBSERV_REQUESTFILEPARSER_HPP
