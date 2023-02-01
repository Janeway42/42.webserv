#include <fstream>
#include "includes/Parser.hpp"
#include "includes/RequestData.hpp"
#include "includes/RequestParser.hpp"

int main() {
    data::Request requestFile;

	if (requestFile.handleFile("incomingRequest"))
	{
		// STORE FIRST LINE HEADER
		std::string reqMethodGET = requestFile.getRequestData().getRequestMethod();
		std::string reqHttpPath = requestFile.getRequestData().getHttpPath();
		std::string reqHttpVersion = requestFile.getRequestData().getHttpVersion();

		std::cout << RED_BG << "Found first line header: [" << reqMethodGET << ", " << reqHttpPath << ", " << reqHttpVersion << "]" << BACK << std::endl;

		// STORE OTHER HEADERS
		std::string reqHost = requestFile.getRequestData().getRequestHost();
		std::cout << RED_BG << "Found request Host: [" << reqHost << "]" << BACK << std::endl;

		std::string reqAccept = requestFile.getRequestData().getRequestAccept();
		std::cout << RED_BG << "Found request Accept: [" << reqAccept << "]" << BACK << std::endl;

		// STORE BODY
	}
    return EXIT_SUCCESS;
}
