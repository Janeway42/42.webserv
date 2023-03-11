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

		unsigned long _bytesToClient;
		bool _errorOverride;

	public:
		ResponseData();
		virtual ~ResponseData();

		// getters
		std::string		getHeader();
		std::string		getBody();
		std::string&	getFullResponse();
		std::string		getResponsePath();
		std::string		getResponseBody();
		unsigned long			getBytesToClient();
		bool			getOverride();

		//setters
		void		overrideFullResponse();
		void		setResponse(struct kevent& event);
		std::string	setResponseStatus(struct kevent& event);
		void		setResponsePath(std::string path);
		void		setOverride(bool val);
		void		setResponseBody(std::string file);

		std::string	streamFile(std::string file);
		std::string setImage(std::string imagePath);

		void setBytesToClient(int val);
};

#endif
