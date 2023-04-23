#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include "../includes/Parser.hpp"
#include "../includes/ResponseData.hpp"


std::string appendHTMLhead(std::string path, std::string & htmlStr) {

	htmlStr.append("<html>\n<head>\n<title>Index");
	htmlStr.append(path);
	htmlStr.append("</title>\n</head>\n");
	htmlStr.append("<body><h2>Index of the folder: <span style='color:blue;'>");
	htmlStr.append(path);
	htmlStr.append("</h2><ul></span>");
	return (htmlStr);
}


// std::string removeLastFolderFromPath(std::string path) {
// 	std::string parentPath;

// 	size_t len = path.length() - 1;
// 	for ( ; path[len] == '/' ; len--);
// 	for (; path[len] != '/' ; len--);

// 	parentPath = path.substr(0, len);

// 	//std::cout << GRN << "parentPath: [" << parentPath << "]\n" << RES;
// 	return (parentPath);
// }


std::string removeRootFolderNameFromPath(std::string path) {

	size_t positionSecondSlash = path.find("/", 2);
	//std::cout << RED << "positionSecondSlash: [" << positionSecondSlash << "]\n" << RES;

	std::string pathWithoutRoot = path.substr(positionSecondSlash);
	return pathWithoutRoot;
}


std::string appendHTMLbody(std::string line, std::string path, std::string & htmlStr) {
	int	found;
	//(void)path;
	// std::cout << BLU << "Line: [" << line << "]\n" << RES;
	// std::cout << YEL << "Path: [" << path << "]\n" << RES;
	found = line.find_last_of(" ");
	std::string lastWord = line.substr(found + 1, std::string::npos);
	std::cout << YEL << "lastWord: [" << lastWord << "]\n" << RES;
	
	htmlStr.append("<li><a href='");
	if (lastWord != ".." && lastWord != ".") {
		htmlStr.append(removeRootFolderNameFromPath(path));
		htmlStr.append(lastWord);
	}
	else if (lastWord == "..")
		htmlStr.append("..");
	htmlStr.append("'>  ");		// end < href >
	if (lastWord == "..")
		htmlStr.append(". .<br><br>");
	else
		htmlStr.append(lastWord);
	htmlStr.append("  </a></li>\n");
	return (htmlStr);
}


// void	makeHtmlString(std::string folderContent, std::string path) {
std::string	makeHtmlString(std::string folderContent, std::string path) {

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
	//std::cout << "\nFolder content as HTML:\n" << BLU << htmlStr << RES "\n";
	return (htmlStr);
}


// Check if maybe fds are leaking
std::string ResponseData::storeFolderContent(const char *path) {
	std::cout << RED "start StoreFolderContent(), PATH: [" << path << "]\n" RES;
	int    		fd[2];
	pid_t		retFork;
	std::string	incomingStr;

	if (pipe(fd) == -1)
		std::cout << "Error: Pipe failed\n";
	retFork = fork();

	if (retFork == 0) {
		//std::cout << "Start CHILD LS\n";
		if (retFork < 0)
			std::cout << "Error: Fork failed\n";
		dup2(fd[1], 1);
		close(fd[0]);

		char *arr[4] = {(char*)"/bin/ls", (char*)"-la", (char*)path, NULL};
		int ret = execve(arr[0], arr, NULL);
		std::cout << RED "Error: Execve failed: " << ret << "\n" RES;
	}
	else {
		close(fd[1]);
		char buff[100];
		dup2(fd[0], 0);

		for (int ret = 1; ret != 0; ) {
			memset(buff, '\0', 100);
			ret = read(fd[0], buff, 99);
			//std::cout << YEL "Returned buffer, ret " << ret << "\n" RES;
			incomingStr.append(buff);
		}
	}    
	std::string htmlStr = makeHtmlString(incomingStr, path);
	std::cout << "\nHTML CONTENT OF THE FOLDER: [\n" << htmlStr << "\n]\n";
	return (htmlStr);
}

//int main()
//{
//	std::string path = "../resources/_folderA/folderB/";
//	std::string folderContentStr = storeFolderContent((char*)path.c_str());
//	makeHtmlString(folderContentStr, path);
//
//	return (0);
//}
