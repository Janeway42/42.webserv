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
	_fileExtention		 = "";
	_isFolder			 = false;
	_autoIndex			 = false;
	_queryString		 = "";
	_cgiBody			 = "";

	_responseContentType 	= "text/html";
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

const std::string RequestData::getFileExtention() const {
	return _fileExtention;
}

bool	RequestData::getIsFolder() const {
	return _isFolder;
}

bool	RequestData::getAutoIndex() const {
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

void RequestData::setRequestPath(std::string reqPath)
{
	_reqHttpPath = reqPath;
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

void RequestData::setResponseContentType(std::string fileExtention)
{
	if (fileExtention == ".html")
		_responseContentType = "text/html";
	else if (fileExtention == ".jpg")
		_responseContentType = "image/jpg";
	else if (fileExtention == ".png")
		_responseContentType = "image/png";
	else if (fileExtention == ".ico")
		_responseContentType = "image/x-con";
	else if (fileExtention == "")			// Not sure if needed,
		_responseContentType = "";				// in case of no suffix
	//_responseContentType = str;
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

void RequestData::setFileExtention(std::string fileExtention) {
	_fileExtention = fileExtention;
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
