#include "../includes/CgiData.hpp"
#include "../includes/RequestParser.hpp"

CgiData::CgiData()
{
	_fd_in[0] = -1;
	_fd_in[1] = -1;
	_fd_out[0] = -1;
	_fd_out[1] = -1;
	_bytesToCgi = 0;
	_pipesDone = false;
}

CgiData::~CgiData(){}

// --------------------------------------------------------- cgi functions
// -----------------------------------------------------------------------

bool CgiData::checkFdCgi(struct kevent & event, int filter)
{
	if (filter == EVFILT_READ && event.ident == _fd_out[0])
		return (true);
	else if (filter == EVFILT_WRITE && event.ident == _fd_in[1])
		return (true);
	return (false);
}

// ------------------------------------------------------------------ getters
// --------------------------------------------------------------------------

int CgiData::getBytesToCgi()
{
	return (_bytesToCgi);
}

int CgiData::getPipeCgiIn()
{
	return (_fd_in[1]);
}

int CgiData::getPipeCgiOut()
{
	return (_fd_out[0]);
}

bool CgiData::getPipesDone()
{
	return (_pipesDone);
}

// ------------------------------------------------------------------ setters
// --------------------------------------------------------------------------

void CgiData::setPosition(int val)
{
	_bytesToCgi += val;
}

void CgiData::setPipesDone(bool val)
{
	_pipesDone = val;
}

void CgiData::setBytesToCgi(int bytesSent)
{
	_bytesToCgi += bytesSent;
}


void CgiData::createPipes(int kq, struct kevent & event)
{
	pipe(_fd_in);
	pipe(_fd_out);
	fcntl(_fd_in[1], F_SETFL, O_NONBLOCK);
	fcntl(_fd_out[0], F_SETFL, O_NONBLOCK);

	Request *storage = (Request *)event.udata;
	struct kevent evSet;
	EV_SET(&evSet, _fd_in[1], EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw CgiException("failed kevent add EVFILT_WRITE in _fd_in[1]");
	EV_SET(&evSet, _fd_out[0], EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw CgiException("failed kevent add EVFILT_READ in _fd_out[0]");
	_pipesDone = true;

	// upon fork close in the parent _fd_in[0] && _fd_out[1]
}

// -------------------------------------------------------------------- utils
// --------------------------------------------------------------------------

void CgiData::closeFileDescriptors(struct kevent & event)
{
	// -------- alternative method (if a fd is closed after change the val to -1 )
	if (_pipesDone == true)
	{
		if (_fd_in[0] != -1)
			close(_fd_in[0]);	
		if (_fd_in[1] != -1)
			close(_fd_in[1]);
		if (_fd_out[0] != -1)
			close(_fd_out[0]);
		if (_fd_out[1] != -1)
			close(_fd_out[1]);
	}	
}