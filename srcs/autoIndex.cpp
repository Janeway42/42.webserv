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
    if (path != "./" && path != "../") {
        std::string parentPath;

        size_t len = path.length() - 1;
        for (; path[len] == '/'; len--);
        for (; path[len] != '/'; len--);


        parentPath = path.substr(0, len + 1);
        std::cout << GRN << "parentPath: [" << parentPath << "]\n" << RES;
        return parentPath;
    }
    return std::string();
}

std::string removeRootFolderNameFromPath(std::string path) {
    if (path != "./" && path != "../") {
        size_t positionSecondSlash = path.find("/", 2);
//        std::cout << RED << "positionSecondSlash: [" << positionSecondSlash << "]\n" << RES;
        std::string pathWithoutRoot = path.substr(positionSecondSlash);
        return pathWithoutRoot;
    }
    return std::string();
}

std::string appendHTMLbody(std::string line, std::string path, std::string & htmlStr) {
    // std::cout << BLU << "Line: [" << line << "]\n" << RES;
    // std::cout << BLU << "Path: [" << path << "]\n" << RES;
	int found = line.find_first_of(" ");
	std::string lastWord = line.substr(found + 1, std::string::npos);

	htmlStr.append("<li><a href='");
	if (lastWord != "../" && lastWord != "./") {
		htmlStr.append(removeRootFolderNameFromPath(path));
		htmlStr.append(lastWord);
	}
	else if (lastWord == "../" && path.size() > 2) {
		std::string temp = removeLastFolderFromPath(path);
		size_t positionSecondSlash = path.find("/", 2);
		std::string rootfolder = path.substr(0, positionSecondSlash);
		if (temp == rootfolder) {
			// std::cout << RED "NEW PATH IS NOW THE SAME AS ROOT FOLDER: [" << temp << "], rootfolder: [" << rootfolder<< "]\n" RES;
			temp = "../";
		}
        temp = removeRootFolderNameFromPath(temp);
		// std::cout << RED "PARENT FOLDER: [" << temp << "\n" RES;
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
