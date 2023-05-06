#include "RequestData.hpp"

/** Default constructor */
RequestData::RequestData() {
	/* Initializing default values for the request block */
	_reqMethod 			 = "default Method";
	_reqHttpPath 		 = "default Path";
    _reqHttpVersion 	 = "default Version";
    _reqHost 			 = "default Host";	// not sure if all these are needed
	_reqHeader			 = "";
    _reqContentLength	 = 0;
	_clientBytesSoFar	 = 0;
	//_reqBody			 = "";		// does not need setting to "", it is now a vector

    _URLPath			 = "default";
    _URLPath_full = "default";
	_pathFirstPart		 = "default";
	_pathLastPart		 = "default";
	_fileExtension		 = "";
	_isFolder			 = false;
	_autoIndex			 = false;
	_queryString		 = "";
	_cgiBody			 = "";

	_responseContentType 	= "text/html";

	_reqCookies			= "";
//	_formList			   = NULL;	// ???
//	_formData			   = NULL;	// ???
}

/** Destructor */
RequestData::~RequestData() {
	/** Cleaning default values for the request block */
	_reqMethod 		= "";
	_reqHttpPath 	= "";
    _reqHttpVersion = "";
    _reqHost 		= "";	
	// _next = nullptr;
    // todo finish unsetting?
}

/** ########################################################################## */
/** Request Getters */
int	 RequestData::getKqFd() const {
	return _kqFd;
}

const std::string RequestData::getRequestMethod() const {
	return _reqMethod;
}

const std::string RequestData::getHttpPath() const {
	return _reqHttpPath;
}

const std::string RequestData::getHttpVersion() const {
	return _reqHttpVersion;
}

const std::string RequestData::getHeader() const {
	return _reqHeader;
}

// const std::string RequestData::getBody() const {
// 	return _reqBody;
// }
std::vector<uint8_t> & RequestData::getBody() {
	return _reqBody;
}

const std::string RequestData::getTemp() const {
	return _reqTemp;
}

size_t RequestData::getClientBytesSoFar() const {
	return _clientBytesSoFar;
}

const std::string RequestData::getRequestHost() const {
	return _reqHost;
}

const std::string RequestData::getRequestAccept() const {
	return _reqAccept;
}

size_t RequestData::getRequestContentLength() const {
	return _reqContentLength;
}

const std::string RequestData::getResponseContentType() const {
	return _responseContentType;
}

const std::string RequestData::getRequestContentType() const {
	return _requestContentType;
}

// PATH PARTS AND QUERY_STRING (FORM DATA)
const std::string RequestData::getURLPath() const {
	return _URLPath;
}

const std::string RequestData::getURLPath_full() const {
    return _URLPath_full;
}

const std::string RequestData::getURLPathFirstPart() const {
	return _pathFirstPart;
}

const std::string RequestData::getURLPathLastPart() const {
	return _pathLastPart;
}

const std::string RequestData::getFileExtension() const {
	return _fileExtension;
}

bool RequestData::getIsFolder() const {
	return _isFolder;
}

bool RequestData::getAutoIndex() const {
	return _autoIndex;
}

const std::string RequestData::getQueryString() const {
	return _queryString;
}

const std::string RequestData::getCgiBody() const {
	return _cgiBody;
}

std::map<std::string, std::string> RequestData::getFormData() const {	// Cannot return const
	return _formData;													// because iterator won't work
}

std::vector<std::string> RequestData::getFormList() const {	// Cannot return const
	return _formList;										// because iterator won't work
}

const std::string RequestData::getRequestCookies() const {
	return _reqCookies;
}

/** ########################################################################## */
/** Request Setters */

void RequestData::setKqFd(int kqFd)
{
	_kqFd = kqFd;
}

void RequestData::setRequestMethod(std::string reqMethod)
{
	_reqMethod = reqMethod;
}



// added jaka: to remove slashes at the end of path. Just leave 1 slash.
// It was causing a problem with autoindex structure
std::string remove_trailing_slashes(std:: string str) {
    std::string::reverse_iterator rit;
    for (rit = str.rbegin(); rit != str.rend(); rit++) {
        if (*rit != '/') {
            //str.erase(rit.base(), str.end());
            break ;
        }
    }
    return (str);
}

// added jaka
std::string remove_multiple_slashes(std::string str) {
    for (std::string::iterator it = str.begin(); it != str.end();) {
        if (*it == '/') {
            std::string::iterator next_it = std::next(it);
            while (next_it != str.end() && *next_it == '/')
                next_it = str.erase(next_it);
        }
        if (it != str.end())
            ++it;
    }
    return str;
}


void RequestData::setRequestPath(std::string reqPath)
{
	_reqHttpPath = remove_trailing_slashes(reqPath);
	_reqHttpPath = remove_multiple_slashes(_reqHttpPath);
	// std::cout << RED "AFTER REMOVED SLASHES: [" << _reqHttpPath << RES "]\n"; 
}

void RequestData::setHttpVersion(std::string reqHttpVersion)
{
	_reqHttpVersion = reqHttpVersion;
}

// void RequestData::setBody(std::string reqBody)
// {
// 	_reqBody = reqBody;
// }
void RequestData::setBody(std::vector<uint8_t> & reqBody)
{
	_reqBody = reqBody;
}

void RequestData::setTemp(std::string reqTemp)
{
	_reqTemp = reqTemp;
}

void RequestData::setClientBytesSoFar(size_t ret)
{
	_clientBytesSoFar += ret;
}

void RequestData::setHeader(std::string reqHeader)
{
	_reqHeader = reqHeader;
}

void RequestData::setRequestHost(std::string reqHost)
{
	_reqHost = reqHost;
}

void RequestData::setRequestAccept(std::string reqAccept)
{
	_reqAccept = reqAccept;
}

void RequestData::setRequestContentLength(std::string reqContentLength)
{
	_reqContentLength = stoi(reqContentLength); // is STOI allowed ???
}

void RequestData::setResponseContentType(std::string fileExtension)
{
    // todo there are no other extensions that people could try to serve??????
	if (fileExtension == ".html")
		_responseContentType = "text/html";
	else if (fileExtension == ".jpg")
		_responseContentType = "image/jpg";
	else if (fileExtension == ".png")
		_responseContentType = "image/png";
	else if (fileExtension == ".ico")
		_responseContentType = "image/x-con";
//	else if (fileExtension == "")// Not sure if needed, -> joyce comment -> when http://localhost:8080/_folderA it was comming here and so the response body with the 404 error was nor being set, Idk if this line is needed for something else though, if not we can delete (is it text/html the default anyway? response has always to have any content type???)
//		_responseContentType = "";// in case of no suffix
}

void RequestData::setRequestContentType(std::string str) {
	_requestContentType = str;
}

// PATH PARTS AND QUERY_STRING (FORM DATA)
void RequestData::setURLPath(std::string path) {
    _URLPath = path;
}

void RequestData::setURLPath_full(std::string path) {
    _URLPath_full = path;
}

void RequestData::setPathFirstPart(std::string pathFirstPart) {
	_pathFirstPart = pathFirstPart;
}

void RequestData::setPathLastPart(std::string pathLastPart) {
	_pathLastPart = pathLastPart;
}

void RequestData::setFileExtension(std::string fileExtension) {
	_fileExtension = fileExtension;
}

void RequestData::setIsFolder(bool b) {
	_isFolder = b;
}

void RequestData::setAutoIndex(bool b) {
	_autoIndex = b;
}

void RequestData::setQueryString(std::string queryString) {
	_queryString = queryString;
}

void RequestData::setCgiBody(std::string cgiBody) {
	_cgiBody = cgiBody;
}

void RequestData::setFormData(std::map<std::string, std::string> formData) {
	_formData = formData;
}

void RequestData::setFormList(std::vector<std::string> formList) {
	_formList = formList;
}
