
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>


int main() {

std::cout << "START CALL_CGI, _cgi path: \n";
	//(void)reqData;

	// Declare all necessary variables
	std::string comspec			= "COMSPEC=";
	std::string request_method	= "REQUEST_METHOD=";
	std::string query_string	= "QUERY_STRING=";
	std::string server_name		= "SERVER_NAME=";
	std::string content_length	= "CONTENT_LENGTH=";

	// Declare a vector and fill it with variables, with attached =values
	std::vector<std::string> temp;
	temp.push_back(comspec.append("default"));
	temp.push_back(request_method.append("default"));
	temp.push_back(query_string.append("default"));
	temp.push_back(server_name.append("default"));

	std::stringstream ss;	// convert length int to string variable
	ss << "30";
	temp.push_back(content_length.append(ss.str()));

	// Make a char** array and copy all content of the above vector
	char **env = new char*[temp.size()  + 1];

	size_t i = 0;
	for (i = 0; i < temp.size(); i++) {
		env[i] = new char[temp[i].length() + 1];
		std::strcpy(env[i], temp[i].c_str());
	}
	env[i] = NULL;

	char *args[3];
	args[0] = (char *)"/usr/bin/python";
	std::string tempPath = "python_cgi_POST.py";
	const char *path = (char *)tempPath.c_str();	//  ie: "./resources/_cgi//python_cgi_GET.py"
	args[1] = (char *)path;
	args[2] = NULL;


	// Cleanup
	for (size_t j = 0; j < temp.size(); j++) {
		delete env[j];
	}
	delete[] env;




    char buff[500];

	int ret = 0;
	pid_t		retFork;
    int mypipeIN[2];
    int mypipeOUT[2];
    ret = pipe(mypipeIN);
    if (ret < 0)
		std::cout << "Error: creating pipIN\n";
    pipe(mypipeOUT);
    if (ret < 0)
		std::cout << "Error: creating pipOUT\n";

	retFork = fork();
    if (retFork < 0)
		std::cout << "Error: creating FORK\n";

	if (retFork == 0) { // CHILD
		std::cout << "Start CHILD execve()\n";
		if (retFork < 0)
			std::cout << "Error: Fork failed\n";


        close(mypipeIN[1]);
        ret = dup2(mypipeIN[0],  0);     // child reads from parent
        if (ret < 0)
			std::cout << "Error: pipe in[0] failed\n";
        close(mypipeIN[0]);

        
        
        close(mypipeOUT[0]);
        ret = dup2(mypipeOUT[1],  1);     // child writes to parent
        if (ret < 0)
			std::cout << "Error: pipe OUT[1] failed\n";
		std::cout <<  "   child e)\n" ;
        close(mypipeOUT[1]);

		std::cout <<  "Before execve in child\n" ;
		ret = execve(args[0], args, env);
		std::cout << "Error: Execve failed: " << ret << "\n" ;
	}


	else {				// PARENT
		std::cout << "    Start Parent\n";
        std::string str = "street=Damrak&city=Tokio";



        close(mypipeIN[1]);
		std::cout <<  "   parent a)\n" ;

		// ret = dup2(mypipeOUT[1]   ,  1);	// parent writes to _cgi via pipe fd_in
        //  if (ret < 0)
		// 	std::cout << "Error: pipe IN[1] failed\n";
		// std::cout <<  "   parent b)\n" ;
        
		ret = write(mypipeIN[0], "street=Damrak&city=Tokio", 24);
        if (ret < 0)
			std::cout << "Error: write failed\n";
        close(mypipeIN[0]);
       // close(mypipeOUT[1]);
		
        //close(mypipeIN[0]);
        //  ret = dup2(mypipeIN[0]   ,  0);	// parent reads from _cgi
        //  if (ret < 0)
		// 	std::cout << "Error: pipe OUT[0] failed\n";

       close(mypipeOUT[1]);


		memset(buff, '\0', 1000);
		ret = read(mypipeOUT[0], buff, 999);
        if (ret < 0)
			std::cout << "Error: read failed\n";
		//check ret
		std::cout << "Ret: read from CGI, from _fd_out[0]:  " << ret << "\n";
		//incomingStr.append(buff);	// HOW TO KEEP APPENDING, IF THERE IS MORE DATA THEN BUFFER SIZE ???
		std::cout <<"\n       All content read from CGI\n[" << buff << "]\n" ;
		//std::cout << BLU "\n       End parent\n" << RES;
		wait(NULL);
	}
	return (0);
}



