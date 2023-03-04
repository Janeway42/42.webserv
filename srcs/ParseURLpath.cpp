#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>

/*
	What happens if you dont have a form on your page, but you directly write ?city=aaa in the URL?
	In this case, no action file is specified ???


*/



// #include <sys/types.h>
#include <sys/wait.h>	// for wait() on Linux

#include "../includes/Parser.hpp" // for colors
#include "../includes/RequestParser.hpp"

/*
char* ENV[25] = {
	(char*)"COMSPEC=", (char*)"DOCUMENT_ROOT=", (char*)"GATEWAY_INTERFACE=", (char*)"HTTP_ACCEPT=", (char*)"HTTP_ACCEPT_ENCODING=",             
	(char*)"HTTP_ACCEPT_LANGUAGE=", (char*)"HTTP_CONNECTION=", (char*)"HTTP_HOST=", (char*)"HTTP_USER_AGENT=", (char*)"PATH=",            
	(char*)"QUERY_STRING=", (char*)"REMOTE_ADDR=", (char*)"REMOTE_PORT=", request_method2, (char*)"REQUEST_URI=", (char*)"SCRIPT_FILENAME=",
	(char*)"SCRIPT_NAME=", (char*)"SERVER_ADDR=", (char*)"SERVER_ADMIN=", (char*)"SERVER_NAME=",(char*)"SERVER_PORT=",(char*)"SERVER_PROTOCOL=",     
	(char*)"SERVER_SIGNATURE=", (char*)"SERVER_SOFTWARE=", NULL
};
*/

std::string runExecve(char *ENV[], char *args[], int fdClient) {
	//std::cout << BLU << "START runExeve\n" << RES;
	//std::cout << "ENV: " << ENV[0] << "\n";

	int    		fd[2];
	
	int    		fdSendBody[2];
	
	pid_t		retFork;
	std::string	incomingStr;

	if (pipe(fd) == -1)
		std::cout << "Error: Pipe failed\n";
	
	//std::cout << "pipe fd[0] " << fd[0] << ", pipe fd[1] " << fd[1] << " \n";
	//dup2(fdClient, fd[1]);
	//std::cout << BLU << "POST BODY ENV : " << ENV[2] << "\n" << RES;
	// BY HERE, THE HUGE TEXTFILE IS STORED OK


	retFork = fork();

	if (retFork == 0) {
		std::cout << "    Start CHILD execve()\n";
		if (retFork < 0)
			std::cout << "Error: Fork failed\n";

		dup2(fd[1], 1);
		(void)fdClient;
		//dup2(fdClient, fd[1]);
		close(fd[0]);

		// BIG BODY NEEDS TO BE SENT TO THE CGI BY WRITE TO PIPE
		close(fdSendBody[0]);	// close stdout reading from
		dup2(fdSendBody[1], 1);
		int ret2 = write(fdSendBody[1], "Something ..." , 13);
		std::cout << YEL << "ret from write to CGI : " << ret2 << "\n" << RES;


		// std::cout << YEL << "POST BODY ENV : " << ENV[2] << "\n" << RES;



		int ret = execve(args[0], args, ENV);
		std::cout << RED << "Error: Execve failed: " << ret << "\n" << RES;
	}
	else {
		wait(NULL);
		//std::cout << "    Start Parent\n";
		char buff[100];

		close(fd[1]);
		dup2(fd[0], 0);
		//dup2(fdClient, fd[0]);

		close(fdSendBody[1]);
		close(fdSendBody[0]);
		//dup2(fdSendBody[0], 0);

		//std::cout << RED << "        Start loop reading from child\n" << RES;
		for (int ret = 1; ret != 0; ) {
			memset(buff, '\0', 100);
			ret = read(fd[0], buff, 99);
			incomingStr.append(buff);
		}
		//std::cout << BLU "\n       All content read from CGI\n[" << incomingStr << "]\n" << RES;
	}
	return (incomingStr);
}

// Some arguments not used
void printPathParts(std::string str, RequestData reqData) {

	std::cout << "Found path:      [" << BLU << str << RES << "]\n";
//	std::cout << "Path trimmed:    [" << BLU << strTrim << << RES "]\n";
	std::cout << "Path:            [" << PUR << reqData.getPath() << RES << "]\n";
	std::cout << "Path first part: [" << PUR << reqData.getPathFirstPart() << RES << "]\n";
	std::cout << "File/Folder:     [" << PUR << reqData.getPathLastWord() << RES << "]\n";
	std::cout << "File extention:  [" << PUR << reqData.getFileExtention() << RES << "]\n";

	std::map<std::string, std::string> formData;
	formData = reqData.getFormData();

	if (! formData.empty()) {
		std::cout << "\nSTORED FORM DATA PAIRS:\n";// Print the map
		std::map<std::string, std::string>::iterator it;
		for (it = formData.begin(); it != formData.end(); it++)
			std::cout << PUR << "   " << it->first << RES << " ---> " << PUR << it->second << "\n" << RES;
	}
	else	
		std::cout << "Form Data:    " << YEL << "(not present)\n" << RES;
	std::cout << "\n";
}

int checkIfFileExists(const std::string& path) {
	std::ifstream file(path.c_str());

	if (not file.is_open()) {
		std::cout << RED << "Error: File " << path << " not found\n" << RES;
		return (-1);
	}
	std::cout << GRN << "File/folder " << path << " exists\n" << RES;
	return 0;
}

// Not in use
// There is a read buffer overflow
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
