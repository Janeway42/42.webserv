#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>

// #include "_colors.h"
#include "../includes/RequestParser.hpp"

void printPathParts(std::string str, std::string strTrim, std::string path,
					std::string fileName, std::map<std::string, std::string> query)
{
	std::cout << "Found path:  [" << BLU << str << RES "]\n";
	std::cout << "After trim:  [" << BLU << strTrim << RES "]\n";
	std::cout << "Path part:   [" << MAG << path << RES "]\n";
	std::cout << "File/Folder: [" << MAG << fileName << RES "]\n";

	if (!query.empty()) {
		std::cout << "\nSTORED KEY:VALUE PAIRS:\n";// Print the map
		std::map<std::string, std::string>::iterator it;
		for (it = query.begin(); it != query.end(); it++)
			std::cout << MAG "   " << it->first << RES " ---> " MAG << it->second << "\n" RES;
	}
	else	
		std::cout << "Query:        " << GRE "(not present)\n" RES;
	std::cout << "\n";
}



std::string removeDuplicateSlash(std::string pathOld) {
  
	char *temp = (char *)malloc(pathOld.length() * sizeof(char) + 1);
	if (temp == NULL)
	{ std::cout << "Error: removeDuplicate failed mallocing\n";  exit(-1); }  

	int beginOfQuery = false;
	int j = 0, i = 0;
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
	return pathNew;
}


std::map<std::string, std::string> storePathQuery(std::string &pq) {

	// Split string at '&' and store into vector<> 
	std::string temp;
	std::vector<std::string> queryList;

    std::stringstream pathQuery(pq);
	while (std::getline(pathQuery, temp, '&'))
		queryList.push_back(temp);

	// Split each line in vector into map<> key:value
	std::string key, val;
	std::map<std::string, std::string> queryMap;

	std::vector<std::string>::iterator it;
	for (it = queryList.begin(); it != queryList.end(); it++) {
		std::stringstream iss(*it);
	 	std::getline(iss, key, '=') >> val;
		queryMap[key] = val;
	}
	return (queryMap);
}



namespace data {

int Request::parsePath(std::string str)
{
	// maybe also trim white spaces front and back
	std::string path			= removeDuplicateSlash(str);
	int ret						= 0;
	int len						= path.length();
	std::string pathFirstPart	= "";
	std::string pathLastName	= "";
	std::string pathQuery		= "";
	std::map<std::string, std::string> queryMap;
	
	if (path == "")
		return (-1);
	else if (path == "/") {
		std::cout << GRN "The path has no GET-Query. Path is the root '/'\n" RES;
	}
	else if (path.back() == '/'  && (path.find("?") == std::string::npos)) {
		std::cout << GRN "The path has no GET-Query. Last char is '/', it must be a folder.\n" RES;

		int  pos1	= 0;
		int  pos2	= 0;
		int  cnt	= 0;
		pos2 		= path.find_first_of("/");

		while( cnt < len ) {
			if ((cnt = path.find("/", cnt)) != std::string::npos) {
				pos1 = pos2;
				pos2 = cnt;
			}
			if ( cnt == std::string::npos )
				break ;
			cnt++;
		}
		pathFirstPart	= path.substr(0, pos1 + 1);
		pathLastName	= path.substr(pos1 + 1, pos2);
		printPathParts(str, path, pathFirstPart, pathLastName, queryMap);
	}
	// if the last char is not slash /   then look for question mark 
	else if ((ret = path.find("?")) == std::string::npos ) {
		int pos = 0;
		std::cout << GRN "There is no Query, the '?' not found\n" RES;
		pos = path.find_last_of("/");	
		pathFirstPart	= path.substr(0, pos);
		pathLastName	= path.substr(pos, std::string::npos);
		printPathParts(str, path, pathFirstPart, pathLastName, queryMap);
	}
	
	else if ((ret = path.find("?")) != std::string::npos) {			// Found '?' in the path
		std::cout << GRN "There is Query, the '?' is found\n" RES;

		int temp			= path.find_first_of("?");
		std::string tempStr	= path.substr(0, temp);
		int pos 			= tempStr.find_last_of("/");
		pathFirstPart		= tempStr.substr(0, pos);
		pathLastName		= path.substr(pos, ret - pos);	
		pathQuery			= path.substr(temp, std::string::npos);

		if (pathQuery[0] == '?') 	// Remove the '?' from string
			pathQuery = &pathQuery[1];
		queryMap = storePathQuery(pathQuery);
		printPathParts(str, path, pathFirstPart, pathLastName, queryMap);
	}
	return (0);
}

} // namespace data

int main_OLD()
{
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
	return (0);
}