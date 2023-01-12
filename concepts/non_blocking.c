 #include <fcntl.h>
 #include <poll.h> 

    //  struct pollfd {
    //     int   fd;         /* file descriptor */
    //     short events;     /* requested events */
    //     short revents;    /* returned events */
    // };

 int main(void)
 {
	int server_fd;
	int fc;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	// F_GETFL - get descriptor status flag
	// int fcntl(file descriptor, command to be applied, arguments);

	//F_SETFL overwrites the flags with exactly what you pass in. 
	// If you want to set a specific flag and leave the other flags as-is,
	// then you must F_GETFL the old flags, | the new flag in, 
	// and then F_SETFL the result. This must be done as two separate system calls;
	fc = fcntl(server_fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);


	// poll



 }
