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
		std::string		_status;
		std::string		_type;
		
		std::string 	_responseHeader;
		std::string 	_responseBody;
		std::string		_fullResponse;
		std::string		_responsePath;

		ssize_t 		_lengthFullResponse;	// needs to be set at the end of making the response (just once)
		unsigned long 	_bytesToClient;
		bool			_responseDone;

	public:
		ResponseData();
		virtual ~ResponseData();

		// getters
		std::string		getHeader();
		std::string		getBody();
		std::string		getResponsePath();
		std::string		getResponseBody();
		std::string&	getFullResponse();
		unsigned long	getBytesToClient();
		size_t			getSentSoFar();
		bool			getResponseDone();

		//setters
		void			setResponse(struct kevent& event);
		std::string		setResponseStatus(struct kevent& event);
		void			setResponsePath(std::string path);
		void			setResponseBody(std::string file);
		void			setResponseDone(bool val);
		void			setBytesToClient(int val);

		std::string		streamFile(std::string file);
		std::string 	setImage(std::string imagePath);
		void			increaseSentSoFar(size_t bytesSent);
		std::string&	eraseSentChunkFromFullResponse(unsigned long retBytes); // to erase the sent chunk from the remaining response content


		// NOT USED - to be cleaned out
		// ***************************************************************************
		// added jaka
		//size_t		getCurrentLength(); // jaka
		//void 			setCurrentLength(size_t len);
		// ***************************************************************************
		// -------------------------------------------------------------
		// bool 			_errorOverride;
		// bool			getOverride();
		// void		setOverride(bool val);
		// void		overrideFullResponse();			// jaka: maybe will not be used
		// void		adjustFullResponse(ssize_t ret);
};

#endif
