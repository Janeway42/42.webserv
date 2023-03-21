#ifndef PARSEURLPATH_HPP
#define PARSEURLPATH_HPP

#include "RequestParser.hpp"

// int checkIfFileExists(const std::string & path); not used
int checkTypeOfFile(const std::string path);
std::string removeDuplicateSlash(std::string pathOld);

#endif //PARSEURLPATH_HPP
