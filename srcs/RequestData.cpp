#include "../includes/RequestData.hpp"

namespace data {
/** Default constructor */
HttpRequest::HttpRequest() {

	/** Initializing default values for the request block */
	_reqMethod 		= "default Method";
	_reqHttpPath 	= "default Path";
    _reqHttpVersion = "default Version";
    _reqHost 		= "default Host";			// not sure if these are needed

}

/** Destructor */
HttpRequest::~HttpRequest() {

	/** Cleaning default values for the request block */
	_reqMethod 		= "";
	_reqHttpPath 	= "";
    _reqHttpVersion = "";
    _reqHost 		= "";
	// _next = nullptr;
}

/** ########################################################################## */

/** Request Getters */

const std::string HttpRequest::getRequestMethod() const {
	return _reqMethod;
}

const std::string HttpRequest::getHttpPath() const {
	return _reqHttpPath;
}

const std::string HttpRequest::getHttpVersion() const {
	return _reqHttpVersion;
}

const std::string HttpRequest::getRequestHost() const {
	return _reqHost;
}

const std::string HttpRequest::getRequestAccept() const {
	return _reqAccept;
}



/** ########################################################################## */
/** Request Setters */

void HttpRequest::setRequestMethod(std::string reqMethod)
{
	_reqMethod = reqMethod;
}

void HttpRequest::setRequestPath(std::string reqPath)
{
	_reqHttpPath = reqPath;
}

void HttpRequest::setHttpVersion(std::string reqHttpVersion)
{
	_reqHttpVersion = reqHttpVersion;
}

void HttpRequest::setRequestHost(std::string reqHost)
{
	_reqHost = reqHost;
}

void HttpRequest::setRequestAccept(std::string reqAccept)
{
	_reqAccept = reqAccept;
}

} // data
