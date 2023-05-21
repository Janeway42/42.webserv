#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include "../includes/Parser.hpp"
#include "../includes/ResponseData.hpp"

std::string appendHTMLhead(std::string path, std::string & htmlStr) {
	htmlStr.append("<html>\n<head>\n<title>Index");
	htmlStr.append(path);
	htmlStr.append("</title>\n</head>\n");
	htmlStr.append("<body><h2>Index of the folder: <span style='color:blue;'>");
	htmlStr.append(path);
	htmlStr.append("</h2><ul></span>\n");
	return (htmlStr);
}

std::string removeLastFolderFromPath(std::string path) {
	std::string parentPath;

	size_t len = path.length() - 1;
	for ( ; path[len] == '/' ; len--);
	for (; path[len] != '/' ; len--);

	parentPath = path.substr(0, len + 1);
	std::cout << GRN << "parentPath: [" << parentPath << "]\n" << RES;
	return (parentPath);
}

std::string removeRootFolderNameFromPath(std::string path) {
	size_t positionSecondSlash = path.find("/", 2);
	//std::cout << RED << "positionSecondSlash: [" << positionSecondSlash << "]\n" << RES;
	std::string pathWithoutRoot = path.substr(positionSecondSlash);
	return pathWithoutRoot;
}

std::string appendHTMLbody(std::string line, std::string path, std::string & htmlStr) {
//    std::cout << BLU << "Line: [" << line << "]\n" << RES;
//    std::cout << YEL << "Path: [" << path << "]\n" << RES;
	int found = line.find_first_of(" ");
	std::string lastWord = line.substr(found + 1, std::string::npos);

	htmlStr.append("<li><a href='");
	if (lastWord != "../" && lastWord != "./") {
		htmlStr.append(removeRootFolderNameFromPath(path));
		htmlStr.append(lastWord);
	}
	else if (lastWord == "../") {
		std::string temp = removeLastFolderFromPath(path);
		size_t positionSecondSlash = path.find("/", 2);
		std::string rootfolder = path.substr(0, positionSecondSlash);
		if (temp == rootfolder) {
//			std::cout << RED "NEW PATH IS NOW THE SAME AS ROOT FOLDER: [" << temp << "], rootfolder: [" << rootfolder<< "]\n" RES;
			temp = "../";
		}
        temp = removeRootFolderNameFromPath(temp);
//		std::cout << RED "PARENT FOLDER: [" << temp << "\n" RES;
		htmlStr.append(temp);
	}
	htmlStr.append("'>  ");		// end < href >
	if (lastWord == "../")
		htmlStr.append(". ./<br>");
	else
		htmlStr.append(lastWord);
	htmlStr.append("  </a></li>\n");
	return (htmlStr);
}

std::string	makeHtmlString(std::string folderContent, std::string path) {

	std::string			line;
	std::istringstream	iss(folderContent);
	std::string			htmlStr;

    appendHTMLhead(path, htmlStr);
    while (std::getline(iss, line)) {
		if (iss.fail())
			return (std::string());
        if (line[0] == 'D') {
			appendHTMLbody(line + "/", path, htmlStr);
		}
		else if (line[0] == 'F') {
			htmlStr = appendHTMLbody(line, path, htmlStr);
		}
	}
	htmlStr.append("</ul></body></html>");
	//std::cout << "\nFolder content as HTML:\n" << BLU << htmlStr << RES "\n";
	return (htmlStr);
}

std::string ResponseData::storeFolderContent(const char* path) {
	DIR *dir;
	struct dirent *entry;
	struct stat file_stat;
	char full_path[512];

	dir = opendir(path);
	if (dir == NULL) {
		printf("Failed to open directory: %s\n", path);
        return std::string();// return empty string so we can continue with the 404 error set (also not exit the server)
		//exit(EXIT_FAILURE);
	}

	std::string folders;
	std::string files;
	std::string allFolderContent;

	while ((entry = readdir(dir)) != NULL) {                                    //      . /    filename
		snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);   // joins path + d_name and stores it into full_path
		if (stat(full_path, &file_stat) == -1) {
			printf("Failed to get file information for %s\n", full_path);
			continue;
		}
		if (S_ISREG(file_stat.st_mode)) {
			files.append("F ").append(entry->d_name).append("\n");
		} else if (S_ISDIR(file_stat.st_mode)) {
			folders.append("D ").append(entry->d_name).append("\n");
		}
	}
	closedir(dir);
	allFolderContent.append(folders).append(files);
	//std::cout << BLU "ALL CONTENT:\n" << allFolderContent << "\n" RES;
	std::string htmlStr = makeHtmlString(allFolderContent, path);
	return (htmlStr);
}


// OLD OLD
// // Check if maybe fds are leaking
// std::string ResponseData::storeFolderContentOLD(const char *path) {
// 	std::cout << RED "start StoreFolderContent(), PATH: [" << path << "]\n" RES;
// 	int    		fd[2];
// 	pid_t		retFork;
// 	std::string	incomingStr;

// 	if (pipe(fd) == -1)
// 		std::cout << "Error: Pipe failed\n";
// 	retFork = fork();

// 	if (retFork == 0) {
// 		//std::cout << "Start CHILD LS\n";
// 		if (retFork < 0)
// 			std::cout << "Error: Fork failed\n";
// 		dup2(fd[1], 1);
// 		close(fd[0]);

// 		char *arr[4] = {(char*)"/bin/ls", (char*)"-la", (char*)path, NULL};
// 		int ret = execve(arr[0], arr, NULL);
// 		std::cout << RED "Error: Execve failed: " << ret << "\n" RES;
// 	}
// 	else {
// 		close(fd[1]);
// 		char buff[100];
// 		dup2(fd[0], 0);

// 		for (int ret = 1; ret != 0; ) {
// 			memset(buff, '\0', 100);
// 			ret = read(fd[0], buff, 99);
// 			incomingStr.append(buff);
// 			// std::cout << YEL "Returned buffer, ret " << ret << "\n" RES;
// 			//std::cout << CYN "[" << buff << "]\n" RES;
// 			//std::cout << BLU "[" << incomingStr << "]\n" RES;
// 		}
// 	}    
// 	// std::cout << BLU "[" << incomingStr << "]\n" RES;
// 	std::string htmlStr = makeHtmlString(incomingStr, path);
// 	//std::cout << "\nHTML CONTENT OF THE FOLDER: [\n" << htmlStr << "\n]\n";
// 	return (htmlStr);
// }

