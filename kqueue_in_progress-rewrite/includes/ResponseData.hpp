#ifndef RESPONSEDATA_HPP
#define RESPONSEDATA_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Parser.hpp"

class ResponseData: public Parser
{
	private:
		std::string _status;
		std::string _length;
		std::string _type;
		
		std::string _responseHeader;
		std::string _responseBody;
		std::string _fullResponse;
		std::string _responsePath;

	public:
		ResponseData();
		virtual ~ResponseData();

		std::string streamFile(std::string file);

		// getters
		std::string getHeader();
		std::string getBody();
		std::string & getFullResponse();
		std::string getResponsePath();

		//setters
		void setResponse(struct kevent& event);

		std::string setResponseStatus(struct kevent& event);
		void setResponseHeader(std::string val);
		void setResponseBody(std::string file);
		void setResponsePath(std::string file);

		std::string setImage(std::string imagePath);
};

#endif
