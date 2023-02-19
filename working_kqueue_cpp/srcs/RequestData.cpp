//
// Created by Joyce Macksuele on 1/22/23.
// Adapted Jaka
//

#include "../includes/RequestData.hpp"

namespace data {

/** Default constructor */
RequestData::RequestData() {

	/** Initializing default values for the request block */
	_reqMethod 			= "default Method";
	_reqHttpPath 		= "default Path";
    _reqHttpVersion 	= "default Version";
    _reqHost 			= "default Host";	// not sure if all these are needed
    _reqContentLength	= 0;
	// bool for the flag Done
	// also getters and setters for Done 
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

const std::string RequestData::getRequestHost() const {
	return _reqHost;
}
const std::string RequestData::getRequestAccept() const {
	return _reqAccept;
}
const int RequestData::getRequestContentLength() const {
	return _reqContentLength;
}
const std::string RequestData::getRequestContentType() const {
	return _reqContentType;
}


// PATH PARTS AND FORM DATA
const std::string RequestData::getPathFirstPart() const {
	return _pathFirstPart;
}
const std::string RequestData::getPathLastWord() const {
	return _pathLastWord;
}
const std::map<std::string, std::string> RequestData::getFormData() const {
	return _formData;
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

void RequestData::setRequestContentType(std::string reqContentType)
{
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






} // data