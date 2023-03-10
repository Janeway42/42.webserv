#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>

/*
	What happens if you dont have a form on your page, but you directly write ?city=aaa in the URL?
	In this case, no action file is specified ???


*/

// #include <sys/types.h>
#include <sys/wait.h>	// for wait() on Linux

#include "../includes/Parser.hpp" // for colors
#include "../includes/RequestParser.hpp"

int checkIfFileExists(const std::string& path) {
	std::ifstream file(path.c_str());

	if (not file.is_open()) {
		std::cout << RED << "Error: File " << path << " not found\n" << RES;
		return (-1);
	}
	std::cout << GRN << "File/folder " << path << " exists\n" << RES;
	return 0;
}

// Not in use
// There is a read buffer overflow
std::string removeDuplicateSlash(std::string pathOld) {
  
	char *temp = (char *)malloc(pathOld.length() * sizeof(char) + 1);
	if (temp == NULL)
	{ std::cout << "Error: removeDuplicate failed mallocing\n";  exit(-1); }  

	int beginOfQuery = false;
	size_t j = 0, i = 0;
	while (i < pathOld.length()) {
		if (pathOld[i] == '/' && pathOld[i - 1] == '/' && i != 0 && beginOfQuery == false) {
			i++;
			continue ;
		}
		if (pathOld[i] == '?')
			beginOfQuery = true;
		temp[j++] = pathOld[i++];
	}
	temp[j] = '\0';
	std::string pathNew(temp);
	free(temp);

	if (pathNew[0] == '/')	{
		std::string prefix = "."; // Not sure why this was necessary ???
		prefix.append(pathNew);
		return prefix;
	}
	return pathNew;
}
