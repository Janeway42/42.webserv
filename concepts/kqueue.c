#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/event.h>

#define NR_CLIENTS 10

struct client_array
{
	int fd;
}				fd_array[NR_CLIENTS];

	//-------------------------------

	// 	#include <netdb.h>

	// struct addrinfo {
	//     int ai_flags;				// Flags. Includes AI_PASSIVE, AI_CANONNAME, and AI_NUMERICHOST. For a complete list, see <netdb.h>. 
	//     int ai_family;				// Protocol family. Includes PF_UNSPEC and PF_INET. For a complete list, see <sys/socket.h>. 
	//     int ai_socktype;				// Socket type. Includes SOCK_STREAM and SOCK_DGRAM. For a complete list, see <sys/socket.h>. 
	//     int ai_protocol;				// Protocol. Includes IPPROTO_TCP and IPPROTO_UDP. For a complete list, see <netinet/in.h>. 
	//     size_t ai_addrlen;			// The length of the ai_addr member. 
	//     char * ai_canonname;			// The canonical name for nodename.
	//     struct sockaddr * ai_addr;	// Binary socket address. 
	//     struct addrinfo * ai_next	// A pointer to the next addrinfo structure in the linked list. 
	// };

	//--------------------------------

int create_socket_and_listen(void)
{
	// 1. create socket
	struct addrinfo *requested;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);  // memset not an allowed function 
	hints.ai_family = AF_INET;
	hints.ai_family = 
	hints.ai_socktype = SOCK_STREAM;

	// int	getaddrinfo(const char *hostname, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
	// hints stores the type of info that is requested
	// requested stores the requested information after succesful completion of the function
	getaddrinfo("127.0.0.1", "8080", &hints, &requested);

	int server_fd;
	if ((server_fd = socket(requested->ai_family, requested->ai_socktype, requested->ai_protocol)) == 0)
    {
        perror("socket fail\n");
        exit(EXIT_FAILURE);
    }

	// 2. bind it to address (<localhost, port 8080>)
	bind(server_fd, requested->ai_addr, requested->ai_addrlen);
	//  int   listen(int socket, int backlog);
	//  The backlog parameter defines the maximum length for the queue of pending connections.  If a connection request arrives with the queue full, 
	//  the client may receive an error with an indication of ECONNREFUSED.  Alternatively, if the underlying protocol supports retransmission, 
	//	the request may be ignored so that retries may succeed.

	// 3.listen for incloming connections
	listen(server_fd, 5); // sort queue
	return (server_fd);
}


// struct kevent {
//             uintptr_t ident;        /* identifier for this event */
//             uint32_t  filter;       /* filter for event */
//             uint32_t  flags;        /* action flags for kqueue */
//             uint32_t  fflags;       /* filter flag value */
//             int64_t   data;         /* filter data value */
//             void     *udata;        /* opaque user data identifier */
// };

int main()
{
	// 1. create an empty kqueue
	int server_fd = create_socket_and_listen();
	int kq = kqueue();  // allocates a kqueue file descriptor   // does the kq fd need to be non blocking????? 
	if (kq == -1)
		exit_error();
	struct kevent evSet;
	EV_SET(&evSet, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);  // a macro that initializes the kevent structure

	// if multiple sockets
	// struct kevent ev[n];
	// for (int i = 0; i < n, i++)
	// 	EV_SET(&ev[i], socket_fd[i], EVFILT_READ, EV_ADD, 0, 0, 0);

	kevent(kq, &evSet, 1, NULL, 0, NULL);  // purpose? 

	// run_event_loop(kq, server_fd); 
	struct kevent evList;
	while (1)
	{
		// int nr_events = kevent(kq, NULL, 0, evList)

	}
	
	printf("success\n");
	return (EXIT_SUCCESS);
}