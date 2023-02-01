#ifndef WEBSERV_REQUESTFILEPARSER_HPP
#define WEBSERV_REQUESTFILEPARSER_HPP

#include "Parser.hpp"
#include "RequestData.hpp"

namespace data {
class Request : public Parser {
    private:
        HttpRequest _data;
        std::string keyParser(std::string & lineContent, const std::string& keyToFind);

    public:
        Request();
        virtual ~Request();

        /** Getters */
        HttpRequest const & getRequestData() const;

        /** Methods */
        bool handleFile(std::string const & configFileName);
        void parseFile(std::ifstream & configFile);
        int storeWordsFromFirstLine(const std::string& firstLine);
};
} // data
#endif // WEBSERV_REQUESTFILEPARSER_HPP
