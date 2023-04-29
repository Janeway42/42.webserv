#include "CgiData.hpp"
#include "RequestParser.hpp"

CgiData::CgiData()
{
	_fd_in[0] = -1;
	_fd_in[1] = -1;
	_fd_out[0] = -1;
	_fd_out[1] = -1;
	_bytesToCgi = 0;
	_isCgi = false;
	_pipesDone = false;
}

CgiData::~CgiData(){}

// --------------------------------------------------------- _cgi functions
// -----------------------------------------------------------------------

void CgiData::createPipes(int kq, struct kevent & event)
{
	pipe(_fd_in);
	pipe(_fd_out);
	fcntl(_fd_in[1], F_SETFL, O_NONBLOCK);
	fcntl(_fd_out[0], F_SETFL, O_NONBLOCK);

	std::cout << YEL "Created pipes:  in[0]" << _fd_in[0] << ",  in[1]" << _fd_in[1] 
							   << ", out[0]" << _fd_out[0] << ", out[1]" << _fd_out[1] << " \n" RES; 

	Request *storage = (Request *)event.udata;
	struct kevent evSet;
	EV_SET(&evSet, _fd_in[1], EVFILT_WRITE, EV_ADD, 0, 0, storage); 
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw CgiException("failed kevent EV_ADD, EVFILT_WRITE _fd_in[1] in createPipes");
	EV_SET(&evSet, _fd_out[0], EVFILT_READ, EV_ADD, 0, 0, storage); 
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw CgiException("failed kevent EV_ADD, EVFILT_READ _fd_out[0] in createPipes");
	_pipesDone = true;

	// upon fork close in the parent _fd_in[0] && _fd_out[1]
}

void CgiData::closePipes()
{
	if (fcntl(_fd_in[0], F_GETFD) != -1)
		close(_fd_in[0]);
	if (fcntl(_fd_in[1], F_GETFD) != -1)
		close(_fd_in[1]);
	if (fcntl(_fd_out[0], F_GETFD) != -1)
		close(_fd_out[0]);
	if (fcntl(_fd_out[1], F_GETFD) != -1)
		close(_fd_out[1]);
}

// ------------------------------------------------------------------ getters
// --------------------------------------------------------------------------

bool CgiData::getIsCgi()	// added jaka
{
	return (_isCgi);
}

unsigned long CgiData::getBytesToCgi()
{
	return (_bytesToCgi);
}

int CgiData::getPipeCgiIn_0()
{
	return (_fd_in[0]);
}

int CgiData::getPipeCgiIn_1()
{
	return (_fd_in[1]);
}

int CgiData::getPipeCgiOut_0()
{
	return (_fd_out[0]);
}
int CgiData::getPipeCgiOut_1()
{
	return (_fd_out[1]);
}

bool CgiData::getPipesDone()
{
	return (_pipesDone);
}

// ------------------------------------------------------------------ setters
// --------------------------------------------------------------------------

void CgiData::setIsCgi(bool val)
{
	_isCgi = val;
}

void CgiData::setBytesToCgi(int val)
{
	_bytesToCgi += val;
}

void CgiData::setPipesDone(bool val)
{
	_pipesDone = val;
}
