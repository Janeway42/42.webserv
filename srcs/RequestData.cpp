#include "RequestData.hpp"

/** Default constructor */
RequestData::RequestData() {
	/* Initializing default values for the request block */
	_reqMethod 			 = NONE;
	_reqHttpPath 		 = "default Path";
    _reqHttpVersion 	 = "default Version";
	_reqServerName		 = "default Name";
	_reqPort			 = "";
	_reqHeader			 = "";
    _reqContentLength	 = 0;
	_reqHeaderBytesSoFar = 0;
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

	_responseContentType = "text/html";
	_reqSetCookie			 = "";
	_reqCookie			 = "";
}

/** Destructor */
RequestData::~RequestData() {
	/** Cleaning default values for the request block */
	_reqMethod 		= NONE;
	_reqHttpPath 	= "";
    _reqHttpVersion = "";
	_reqServerName  = "";
	_reqPort        = "";
	_reqSetCookie	= "";
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

ssize_t RequestData::getReqHeaderBytesSoFar() {
	return _reqHeaderBytesSoFar;
}

std::vector<uint8_t> & RequestData::getBody() {
	return _reqBody;
}

const std::string RequestData::getTemp() const {
	return _reqTemp;
}

ssize_t RequestData::getClientBytesSoFar() const {
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

ssize_t RequestData::getRequestContentLength() const {
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

const std::string RequestData::getRequestSetCookie() const {
	return _reqSetCookie;
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


// FOR DECODING URL
static int hexCharToInt(char hexChar) {
    if (hexChar >= '0' && hexChar <= '9') {
        return hexChar - '0';
    } else if (hexChar >= 'A' && hexChar <= 'F') {
        return hexChar - 'A' + 10;
    } else if (hexChar >= 'a' && hexChar <= 'f') {
        return hexChar - 'a' + 10;
    }
    return -1;  // Invalid hex character
}

static std::string decodeURL(std::string encodedUrl) {
    std::string decodedUrl;

    size_t len = encodedUrl.length();
    size_t i = 0;
    while (i < len) {
        if (encodedUrl[i] == '%' && i + 2 < len) {
            char hex[3] = { encodedUrl[i + 1], encodedUrl[i + 2], '\0' };
            int digit1 = hexCharToInt(hex[0]);
            int digit2 = hexCharToInt(hex[1]);
            if (digit1 != -1 && digit2 != -1) {	// Check the hexadecimal digits
                i += 2;
                int charCode = (digit1 << 4) + digit2;
                decodedUrl += static_cast<char>(charCode);
            } else {
                decodedUrl += encodedUrl[i];
            }
        } else {
            decodedUrl += encodedUrl[i];
        }
        ++i;
    }
    return decodedUrl;
}

void RequestData::setRequestPath(std::string reqPath) {
	_reqHttpPath = remove_trailing_slashes(reqPath);
	_reqHttpPath = remove_multiple_slashes(_reqHttpPath);
	_reqHttpPath = decodeURL(_reqHttpPath);
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

void RequestData::setClientBytesSoFar(ssize_t ret)
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
	_reqContentLength = stol(reqContentLength);
}

void RequestData::setResponseContentType(std::string fileExtension)
{
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
	else if (fileExtension == ".pdf")
		_responseContentType = "application/pdf";
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

void RequestData::setFileExtension(std::string path) {
    // Ex.: localhost:8080/favicon.ico or localhost:8080/cgi/python_cgi_GET.py?street=test&city=test+city or localhost:8080/index.html
    std::string urlPath = path;
    std::string extension = std::string();

    std::string::size_type hasQuery = path.find_first_of('?');
    if (hasQuery != std::string::npos) {
        urlPath = path.substr(0, hasQuery);
    }

    std::string::size_type hasExtension = urlPath.find_last_of(".");
    if (hasExtension != std::string::npos) {
        extension = urlPath.substr(hasExtension);
        std::cout << BLU << "File extension: "  << extension << RES << std::endl;
    } else {
        std::cout  << "There is no extension in the file" << std::endl;
    }
	_fileExtension = extension;
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

void RequestData::setRequestSetCookie(std::string reqSetCookie){
	_reqSetCookie = reqSetCookie;
}

void RequestData::setRequestCookie(std::string reqCookie){
	_reqCookie = reqCookie;
}
void RequestData::setReqHeaderBytesSoFar(ssize_t nrBytes) {
	_reqHeaderBytesSoFar += nrBytes;
}
