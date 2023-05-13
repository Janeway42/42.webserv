#ifndef CGIDATA_HPP
#define CGIDATA_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Parser.hpp"

class CgiData: public Parser
{
	private:
		int				_fd_in[2];  // we write to cgi  - we write on _fd_in[1]
		int				_fd_out[2]; // we read from cgi - we read from _fd_out[0]
		bool			_pipesDone;
		bool			_isCgi;
		ssize_t   		_bytesToCgi;

	public:
		CgiData();
		virtual ~CgiData();

		int 			createPipes(int kq, struct kevent & event);
		void 			closePipes();

		// getters
		ssize_t			getBytesToCgi();
		bool			getIsCgi();
		bool			getPipesDone();
		int				getPipeCgiIn_0();
		int				getPipeCgiIn_1();
		int				getPipeCgiOut_0();
		int				getPipeCgiOut_1();

		// setters
		void			setIsCgi(bool val);
		void			setPipesDone(bool val);
		void			setBytesToCgi(ssize_t val);
	
		class CgiException: public std::exception
        {
            private:
                std::string _errorMessage;

            public:
                CgiException(std::string message) throw()
                {
                    _errorMessage = message;
                }
                virtual const char* what() const throw()
                {
                    return (_errorMessage.c_str());
                }
                virtual ~CgiException() throw(){}
        };
};

#endif
