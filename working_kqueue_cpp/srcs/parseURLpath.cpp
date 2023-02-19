#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>

// #include "_colors.h"
#include "../includes/RequestParser.hpp"


namespace data {




void printPathParts(std::string str, std::string strTrim, std::string path,
					std::string fileName, RequestData reqData)
{
	std::cout << "Found path:  [" << BLU << str << RES "]\n";
	std::cout << "After trim:  [" << BLU << strTrim << RES "]\n";
	std::cout << "Path part:   [" << MAG << path << RES "]\n";
	std::cout << "File/Folder: [" << MAG << fileName << RES "]\n";

	if (!formData.empty()) {
		std::cout << "\nSTORED KEY:VALUE PAIRS:\n";// Print the map
		std::map<std::string, std::string>::iterator it;
		for (it = formData.begin(); it != formData.end(); it++)
			std::cout << MAG "   " << it->first << RES " ---> " MAG << it->second << "\n" RES;
	}
	else	
		std::cout << "Query:        " << GRE "(not present)\n" RES;
	std::cout << "\n";
}



int checkIfFileExists (const std::string& path) {
    std::ifstream file(path.c_str());

	if (!(file.is_open())) {
		std::cout << RED "File " << path << " not found\n" RES;
		return (-1);
	}
	std::cout << GRN "File " << path << " exists\n" RES;
    return 0;
}



int checkTypeOfFile(const std::string & path) {
	
	std::size_t found = path.find_last_of(".");
	std::string extention = path.substr(found, std::string::npos);
	
	std::cout << GRN "Found Extension: [" << path << "]\n" RES;
	
	return (0);
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

	if (pathNew[0] == '/')	{
		std::string prefix = ".";
		prefix.append(pathNew);
		return prefix;
	}
	return pathNew;
}



/* 	Split string at '&' and store each line into vector<>
	Then split each line in vector into map<> key:value */
std::map<std::string, std::string> Request::storeFormData(std::string &pq) {

	std::string					temp;
	std::vector<std::string>	queryList;

    std::stringstream pathForm(pq);
	while (std::getline(pathForm, temp, '&'))
		queryList.push_back(temp);

	std::string							key, val;
	std::map<std::string, std::string>	formData;
	std::vector<std::string>::iterator	it;

	for (it = queryList.begin(); it != queryList.end(); it++) {
		std::stringstream iss(*it);
	 	std::getline(iss, key, '=') >> val;
		formData[key] = val;
	}
	return (formData);
}





void	storePath_and_FolderName(std::string path, std::string pathFirstPart, std::string pathLastWord) {

		int  pos1	= 0;
		int  pos2	= 0;
		int  count	= 0;
		pos2 		= path.find_first_of("/");

		while (count < path.length()) {
			if ((count = path.find("/", count)) != std::string::npos) {
				pos1 = pos2;
				pos2 = count;
			}
			if ( count == std::string::npos )
				break ;
			count++;
		}
		pathFirstPart	= path.substr(0, pos1 + 1);
		pathLastWord	= path.substr(pos1 + 1, pos2);
}



void	Request::storePathParts_and_FormData(std::string path, std::string pathFirstPart, std::string pathLastWord) {

	int ret					= 0;
	int temp				= path.find_first_of("?");
	std::string tempStr		= path.substr(0, temp);
	int pos 				= tempStr.find_last_of("/");
	pathFirstPart			= tempStr.substr(0, pos);
	pathLastWord			= path.substr(pos, ret - pos);	
	std::string	pathForm	= path.substr(temp, std::string::npos);
	std::map<std::string, std::string>	formData;

	if (pathForm[0] == '?') 	// Remove the '?' from string
		pathForm = &pathForm[1];

	formData = storeFormData(pathForm);
}


int Request::parsePath(std::string str) {
	// maybe also trim white spaces front and back
	Request								req;
	std::string 						path			= removeDuplicateSlash(str);
//	int									len				= path.length();
	int									ret				= 0;
	std::string 						pathFirstPart	= "";
	std::string 						pathLastWord	= "";
	// std::string							pathForm		= "";
	// std::map<std::string, std::string>	formData;
	
	if (path == "")
		return (-1);
	else if (path == "/") {
		std::cout << GRN "The path has no GET-Form data. Path is the root '/'\n" RES;
	}
	else if (path.back() == '/'  && (path.find("?") == std::string::npos)) {
		std::cout << GRN "The path has no GET-Form data. Last char is '/', it must be a folder.\n" RES;
		storePath_and_FolderName(path, pathFirstPart, pathLastWord);

		// int  pos1	= 0;
		// int  pos2	= 0;
		// int  count	= 0;
		// pos2 		= path.find_first_of("/");

		// while (count < len) {
		// 	if ((count = path.find("/", count)) != std::string::npos) {
		// 		pos1 = pos2;
		// 		pos2 = count;
		// 	}
		// 	if ( count == std::string::npos )
		// 		break ;
		// 	count++;
		// }
		// pathFirstPart	= path.substr(0, pos1 + 1);
		// pathLastWord	= path.substr(pos1 + 1, pos2);
		printPathParts(str, path, pathFirstPart, pathLastWord, getRequestData());
	}
	// if the last char is not slash /   then look for question mark 
	else if ((ret = path.find("?")) == std::string::npos ) {
		std::cout << GRN "There is no Form data, the '?' not found\n" RES;
		int pos			= 0;
		pos				= path.find_last_of("/");	
		pathFirstPart	= path.substr(0, pos);
		pathLastWord	= path.substr(pos, std::string::npos);
		printPathParts(str, path, pathFirstPart, pathLastWord, getRequestData());
	}
	
	else if ((ret = path.find("?")) != std::string::npos) {			// Found '?' in the path
		std::cout << GRN "There is Form data, the '?' is found\n" RES;
		storePathParts_and_FormData(path, pathFirstPart, pathLastWord);



		// int temp			= path.find_first_of("?");
		// std::string tempStr	= path.substr(0, temp);
		// int pos 			= tempStr.find_last_of("/");
		// pathFirstPart		= tempStr.substr(0, pos);
		// pathLastWord		= path.substr(pos, ret - pos);	
		// pathForm			= path.substr(temp, std::string::npos);

		// if (pathForm[0] == '?') 	// Remove the '?' from string
		// 	pathForm = &pathForm[1];

		// formData = storeFormData(pathForm);
		printPathParts(str, path, pathFirstPart, pathLastWord, getRequestData());
	}

	checkIfFileExists(path);	// What in case of root only "/"  ???
	checkTypeOfFile(path);
	return (0);
}
} // namespace data






int main()
// int main()
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
	
	

	std::cout << data::checkIfFileExists("test.html") << "\n";
	std::cout << data::checkIfFileExists("_testFolder") << "\n";
	
	
	return (0);
}



