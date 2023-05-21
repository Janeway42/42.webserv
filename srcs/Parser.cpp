#include <sys/stat.h>
#include <iomanip>
#include <dirent.h>

#include "Parser.hpp"

std::string Parser::keyParser(std::string & lineContent, std::string const & keyToFind) {
    if (not lineContent.empty() || not keyToFind.empty()) {
        if (lineContent.find(keyToFind) != std::string::npos) {
//            std::cout << GRY << lineContent << RES << std::endl;
            return getOneCleanValueFromKey(lineContent, keyToFind);
        }
    }
    return std::string();
}

std::string Parser::getOneCleanValueFromKey(std::string & contentLine, std::string const & key) {
    std::string content = contentLine.substr(contentLine.find(key) + key.size());

    if (not content.empty()) {
        size_t i = 0;
        for (; content.at(i) == ' ' || content.at(i) == '\t'; i++) {}
        std::string trimmed_content = content.substr(i);
        if (trimmed_content.find(';') != std::string::npos) {
            return trimmed_content.substr(0, trimmed_content.size() - 1);
        } else if (trimmed_content.find('{') != std::string::npos) {
            return trimmed_content.substr(0, trimmed_content.size() - 2);
        }
        return trimmed_content.substr(0, trimmed_content.size() - 0);
    }
    return std::string();
}

/*
 * #define S_IRUSR 0000400  read permission, owner
 * #define S_IWUSR 0000200  write permission, owner
 * #define S_IXUSR 0000100  execute/search permission, owner
 */
/* define if path is file(1), folder(2) or something else(3) */
PathType Parser::pathType(std::string const & path) {
    struct stat	buffer = {};

    DIR* dir = opendir(path.c_str());
    if (stat(path.c_str(), &buffer) == 0) {
        if (buffer.st_mode & S_IFREG) {
            if (buffer.st_mode & S_IRUSR) {
                return (REG_FILE);
            } else if (not (buffer.st_mode & S_IRUSR)) {
                std::cout << RED "Error: This file has no READ permission\n" RES;
                return (NO_PERMISSION_FILE);
            }
        } else if (buffer.st_mode & S_IFDIR || dir != NULL) {
            /* The & operator computes the logical AND of its operands. The result of x & y is true if
             * both x and y evaluate to true. Otherwise, the result is false */
            if (buffer.st_mode & S_IRUSR) {
                (void)closedir(dir);
                return (DIRECTORY);
            } else if (not (buffer.st_mode & S_IRUSR)) {
                std::cout << RED "Error: This folder has no READ permission\n" RES;
                return (NO_PERMISSION_DIR);
            }
        } else
            return (OTHER_PATH_TYPE);
    }
    return (PATH_TYPE_ERROR);
}

std::string Parser::addCurrentDirPath(std::string const & fileOrDir) const {
    if (fileOrDir.at(0) != '.') {
        return "./";
    }
    return std::string();
}

std::string Parser::addRootDirectoryPath(std::string const & rootDirectory, std::string const & possiblePath) {
    std::string is_root_dir_or_has_path = possiblePath;
    if (rootDirectory == "./" + possiblePath) {
        return is_root_dir_or_has_path;
    }
    // If possiblePath does not start with '/' or '.', then it is not a path. It's either a file or directory name
    if (possiblePath[0] != '/' && possiblePath[0] != '.') {
        // Adding the rootDirectory path to the file or directory name
        std::string::size_type lastIndex = rootDirectory.size() - 1;
        if (rootDirectory[lastIndex] == '/') {
            is_root_dir_or_has_path = rootDirectory + possiblePath;
        } else {
            is_root_dir_or_has_path = rootDirectory + "/" + possiblePath;
        }
    }
    return is_root_dir_or_has_path;
}

bool Parser::isSpace(char ch) {
    return std::isspace(static_cast<unsigned char>(ch));
}
