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
		std::string _type;
		
		std::string _responseHeader;
		std::string _responseBody;
		std::string _fullResponse;
		std::string _responsePath;

		bool		_isCgi;		// added jaka


		ssize_t 		_lengthFullResponse;	// needs to be set at the end of making the response (just once)
		unsigned long 	_bytesToClient;
		bool 			_errorOverride;

	public:
		ResponseData();
		virtual ~ResponseData();

		// getters
		std::string		getHeader();
		std::string		getBody();
		std::string&	getFullResponse();

		// ***************************************************************************
		// added jaka
		//size_t		getCurrentLength(); // jaka
		size_t			getSentSoFar(); // jaka
		std::string&	eraseSentChunkFromFullResponse(unsigned long retBytes); // to erase the sent chunk from the remaining response content
		//void 			setCurrentLength(size_t len);
		void			increaseSentSoFar(size_t bytesSent);
		// ***************************************************************************



		std::string		getResponsePath();
		std::string		getResponseBody();
		unsigned long	getBytesToClient();
		bool			getOverride();
		bool			getIsCgi();		// added jaka

		//setters
		void		overrideFullResponse();			// jaka: maybe will not be used
		void		setResponse(struct kevent& event);
		std::string	setResponseStatus(struct kevent& event);
		void		setResponsePath(std::string path);
		void		setOverride(bool val);
		void		setResponseBody(std::string file);
		void		adjustFullResponse(ssize_t ret);
		std::string	streamFile(std::string file);
		std::string setImage(std::string imagePath);

		void		setIsCgi(bool b);

		void setBytesToClient(int val);
};

#endif
