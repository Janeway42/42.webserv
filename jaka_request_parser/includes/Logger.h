//
// Created by Joyce Macksuele on 1/23/23.
//

#ifndef WEBSERV_LOGGER_H
#define WEBSERV_LOGGER_H

#include <cstdarg>

// todo work in progress
class Logger {
#ifdef LOGGER
#define INFO_LOG(format, ?) infoLog(const char *format, ...)
#define ERROR_LOG(format, ?) errorLog(const char *format, ...)
int		infoLog(const char *format, ...) {
	if (format)
	{
		va_start(ap, format);
		...
        // todo print with colot, print line file:line_number, print values with [], finishes format with :
		va_end(ap);
	}
}
//example of log line now: std::cout << REDB << "Found \"server_name\" line [" << lineContent << "]" << BACK << std::endl;
//example of what I want: LOG("Found \"server_name\" line %s", lineContent);
#endif
};


#endif //WEBSERV_LOGGER_H
