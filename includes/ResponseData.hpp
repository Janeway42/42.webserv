#ifndef RESPONSEDATA_HPP
#define RESPONSEDATA_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Parser.hpp"

#define NOCOOKIE "/cookies/noCookies.html"
#define YESCOOKIE "/cookies/yesCookies.html"

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
        ssize_t 	    _bytesToClient;
		bool			_responseDone;

	public:
		ResponseData();
		virtual ~ResponseData();

        // Methods
        void			createResponse(struct kevent& event);
        void    		setResponseStatus(struct kevent& event);
        // void			createResponseHeader(HttpStatus status, std::string const & redirectionUrl);
		void			createResponseHeader(struct kevent& event);
        std::string		streamFile(std::string file);
        std::string 	setImage(std::string imagePath);
        void			increaseSentSoFar(ssize_t bytesSent);
        std::string&	eraseSentChunkFromFullResponse(unsigned long retBytes); // to erase the sent chunk from the remaining response content
        std::string		storeFolderContent(const char *path);

		// Getters
		std::string		getHeader();
		std::string		getBody();
		std::string		getResponsePath();
		std::string		getResponseBody();
		std::string&	getFullResponse();
		unsigned long	getBytesToClient();
		size_t			getSentSoFar();
		bool			getResponseDone();

        // Setters
		void			setResponsePath(std::string path);
		void			setResponseBody(std::string file);
		void			setResponseDone(bool val);
		void			setBytesToClient(ssize_t val);
		void			setResponseFull(std::string response);

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
