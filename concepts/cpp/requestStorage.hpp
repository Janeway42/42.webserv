#ifndef REQUESTSTORAGE_HPP
#define REQUESTSTORAGE_HPP

#include <string>

class requestStorage
{
	private:
		bool done;
		bool errorReq;
		std::string buffer;

	public:
		requestStorage();
		~requestStorage();

		void appendBuffer(char *temp);

		bool getDone();
		void setDone(bool val);
		bool getError();
		void setError(bool val);

};

#endif