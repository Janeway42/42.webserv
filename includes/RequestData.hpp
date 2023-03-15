#ifndef REQUESTDATA_HPP
#define REQUESTDATA_HPP

#include "Parser.hpp"

#include <string>
#include <vector>
#include <map>

class RequestData {
	private:
		// FIRST LINE HEADER
		std::string _reqMethod;
		std::string _reqHttpPath;
		std::string _reqHttpVersion;

		// added
		std::string _reqHeader;
		std::string _reqBody;		// There is a function storeBody??? in RequestParser!
									// Check if it prints Yellow _data.getBody in funciton storeBody()
		std::string _reqTemp;				

		// OTHER HEADERS
		std::string _reqHost;
		std::string _reqAccept;
		int         _reqContentLength;
		std::string _reqContentType;

		// PARTS OF URL PATH AND FORM DATA
		std::string _path;                      	// ie: /kostja.se/folderA/folderB/index.html
		std::string _pathFirstPart;                 // ie: /kostja.se/folderA/folderB/
		std::string _pathLastWord;                  // ie: /index.html  OR   /folderC/
		std::string _fileExtention;                 // ie: /index.html  OR   /folderC/
		bool		_isFolder;
		std::string	_queryString;
		std::string	_cgiBody;
		std::map<std::string, std::string>	_formData;	 // maybe not needed, maybe Vector list will be enough
		std::vector<std::string>			_formList;



	public:
		RequestData();
		virtual ~RequestData();

		bool doneParsingRequest;
		bool errorRequest;

		/** Request Getters */
		const std::string getRequestMethod() const;
		const std::string getHttpPath() const;
		const std::string getHttpVersion() const;


		const std::string getHeader() const;
		const std::string getBody() const;
		const std::string getTemp() const;



		const std::string getRequestHost() const;
		const std::string getRequestAccept() const;
		size_t         	  getRequestContentLength() const;
		const std::string getRequestContentType() const;

		const std::string 							getPath( )const;
		const std::string 							getPathFirstPart( )const;	// maybe not needed
		const std::string							getPathLastWord() const;	// maybe not needed
		const std::string							getFileExtention() const;
		bool										getIsFolder() const;
		const std::string							getQueryString() const;
		const std::string							getCgiBody() const;

		std::vector<std::string>					getFormList() const;	// maybe not needed
		std::map<std::string, std::string>			getFormData() const;	// maybe not needed


	/** Request Setters */
		// First line
		void setRequestMethod(std::string reqMethod);
		void setRequestPath(std::string reqPath);
		void setHttpVersion(std::string reqHttpVersion);
		
		void setHeader(std::string reqHttpVersion);
		void setBody(std::string reqHttpVersion);
		void setTemp(std::string reqHttpVersion);



		// Other lines
		void setRequestHost(std::string reqHost);
		void setRequestAccept(std::string reqAccept);
		void setRequestContentLength(std::string reqAccept);
		void setRequestContentType(std::string reqAccept);

		// Path parts and Form Data
		void setPath(std::string path);
		void setPathFirstPart(std::string path);				// maybe not needed
		void setPathLastWord(std::string path);					// maybe not needed
		void setFileExtention(std::string path);
		void setIsFolder(bool b);
		void setQueryString(std::string path);
		void setCgiBody(std::string cgiBody);
		void setFormList(std::vector<std::string>);				// maybe not needed
		void setFormData(std::map<std::string, std::string>);	// maybe not needed

};

#endif // REQUESTDATA_HPP
