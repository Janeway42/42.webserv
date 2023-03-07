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
		int _fd_in[2];  // we write to cg  - we write on _fd_in[1]
		int _fd_out[2]; // we read from cgi - we read from _fd_out[0]
		int _sendPosition;
		bool _pipesDone;
		std::string _tempBuffer;


	public:
		CgiData();
		virtual ~CgiData();

		void createPipes(int kq, struct kevent & event);

		// getters
		int		getPosition();
		int		getPipeCgiIn();
		int		getPipeCgiOut();
		bool	getPipesDone();
		std::string getTempBuffer();  // to be writen 

		// setters
		void setPosition(int bytesSent);
		void setPipesDone(bool val);
		void setTempBuffer(std::string value); // to be written 

		// utils 
		bool checkFdCgi(struct kevent & event, int filter);
		void closeFileDescriptors(struct kevent & event);


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
