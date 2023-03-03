#ifndef RESPONSEDATA_HPP
#define RESPONSEDATA_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Parser.hpp"

class CgiData: public Parser
{
	private:
		int _fd_in[2];
		int _fd_out[2];
		int _sendPosition;


	public:
		ResponseData();
		virtual ~ResponseData();

		std::string streamFile(std::string file);

		// getters
		std::string getHeader();
		std::string getBody();
		std::string getFullResponse();
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
