#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

	//create socket
	//bind it to address (<localhost, port 8080>)
	//listen
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

int create_socket_and_listen()
{
	struct addrinfo *requested;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
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

	bind(server_fd, requested->ai_addr, requested->ai_addrlen);
	//  int   listen(int socket, int backlog);
	//  The backlog parameter defines the maximum length for the queue of pending connections.  If a connection request arrives with the queue full, the client may
    //  receive an error with an indication of ECONNREFUSED.  Alternatively, if the underlying protocol supports retransmission, the request may be ignored so that
    //  retries may succeed.
	listen(server_fd, 5);


	return (server_fd);
}

int main()
{


	return (0);
}