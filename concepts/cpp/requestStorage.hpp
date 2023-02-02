#ifndef REQUESTSTORAGE_HPP
#define REQUESTSTORAGE_HPP

#include <string>

class requestStorage
{
	private:
		bool done;
		bool errorRequest;
		std::string buffer;

	public:
		requestStorage();
		~requestStorage();

	bool getDone();
	void setDone(bool val);

};

#endif