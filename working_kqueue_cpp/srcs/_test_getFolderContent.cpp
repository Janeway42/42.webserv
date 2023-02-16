#include <iostream>
#include <unistd.h>

int storeFolderContent(char *path)
{
	int     fd[2];
	pid_t   retFork;

	if (pipe(fd) == -1)
		std::cout << "Pipe failed\n";

	retFork = fork();

	if (retFork == 0) {
		sleep(2);
		std::cout << "Start CHILD\n";
		retFork = fork();
		if (retFork < 0)
			std::cout << "Fork failed\n";
		dup2(1, fd[0]);
		close(fd[1]);

		char *arr[4] = {(char*)"/bin/ls", (char*)"-la", path, NULL};
		int ret = execve(arr[0], arr, NULL);
		std::cout << "Execve failed\n";
	}
	else {
		close(fd[0]);

		// sleep(3);
		char buff[100000];

		read(fd[1], buff, 1000);

		std::cout << "\n\n\n\nReturned buffer: [" << "]\n";
	}    
	return (0);
}




int main()
{

	storeFolderContent((char*)"../");


	return (0);
}