#include "includes/RequesParser.hpp"

#include <fstream>
#include <sstream>
#include <vector>

namespace data {

/** Default constructor */
Request::Request() {
	_data = HttpRequest();
}

/** Destructor */
Request::~Request() {
    return;
}

/** ########################################################################## */

/** Getters */
HttpRequest const & Request::getRequestData() const {
   return _data;
}

/** ########################################################################## */

bool Request::handleFile(std::string const & requestFileName) {
	/** Opening the file */
	// std::ifstream destructor will close the file automatically, which is one of the perks of using this class.
	std::ifstream requestFile;
    // The IDE may compile the program on a child directory and so the file to open would be one path behind
    requestFile.open(requestFileName);

	if (requestFile.is_open()) {
	   Request::parseFile(requestFile);
	} else {
		std::cerr << "Error: Not able to open the Request file" << std::endl;
		return false;
	}
	requestFile.close();
	return true;
}

void Request::parseFile(std::ifstream & requestFile) {

	std::string lineContent;
	int i = 0;

	/** Reading from the file (per word) */
	while (requestFile) {
		std::getline(requestFile, lineContent);
		//std::cout << i << " lineContent: [" << lineContent << "]" << std::endl;

		// FIRST LINE HEADER
		if (i == 0) {
			storeWordsFromFirstLine(lineContent);
		}
		else {
		// OTHER HEADERS
			std::string reqHost = keyParser(lineContent, "Host:");
			if (!reqHost.empty()) {
				std::cout << "ret from keyParser version [" << reqHost << "]" << std::endl;
				_data.setRequestHost(reqHost);
				continue;
			}
			std::string reqAccept = keyParser(lineContent, "Accept:");
			if (!reqAccept.empty()) {
				std::cout << "ret from keyParser accept [" << reqAccept << "]" << BACK << std::endl;
				_data.setRequestAccept(reqAccept);
				continue;
			}
		}
		i++;
	}
}

std::string Request::keyParser(std::string & lineContent, std::string const & keyToFind) {
	if (lineContent.find(keyToFind) != std::string::npos) {
		std::string ret = getOneCleanValueFromKey(lineContent, keyToFind);  // jaka
		std::cout << "ret from getOneCleanValue [" << ret << "]" << std::endl;
		return ret;
	}
	return std::string();
}

int Request::storeWordsFromFirstLine(std::string const & firstLine) {
    std::vector<std::string> arr;
    std::istringstream iss(firstLine);
    std::string word;

    while (iss >> word)
        arr.push_back(word);

    int vecSize = arr.size();
    if (vecSize > 3) { // Jaka: maybe not needed
        std::cout << "Error: There are more then 3 items in the first line header\n";
        return (1);
    }

    std::vector<std::string>::iterator iter = arr.begin();
    for (int i = 0; iter < arr.end(); iter++, i++) {
        std::cout << BLU << *iter << RES << "\n";
        if (i == 0) {
            if (*iter == "GET" || *iter == "POST" || *iter == "DELETE")
                _data.setRequestMethod(*iter);
            else
                std::cout << RED << "Error: This method is not recognized\n" << RES;
        }
        else if (i == 1)
            _data.setRequestPath(*iter);
        else if (i == 2) {
            if (*iter != "HTTP/1.1")
                std::cout << RED << "Error: wrong http version\n" << RES;
            _data.setHttpVersion(*iter);
        }
    }
    return (0);
}
} // data





