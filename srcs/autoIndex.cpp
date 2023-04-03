#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string.h>

#include "Parser.hpp"

std::string appendHTMLhead(std::string path, std::string & htmlStr) {

	htmlStr.append("<html>\n<head>\n<title><title>Index of ");
	htmlStr.append(path);
	htmlStr.append("</title>\n</head>\n");
	htmlStr.append("<body><h1>Index of ");
	htmlStr.append(path);
	htmlStr.append("</h1>\n<ul>\n");
	return (htmlStr);
}


std::string removeLastFolderFromPath(std::string path) {
	std::string parentPath;

	size_t len = path.length() - 1;
	for ( ; path[len] == '/' ; len--);
	for (; path[len] != '/' ; len--);

	parentPath = path.substr(0, len);

	std::cout << GRN << "parentPath: [" << parentPath << "]\n" << RES;
	return (parentPath);
}



std::string appendHTMLbody(std::string line, std::string path, std::string & htmlStr) {
	int	found;


	found = line.find_last_of(" ");
	std::string lastName = line.substr(found + 1, std::string::npos);
	
	htmlStr.append("<li><a href='");
	if (lastName != "..")
		htmlStr.append(path);
	else
		htmlStr.append(removeLastFolderFromPath(path)); // remove last folder from the path

	htmlStr.append("/");
	if (lastName != "." && lastName != "..")
		htmlStr.append(lastName);

	htmlStr.append("'>  ");
	if (lastName == "..")
		htmlStr.append("Parent Directory");
	else
		htmlStr.append(lastName);
	htmlStr.append("  </a></li>\n");
	return (htmlStr);
}


void	makeHtmlString(std::string folderContent, std::string path) {

	// int					found;
	std::string			line;
	std::istringstream	iss(folderContent);
	std::string			htmlStr;
	// std::string			lastWord;

	appendHTMLhead(path, htmlStr);

	while (std::getline(iss, line)) {
		if (line[0] == 'd') {
			// std::cout << "Is DIR  [" << line << "]\n";
			appendHTMLbody(line, path, htmlStr);
		}
		else if (line[0] == '-') {
			// std::cout << "Not DIR [" << line << "]\n";
			htmlStr = appendHTMLbody(line, path, htmlStr);
		}
	}
	htmlStr.append("</ul></body></html>");
	std::cout << "\nFolder content as HTML:\n" << BLU << htmlStr << RES "\n";
}


// Check if maybe fds are leaking
std::string storeFolderContent(char *path) {
	int    		fd[2];
	pid_t		retFork;
	std::string	incomingStr;

	if (pipe(fd) == -1)
		std::cout << "Error: Pipe failed\n";
	retFork = fork();

	if (retFork == 0) {
		//std::cout << "Start CHILD\n";
		if (retFork < 0)
			std::cout << "Error: Fork failed\n";
		dup2(fd[1], 1);
		close(fd[0]);

		char *arr[4] = {(char*)"/bin/ls", (char*)"-la", path, NULL};
		int ret = execve(arr[0], arr, NULL);
		std::cout << "Error: Execve failed: " << ret << "\n";
	}
	else {
		close(fd[1]);
		char buff[100];
		dup2(fd[0], 0);

		for (int ret = 1; ret != 0; ) {
			memset(buff, '\0', 100);
			ret = read(fd[0], buff, 99);
			//std::cout << "Returned buffer, ret " << ret << "\n";
			incomingStr.append(buff);
		}
		//std::cout << "\nIncoming [" << incomingStr << "]\n";
	}    
	return (incomingStr);
}

//int main()
//{
//	std::string path = "../resources/_folderA/folderB/";
//	std::string folderContentStr = storeFolderContent((char*)path.c_str());
//	makeHtmlString(folderContentStr, path);
//
//	return (0);
//}
