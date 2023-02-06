#include "requestStorage.hpp"

requestStorage::requestStorage()
{
	done = false;
	errorReq = false;
}

requestStorage::~requestStorage()
{

}

bool requestStorage::getDone()
{
	return (done);
}

void requestStorage::setDone(bool val)
{
	done = val;
}

bool requestStorage::getError()
{
	return (errorReq);
}

void requestStorage::setError(bool val)
{
	errorReq = val;
}

void requestStorage::appendBuffer(char *temp)
{
	buffer.append(temp);
}