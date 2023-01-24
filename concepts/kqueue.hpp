#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <iostream>
#include <string>
#include <vector>

// using namespace std;

class requestStorage
{
	public:
		std::string buffer;
		int fd;
		int method;

	requestStorage(void);
	~requestStorage(void);
	
	void prepareGet();
	void preparePost();
};

#endif
