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
		std::string				_reqMethod;
		std::string				_reqHttpPath;
		std::string				_reqHttpVersion;

		std::string 			_reqHeader;
		std::vector<uint8_t>	_reqBody;
		size_t					_clientBytesSoFar;	
		std::string				_reqTemp;				

		std::string				_reqHost;
		std::string				_reqAccept;
		int						_reqContentLength;
		std::string				_requestContentType;
		std::string				_responseContentType;

		std::string				_path;                      	// ie: /kostja.se/folderA/folderB/index.html
        std::string				_URLPath_full;                  // ie: /kostja.se/folderA/folderB = $location.$root_directory/folderA/folderB
		std::string				_pathFirstPart;                 // ie: /kostja.se/folderA/folderB/
		std::string				_pathLastPart;                  // ie: /index.html  OR   /folderC/
		std::string				_fileExtention;                 // ie: /index.html  OR   /folderC/
		bool					_isFolder;
		std::string				_queryString;
		std::string				_cgiBody;
		std::map<std::string, std::string>	_formData;	 // maybe not needed, maybe Vector list will be enough
		std::vector<std::string>			_formList;

		// cleanup ------------------------
		// std::string 			_reqBody;	

	public:
		RequestData();
		virtual ~RequestData();

		bool				doneParsingRequest;
		bool				errorRequest;

		//getters
		int					getKqFd() const;
		const std::string	getRequestMethod() const;
		const std::string	getHttpPath() const;
		const std::string	getHttpVersion() const;
		const std::string	getHeader() const;
		const std::string	getTemp() const;
		size_t				getClientBytesSoFar() const;
		std::vector<uint8_t> & getBody();

		const std::string	getRequestHost() const;
		const std::string	getRequestAccept() const;
		const std::string	getRequestContentType() const;
		const std::string	getResponseContentType() const;
		size_t				getRequestContentLength() const;

		const std::string 	getURLPath()const;
		const std::string 	getURLPath_full()const;
		const std::string 	getURLPathFirstPart()const;	// maybe not needed
		const std::string	getURLPathLastPart() const;	// maybe not needed
		const std::string	getFileExtention() const;
		bool				getIsFolder() const;
		const std::string	getQueryString() const;
		const std::string	getCgiBody() const;

		std::vector<std::string>			getFormList() const;	// maybe not needed
		std::map<std::string, std::string>	getFormData() const;	// maybe not needed


		//setters
		void	setKqFd(int kqFd);

		void	setRequestMethod(std::string reqMethod);
		void	setRequestPath(std::string reqPath);
		void	setHttpVersion(std::string reqHttpVersion);
		void	setHeader(std::string str);
		void	setBody(std::vector<uint8_t> & str);
		void	setTemp(std::string str);
		void	setClientBytesSoFar(size_t ret);
		
		void	setRequestHost(std::string reqHost);
		void	setRequestAccept(std::string reqAccept);
		void	setRequestContentLength(std::string reqAccept);
		void	setRequestContentType(std::string reqAccept);
		void	setResponseContentType(std::string reqAccept);

		void	setURLPath(std::string path);
		void	setURLPath_full(std::string path);
		void	setPathFirstPart(std::string path);				// maybe not needed
		void	setPathLastPart(std::string path);					// maybe not needed
		void	setFileExtention(std::string path);
		void	setIsFolder(bool b);
		void	setQueryString(std::string path);
		void	setCgiBody(std::string cgiBody);
		void	setFormList(std::vector<std::string>);				// maybe not needed
		void	setFormData(std::map<std::string, std::string>);	// maybe not needed

		// cleanup ------------------------
		// const std::string getBody() const;
		// void setBody(std::string str);



};

#endif // REQUESTDATA_HPP
