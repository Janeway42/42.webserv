#ifndef REQUESTDATA_HPP
#define REQUESTDATA_HPP

#include "Parser.hpp"

#include <string>
#include <vector>
#include <map>

class RequestData {
	private:
		int			_kqFd;

		// FIRST LINE HEADER
		AllowMethods			_reqMethod;
		std::string				_reqHttpPath;
		std::string				_reqHttpVersion;
		std::string				_reqSetCookie;
		std::string				_reqCookie;

		ssize_t					_reqHeaderBytesSoFar;		// added jaka, maybe needed for TESTER42 /r/n/r/n

		std::string 			_reqHeader;
		std::vector<uint8_t>	_reqBody;
		ssize_t					_clientBytesSoFar;	
		std::string				_reqTemp;				

		std::string				_reqServerName;
		std::string				_reqPort;

		std::string				_reqAccept;
		ssize_t					_reqContentLength;
		std::string				_requestContentType;
		std::string				_responseContentType;

		std::string				_URLPath;                      	// ie: /kostja.se/folderA/folderB/index.html
        std::string				_URLPath_full;                  // ie: /kostja.se/folderA/folderB = $location.$root_directory/folderA/folderB
		std::string				_pathFirstPart;                 // ie: /kostja.se/folderA/folderB/
		std::string				_pathLastPart;                  // ie: /index.html  OR   /folderC/
		std::string				_fileExtension;                 // ie: /index.html  OR   /folderC/
		bool					_isFolder;
		bool					_autoIndex;
		std::string				_queryString;
		std::string				_cgiBody;
		bool				    _formDataHasDelete;
		std::map<std::string, std::string>	_formData;
		std::vector<std::string>			_formList;

	public:
		RequestData();
		virtual ~RequestData();

		bool				doneParsingRequest;
		bool				errorRequest;

		//getters
		int					getKqFd() const;
		AllowMethods		getRequestMethod() const;
		const std::string	getHttpPath() const;
		const std::string	getHttpVersion() const;
		const std::string	getHeader() const;
		ssize_t				getReqHeaderBytesSoFar();	// added jaka, maybe needed for TESTER42

		const std::string	getTemp() const;
		ssize_t				getClientBytesSoFar() const;
		std::vector<uint8_t> & getBody();

		const std::string	getRequestServerName() const;
		const std::string	getRequestPort() const;

		const std::string	getRequestAccept() const;
		const std::string	getRequestContentType() const;
		const std::string	getResponseContentType() const;
		ssize_t				getRequestContentLength() const;
		const std::string	getRequestSetCookie() const;
		const std::string	getRequestCookie() const;

		const std::string 	getURLPath()const;
		const std::string 	getURLPath_full()const;
		const std::string 	getURLPathFirstPart()const;	// maybe not needed
		const std::string	getURLPathLastPart() const;	// maybe not needed
		const std::string	getFileExtension() const;
		bool				getIsFolder() const;
		bool				getAutoIndex() const;
		const std::string	getQueryString() const;
		const std::string	getCgiBody() const;
		bool	            formDataHasDelete() const;

		std::vector<std::string>			getFormList() const;	// maybe not needed
		std::map<std::string, std::string>	getFormData() const;	// maybe not needed

		//setters
		void	setKqFd(int kqFd);

		void	setRequestMethod(AllowMethods reqMethod);
		void	setRequestPath(std::string reqPath);
		void	setHttpVersion(std::string reqHttpVersion);
		void	setRequestSetCookie(std::string reqSetCookie);
		void	setRequestCookie(std::string reqCookie);
		void	setHeader(std::string str);
		void	setBody(std::vector<uint8_t> & str);
		void	setTemp(std::string str);
		void	setClientBytesSoFar(ssize_t ret);		
		void	setRequestServerName(std::string reqHost);
		void	setRequestPort(std::string reqHost);

		void	setRequestAccept(std::string reqAccept);
		void	setRequestContentLength(std::string reqAccept);
		void	setRequestContentType(std::string reqAccept);
		void	setResponseContentType(std::string reqAccept);

		void	setURLPath(std::string path);
		void	setURLPath_full(std::string path);
		void	setPathFirstPart(std::string path);				// maybe not needed
		void	setPathLastPart(std::string path);					// maybe not needed
		void	setFileExtension(std::string path);
		void	setIsFolder(bool b);
		void	setAutoIndex(bool b);
		void	setQueryString(std::string path);
		void	setCgiBody(std::string cgiBody);
		void	setFormDataHasDelete(bool b);
		void	setFormList(std::vector<std::string>);				// maybe not needed
		void	setFormData(std::map<std::string, std::string>);	// maybe not needed
		void	setReqHeaderBytesSoFar(ssize_t nrBytes);
		
};

#endif // REQUESTDATA_HPP
