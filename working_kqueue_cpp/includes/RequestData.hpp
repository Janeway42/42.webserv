//
// Created by Joyce Macksuele on 1/22/23.
// Adapted by JAKA
//

#ifndef WEBSERV_REQUESTDATA_HPP
#define WEBSERV_REQUESTDATA_HPP

#include "Parser.hpp"

#include <string>
#include <vector>
#include <map>

namespace data {
class RequestData {
	private:
		// FIRST LINE HEADER
		std::string _reqMethod;
		std::string _reqHttpPath;
		std::string _reqHttpVersion;
		// OTHER HEADERS
		std::string _reqHost;
		std::string _reqAccept;
		int         _reqContentLength;
		std::string _reqContentType;

		// PARTS OF THE URL PATH
		std::string _pathFirstPart;                      // ie: /kostja.se/folderA/folderB/
		std::string _pathLastWord;                       // ie: index.html  OR   folderC
		std::map<std::string, std::string> _formData;     // maybe not needed


	public:
		RequestData();
		virtual ~RequestData();

		bool doneParsingRequest;
		bool errorRequest;

		/** Request Getters */
		const std::string getRequestMethod() const;
		const std::string getHttpPath() const;
		const std::string getHttpVersion() const;
		const std::string getRequestHost() const;
		const std::string getRequestAccept() const;
		const int         getRequestContentLength() const;
		const std::string getRequestContentType() const;

		const std::string 							getPathFirstPart( )const;
		const std::string							getPathLastWord() const;
		const std::map<std::string, std::string>	getFormData() const;


	/** Request Setters */
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

} // data
#endif //WEBSERV_REQUESTDATA_HPP