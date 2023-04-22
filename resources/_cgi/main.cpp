#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define READ_END 0
#define WRITE_END 1

using namespace std;

int main() {
    cout << "start main" << endl;

    int pipe_IN[2];
    int pipe_OUT[2];
    pid_t pid;

    if (pipe(pipe_IN) == -1) {
        cerr << "Error: Failed to create pipe.\n";
        exit(1);
    }

    pid = fork();

    if (pid == -1) {
        cerr << "Error: Failed to fork.\n";
        exit(1);
    }


    if (pid == 0) {
        // child process
        close(pipe_IN[WRITE_END]);
        close(pipe_OUT[READ_END]);

        dup2(pipe_IN[READ_END], 0);
        dup2(pipe_OUT[WRITE_END], 1);

        // execute Python script
        char *args[3];
        args[0] = (char *)"/usr/bin/python";
        std::string tempPath = "python_cgi_POST.py";
        const char *path = (char *)tempPath.c_str();	//  ie: "./resources/_cgi//python_cgi_GET.py"
        args[1] = (char *)path;
        args[2] = NULL;


        //char* args[] = {"/usr/bin/python", "python_cgi_POST.py", NULL};
        execvp(args[0], args);
        cerr << "Error: Failed to execute Python script.\n";
        exit(1);
    } 
    
    
    
    else {
        // parent process
        close(pipe_IN[READ_END]);
        close(pipe_OUT[WRITE_END]);

        // send data to the child process
        std::string data = "street=Damrak&city=Tokio";
        write(pipe_IN[WRITE_END], data.c_str(), strlen(data.c_str()) + 1);

        // wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // read data from the child process
        char buffer[256];
        read(pipe_OUT[READ_END], buffer, sizeof(buffer));
        cout << "Parent process received: " << buffer << endl;
    }

    return 0;
}
