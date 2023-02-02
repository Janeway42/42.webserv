#include "requestStorage.hpp"

requestStorage::requestStorage()
{
	done = false;
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