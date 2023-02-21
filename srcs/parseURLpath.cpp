#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>

// #include "_colors.h"
#include "../includes/RequestParser.hpp"


namespace data {

// Some of arguments not used
void printPathParts(std::string str, std::string strTrim, std::string path,
					std::string fileName, RequestData reqData) {
	(void)path;
	(void)fileName;

	std::cout << "Found path:   [" << BLU << str << RES "]\n";
	std::cout << "Path trimmed: [" << BLU << strTrim << RES "]\n";
	std::cout << "Path part:    [" << MAG << reqData.getPathFirstPart() << RES "]\n";
	std::cout << "File/Folder:  [" << MAG << reqData.getPathLastWord() << RES "]\n";

	std::map<std::string, std::string> formData;
	formData = reqData.getFormData();

	if (! formData.empty()) {
		std::cout << "\nSTORED FORM DATA PAIRS:\n";// Print the map
		std::map<std::string, std::string>::iterator it;
		for (it = formData.begin(); it != formData.end(); it++)
			std::cout << MAG "   " << it->first << RES " ---> " MAG << it->second << "\n" RES;
	}
	else	
		std::cout << "Form Data:    " << GRE "(not present)\n" RES;
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



int checkTypeOfFile(const std::string path) {
	
	std::string temp = path;
	if (path[0] == '.')
		temp = path.substr(1, std::string::npos);

	std::size_t found = temp.find_last_of(".");

	if (found != std::string::npos) {
		std::string extention = temp.substr(found, std::string::npos);
		std::cout << GRN "Found Extension: [" << extention << "]\n" RES;
	}
	else
		std::cout << GRN "There is no extention in the last name\n" RES;
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



/* 	Split string at '&' and store each line into vector<>
	Then split each line in vector into map<> key:value */
std::map<std::string, std::string> Request::storeFormData(std::string &pathForm)
{
	//std::cout << GRN "Start store form data()\n" RES;
	std::cout << GRN "   BODY:        [" << _body << "]\n" RES;
	std::cout << GRN "   FORM PATH:   [" << pathForm << "]\n" RES;

	std::string					line;
	std::vector<std::string>	formList;

    std::stringstream iss(pathForm);
	while (std::getline(iss, line, '&'))
		formList.push_back(line);

	std::string							key, val;
	std::map<std::string, std::string>	formDataMap;
	std::vector<std::string>::iterator	it;

	for (it = formList.begin(); it != formList.end(); it++) {
		std::stringstream iss(*it);
	 	std::getline(iss, key, '=') >> val;
		formDataMap[key] = val;
	}
	_data.setFormData(formDataMap);
	return (formDataMap);
}



// Last word in path must be a folder (last '/' found)
// The 2nd and 3rd args not needed anymore
// void	Request::storePath_and_FolderName(std::string path, std::string pathFirstPart, std::string pathLastWord, RequestData reqData) {
void	Request::storePath_and_FolderName(std::string path) {

		int 	pos1	= 0;
		int		pos2	= 0;
		size_t 	count	= 0;
		pos2 			= path.find_first_of("/");

		while (count < path.length()) {
			if ((count = path.find("/", count)) != std::string::npos) {
				pos1 = pos2;
				pos2 = count;
			}
			if ( count == std::string::npos )
				break ;
			count++;
		}
	//	pathFirstPart	= path.substr(0, pos1 + 1);
	//	pathLastWord	= path.substr(pos1 + 1, pos2);

		_data.setPathFirstPart(path.substr(0, pos1 + 1));
		_data.setPathLastWord(path.substr(pos1 + 1, pos2));
	//	reqData.setPathLastWord(path.substr(pos1 + 1, pos2));
}


// Found GET Method with '?' Form Data
void	Request::storePathParts_and_FormData(std::string path) {

	int temp				= path.find_first_of("?");
	std::string tempStr		= path.substr(0, temp);
	int posLastSlash 		= tempStr.find_last_of("/");
	int	posFirstQuestMark	= path.find_first_of("?");
	std::string	pathForm	= path.substr(temp, std::string::npos);

	_data.setPathFirstPart(tempStr.substr(0, posLastSlash));
	_data.setPathLastWord(path.substr(posLastSlash, posFirstQuestMark - posLastSlash));

	if (pathForm[0] == '?') 	// Skip the '?' in the path
		pathForm = &pathForm[1];
	storeFormData(pathForm);
}


int Request::parsePath(std::string str) {
	// maybe also trim white spaces front and back
//	Request		req;
	std::string path			= removeDuplicateSlash(str);
	size_t		ret				= 0;
	std::string pathLastWord	= "";
	
	if (path == "")
		return (-1);
	else if (path == "/") {
		std::cout << GRN "The path has no GET-Form data. Path is the root '/'\n" RES;
	}
	else if (path.back() == '/'  && (path.find("?") == std::string::npos)) {
		std::cout << GRN "The path has no GET-Form data. Last char is '/', it must be a folder.\n" RES;
		storePath_and_FolderName(path);
		printPathParts(str, path, "", "", getRequestData());
	}

	// if the last char is not slash /   then look for question mark 
	else if ((ret = path.find("?")) == std::string::npos ) {
		std::cout << GRN "There is no Form data, the '?' not found\n" RES;
		int pos			= 0;
		pos				= path.find_last_of("/");	

		_data.setPathFirstPart(path.substr(0, pos));
		_data.setPathLastWord(path.substr(pos, std::string::npos));
		printPathParts(str, path, "", "", getRequestData());
	}
	
	else if ((ret = path.find("?")) != std::string::npos) {			// Found '?' in the path
		std::cout << GRN "There is GET Form data, the '?' is found\n" RES;
		storePathParts_and_FormData(path);
		printPathParts(str, path, "", "", getRequestData());
	}

	checkIfFileExists(path);	// What in case of root only "/"  ???
	 checkTypeOfFile(path);
	//checkTypeOfFile(_data.getPathLastWord());
	//std::cout << RED "Last word " << _data.getPathLastWord() << RES "\n";
	return (0);
}
} // namespace data


/*
localhost:8080/folder//////folder/something.html?city=Tokio&street=Singel
*/



int mainXXX()
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
	
	

	std::cout << data::checkIfFileExists("../test.html") << "\n";
	std::cout << data::checkIfFileExists("_testFolder") << "\n";
	
	
	return (0);
}



