#ifndef WEBSERV_REQUESTDATA_HPP
#define WEBSERV_REQUESTDATA_HPP

#include "Parser.hpp"

namespace data {
class HttpRequest {
    private:
        // FIRST LINE HEADER
        std::string _reqMethod;
        std::string _reqHttpPath;
        std::string _reqHttpVersion;
        // OTHER HEADERS
        std::string _reqHost;
        std::string _reqAccept;


    public:
        HttpRequest();
        virtual ~HttpRequest();

        /** Request Getters */
        const std::string getRequestMethod() const;
        const std::string getHttpPath() const;
        const std::string getHttpVersion() const;
        const std::string getRequestHost() const;
        const std::string getRequestAccept() const;



        /** Request Setters */
        void setRequestMethod(std::string reqMethod);
        void setRequestPath(std::string reqPath);
        void setHttpVersion(std::string reqHttpVersion);
        void setRequestHost(std::string reqHost);
        void setRequestAccept(std::string reqAccept);

};
} // data
#endif //WEBSERV_REQUESTDATA_HPP
