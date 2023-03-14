#include "../includes/RequestData.hpp"

/** Default constructor */
RequestData::RequestData() {

	/** Initializing default values for the request block */
	_reqMethod 			= "default Method";
	_reqHttpPath 		= "default Path";
    _reqHttpVersion 	= "default Version";
    _reqHost 			= "default Host";	// not sure if all these are needed
    _reqContentLength	= 0;
	_reqHeader			= "";
	_reqBody			= "";
	
	_path				= "default";
	_pathFirstPart		= "default";
	_pathLastWord		= "default";
	_fileExtention		= "";
	_queryString		= "";
	_cgiBody			= "";

	_reqContentType 	= "text/html";
//	_formList			= NULL;	// ???
//	_formData			= NULL;	// ???
}

/** Destructor */
RequestData::~RequestData() {
	/** Cleaning default values for the request block */
	_reqMethod 		= "";
	_reqHttpPath 	= "";
    _reqHttpVersion = "";
    _reqHost 		= "";	
	// _next = nullptr;
}



/** ########################################################################## */
/** Request Getters */


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
const std::string RequestData::getBody() const {
	return _reqBody;
}
const std::string RequestData::getTemp() const {
	return _reqTemp;
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
const std::string RequestData::getRequestContentType() const {
	return _reqContentType;
}


// PATH PARTS AND QUERY_STRING (FORM DATA)
const std::string RequestData::getPath() const {
	return _path;
}
const std::string RequestData::getPathFirstPart() const {
	return _pathFirstPart;
}
const std::string RequestData::getPathLastWord() const {
	return _pathLastWord;
}
const std::string RequestData::getFileExtention() const {
	return _fileExtention;
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

void RequestData::setBody(std::string reqBody)
{
	_reqBody = reqBody;
}
void RequestData::setTemp(std::string reqTemp)
{
	_reqTemp = reqTemp;
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

void RequestData::setRequestContentType(std::string fileExtention)
{
	if (fileExtention == ".html")
		_reqContentType = "text/html";
	else if (fileExtention == ".jpg")
		_reqContentType = "image/jpg";
	else if (fileExtention == ".png")
		_reqContentType = "image/png";
	else if (fileExtention == ".ico")
		_reqContentType = "image/x-con";
}




// PATH PARTS AND QUERY_STRING (FORM DATA)
void RequestData::setPath(std::string path) {
	_path = path;
}
void RequestData::setPathFirstPart(std::string pathFirstPart) {
	_pathFirstPart = pathFirstPart;
}
void RequestData::setPathLastWord(std::string pathLastWord) {
	_pathLastWord = pathLastWord;
}
void RequestData::setFileExtention(std::string fileExtention) {
	_fileExtention = fileExtention;
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
