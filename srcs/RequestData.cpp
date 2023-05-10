#include "RequestData.hpp"

/** Default constructor */
RequestData::RequestData() {
	/* Initializing default values for the request block */
	_reqMethod 			 = NONE;
	_reqHttpPath 		 = "default Path";
    _reqHttpVersion 	 = "default Version";
	_reqServerName		= "default Name";
	_reqPort			= "";
	_reqHeader			 = "";
    _reqContentLength	 = 0;
	_clientBytesSoFar	 = 0;

    _URLPath			 = "default";
    _URLPath_full 		 = "default";
	_pathFirstPart		 = "default";
	_pathLastPart		 = "default";
	_fileExtension		 = "";
	_isFolder			 = false;
	_autoIndex			 = false;
	_queryString		 = "";
	_cgiBody			 = "";

	_responseContentType 	= "text/html";
	_reqCookie			= "";
}

/** Destructor */
RequestData::~RequestData() {
	/** Cleaning default values for the request block */
	_reqMethod 		= NONE;
	_reqHttpPath 	= "";
    _reqHttpVersion = "";
	_reqServerName = "";
	_reqPort = "";
	_reqCookie		= "";
}

/** ########################################################################## */
/** Request Getters */
int	 RequestData::getKqFd() const {
	return _kqFd;
}

AllowMethods RequestData::getRequestMethod() const {
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

std::vector<uint8_t> & RequestData::getBody() {
	return _reqBody;
}

const std::string RequestData::getTemp() const {
	return _reqTemp;
}

size_t RequestData::getClientBytesSoFar() const {
	return _clientBytesSoFar;
}

const std::string RequestData::getRequestServerName() const {
	return _reqServerName;
}

const std::string RequestData::getRequestPort() const {
	return _reqPort;
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

bool RequestData::formDataHasDelete() const {
    return _formDataHasDelete;
}

std::vector<std::string> RequestData::getFormList() const {	// Cannot return const
	return _formList;										// because iterator won't work
}

const std::string RequestData::getRequestCookie() const {
	return _reqCookie;
}

/** ########################################################################## */
/** Request Setters */

void RequestData::setKqFd(int kqFd)
{
	_kqFd = kqFd;
}

void RequestData::setRequestMethod(AllowMethods reqMethod)
{
	_reqMethod = reqMethod;
}

std::string remove_trailing_slashes(std:: string str) {
    std::string::reverse_iterator rit;
    for (rit = str.rbegin(); rit != str.rend(); rit++) {
        if (*rit != '/') {
            break ;
        }
    }
    return (str);
}

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
}

void RequestData::setHttpVersion(std::string reqHttpVersion)
{
	_reqHttpVersion = reqHttpVersion;
}

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

void RequestData::setRequestServerName(std::string reqServerName)
{
	_reqServerName = reqServerName;
}

void RequestData::setRequestPort(std::string reqPort)
{
	_reqPort = reqPort;
}



void RequestData::setRequestAccept(std::string reqAccept)
{
	_reqAccept = reqAccept;
}

void RequestData::setRequestContentLength(std::string reqContentLength)
{
	_reqContentLength = stoi(reqContentLength);
}

void RequestData::setResponseContentType(std::string fileExtension)
{
    // todo there are no other extensions that people could try to serve??????
	if (fileExtension == ".html")
		_responseContentType = "text/html";
	else if (fileExtension == ".jpg")
		_responseContentType = "image/jpg";
	else if (fileExtension == ".jpeg")
		_responseContentType = "image/jpeg";
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

void RequestData::setFormDataHasDelete(bool b) {
    _formDataHasDelete = b;
}

void RequestData::setFormList(std::vector<std::string> formList) {
	_formList = formList;
}

void RequestData::setRequestCookie(std::string reqCookie){
	_reqCookie = reqCookie;
}
