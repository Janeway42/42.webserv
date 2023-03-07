#ifndef REQUESTDATA_HPP
#define REQUESTDATA_HPP

#include <string>
#include <vector>
#include <map>

#include "Parser.hpp"

class RequestData {
	private:
		// FIRST LINE HEADER
		std::string _reqMethod;
		std::string _reqHttpPath;
		std::string _reqHttpVersion;
		std::string _cgiScript;
		// OTHER HEADERS
		std::string _reqHost;
		std::string _reqAccept;
		int         _reqContentLength;
		std::string _reqContentType;

		// PARTS OF PATH AND FORM DATA
		std::string _pathFirstPart;                      // ie: /kostja.se/folderA/folderB/
		std::string _pathLastWord;                       // ie: /index.html  OR   /folderC/
		std::map<std::string, std::string> _formData;


	public:
		RequestData();
		virtual ~RequestData();

		bool doneParsingRequest;
		bool errorRequest;

		/** Getters */
		std::string getRequestMethod() const;
		std::string getHttpPath() const;
		std::string getHttpVersion() const;
		std::string getRequestHost() const;
		std::string getRequestAccept() const;
		size_t      getRequestContentLength() const;
		std::string getRequestContentType() const;

		std::string 						getPathFirstPart( )const;
		std::string							getPathLastWord() const;
		std::map<std::string, std::string>	getFormData() const;
		// const std::map<std::string, std::string>	getFormData() const;


	    /** Setters */
        // TODO all setters can receive a const & EX std::string const & reqMethod
		// First line
		void setRequestMethod(std::string reqMethod);
		void setRequestPath(std::string reqPath);
		void setHttpVersion(std::string reqHttpVersion);

		// Other lines
		void setRequestHost(std::string reqHost);
		void setRequestAccept(std::string reqAccept);
		void setRequestContentLength(std::string reqAccept);
		void setRequestContentType(std::string reqAccept);

		// Path parts and Form Data
		void setPathFirstPart(std::string path);
		void setPathLastWord(std::string path);
		void setFormData(std::map<std::string, std::string>);

};

#endif //REQUESTDATA_HPP
