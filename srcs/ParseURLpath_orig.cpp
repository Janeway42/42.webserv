#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include "RequestParser.hpp"

// #include "_colors.h"

// Some of arguments not used
void printPathParts(std::string str, std::string strTrim, std::string path,
					std::string fileName, RequestData reqData) {
	(void)path;
	(void)fileName;

	std::cout << "Found path:   [" << BLU << str << RES << "]\n";
	std::cout << "Path trimmed: [" << BLU << strTrim << RES << "]\n";
	std::cout << "Path part:    [" << PUR << reqData.getPathFirstPart() << RES << "]\n";
	std::cout << "File/Folder:  [" << PUR << reqData.getPathLastWord() << RES << "]\n";

	std::map<std::string, std::string> formData;
	formData = reqData.getFormData();

	if (! formData.empty()) {
		std::cout << "\nSTORED FORM DATA PAIRS:\n";// Print the map
		std::map<std::string, std::string>::iterator it;
		for (it = formData.begin(); it != formData.end(); it++)
			std::cout << PUR << "   " << it->first << RES << " ---> " << PUR << it->second << "\n" << RES;
	}
	else	
		std::cout << "Form Data:    " << GRE << "(not present)\n" << RES;
	std::cout << "\n";
}

int checkIfFileExists (const std::string& path) {
    std::ifstream file(path.c_str());

	if (!(file.is_open())) {
		std::cout << RED << "File " << path << " not found\n" << RES;
		return (-1);
	}
	std::cout << GRE << "File " << path << " exists\n" << RES;
    return 0;
}

int checkTypeOfFile(const std::string path) {
	
	std::string temp = path;
	if (path[0] == '.')
		temp = path.substr(1, std::string::npos);

	std::size_t found = temp.find_last_of(".");

	if (found != std::string::npos) {
		std::string extention = temp.substr(found, std::string::npos);
		std::cout << GRE << "Found Extension: [" << extention << "]\n" << RES;
	}
	else
		std::cout << GRE << "There is no extention in the last name\n" << RES;
	return (0);
}

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

/*
localhost:8080/folder//////folder/something.html?city=Tokio&street=Singel
*/

//int mainXXX()
// int main()
//{
	// parsePath("/");
	// parsePath("/home/");						// must be folder
	// parsePath("/home");							// check if folder or file
	
	// parsePath("//////home/////folderA/");                	// must be folder
	// parsePath("/home////folderB/");                	// must be folder
	// parsePath("/home/folderC/////");                 	// check if folder or file
	// parsePath("/home/folderD");                 	// check if folder or file
	
	
	// parsePath("/home/index.html");                 	// check if folder or file
	// parsePath("/home/folderD/index.html?street=///singel///");                 	// check if folder or file

	// parsePath("/home/index.html/");             	// check if folder or file
	// parsePath("/home/folder/index.html");		// check if folder or file
	// parsePath("/home/folder/response.php");		// check if folder or file
	// parsePath("/home/folder/response.php?street=Singel&city=London");

	// parsePath("kostja.se////folder//folder/folder/folder///folder/folder//index.html?city=tokio&street=singel");

//	std::cout << checkIfFileExists("../test.html") << "\n";
//	std::cout << checkIfFileExists("_testFolder") << "\n";
//
//	return (0);
//}
