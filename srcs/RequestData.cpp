#include "RequestData.hpp"

/** Default constructor */
RequestData::RequestData() {

	/** Initializing default values for the request block */
	_reqMethod 			= "default Method";
	_reqHttpPath 		= "default Path";
    _reqHttpVersion 	= "default Version";
    _reqHost 			= "default Host";	// not sure if all these are needed
    _reqContentLength	= 0;
	
	_pathFirstPart		= "default";
	_pathLastWord		= "default";
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

std::string RequestData::getRequestMethod() const {
	return _reqMethod;
}

std::string RequestData::getHttpPath() const {
	return _reqHttpPath;
}

std::string RequestData::getHttpVersion() const {
	return _reqHttpVersion;
}

std::string RequestData::getRequestHost() const {
	return _reqHost;
}

std::string RequestData::getRequestAccept() const {
	return _reqAccept;
}

size_t RequestData::getRequestContentLength() const {
	return _reqContentLength;
}

std::string RequestData::getRequestContentType() const {
	return _reqContentType;
}

// PATH PARTS AND FORM DATA
std::string RequestData::getPathFirstPart() const {
	return _pathFirstPart;
}

std::string RequestData::getPathLastWord() const {
	return _pathLastWord;
}

std::map<std::string, std::string> RequestData::getFormData() const {	// Cannot return const
	return _formData;													// because iterator won't work
}

/** ########################################################################## */
/** Request Setters */

void RequestData::setRequestMethod(std::string reqMethod) {
	_reqMethod = reqMethod;
}

void RequestData::setRequestPath(std::string reqPath) {
	_reqHttpPath = reqPath;
}

void RequestData::setHttpVersion(std::string reqHttpVersion) {
	_reqHttpVersion = reqHttpVersion;
}

void RequestData::setRequestHost(std::string reqHost) {
	_reqHost = reqHost;
}

void RequestData::setRequestAccept(std::string reqAccept) {
	_reqAccept = reqAccept;
}

void RequestData::setRequestContentLength(std::string reqContentLength) {
	_reqContentLength = stoi(reqContentLength); // is STOI allowed ???
}

void RequestData::setRequestContentType(std::string reqContentType) {
	_reqContentType = reqContentType;
}

// PATH PARTS AND FORM DATA
void RequestData::setPathFirstPart(std::string pathFirstPart) {
	_pathFirstPart = pathFirstPart;
}

void RequestData::setPathLastWord(std::string pathLastWord) {
	_pathLastWord = pathLastWord;
}

void RequestData::setFormData(std::map<std::string, std::string> formData) {
	_formData = formData;
}
