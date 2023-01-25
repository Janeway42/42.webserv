// Kqueue provides efficient input and output event pipelines between the kernel and userland. 
//Thus, it is possible to modify event filters as well as receive pending events while using 
//only a single system call to kevent(2) per main event loop iteration. 
//This contrasts with older traditional polling system calls such as poll(2) and select(2) 
// which are less efficient, especially when polling for events on numerous file descriptors. 

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netdb.h>

#include <fcntl.h>

#include <sys/event.h>


int error(char *str)
{
	printf("failed %s\n", str);
	return (1);
}

int error_kq(int kq, char *str)
{
	close(kq);
	return (error(str));
}

int main()
{

	// -------------------------- listening socket ---------------------------------------------------------

	// start a listening socket 
	// socket address structure:
	// struct addrinfo {
	// 	int ai_flags;           /* input flags */
	// 	int ai_family;          /* protocol family for socket */
	// 	int ai_socktype;        /* socket type */
	// 	int ai_protocol;        /* protocol for socket */
	// 	socklen_t ai_addrlen;   /* length of socket-address */
	// 	struct sockaddr *ai_addr; /* socket-address for socket */
	// 	char *ai_canonname;     /* canonical name for service location */
	// 	struct addrinfo *ai_next; /* pointer to next in list */
	// };

	// we need one addrinfo struct hints where we set the characteristics of the soccket we want to create
	// getaddrinfo() takes the structure hints and returns a list of IP addresses and port numbers for the inputed characteristics

	struct addrinfo hints;
	struct addrinfo *addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC; // the caller will accept any protocol family supported by the operating system.
	hints.ai_flags = AI_PASSIVE; // it indicates that the returned socket address structure is intended for use in a call to bind.
	hints.ai_socktype = SOCK_STREAM; // the type of socket that is wanted: SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW.  When ai_socktype is zero the caller will accept any socket type.
	if (getaddrinfo("127.0.0.1", "8080", &hints, &addr) != 0)
		return (error("getaddrinfo"));

	// we then use the info gathered with getaddrinfo in order to create the socket
	int listening_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (listening_socket == -1)
		return (error("socket"));
	fcntl(listening_socket, F_SETFL, O_NONBLOCK); // set the non-blocking property

	// set the socket options so that it can be reused locally 
	// SOL_SOCKET flag means that the changes are done on a socket level
	// SO_REUSEADDR allows the server to start and bind its port even if there is a previously established connection in use 
	// for example a child process can still use the socket even if the main process has closed its connection 
	// listen is a willingness to accept incoming connections (uses a queue limit for pending connections)
	int socket_on = 1;
	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
	if (bind(listening_socket, addr->ai_addr, addr->ai_addrlen) == -1)
		return (error("bind"));
	// listen:  willingness to accept incoming connections and a queue limit for incoming connections 
	if (listen(listening_socket, 5) == -1)
		return (error("listen"));
	
	// -------------------------- kqueue initialization ---------------------------------------------------------

	int kq; // the file descriptor used to communicate 
	struct kevent evSet; // temporary storage where EV_SET puts necesary info, get overridden once the info gets passed to kq 

	kq = kqueue(); // the file descriptor used to communicate 
	if (kq == -1)
		return (error("kqueue"));
	EV_SET(&evSet, listening_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // passes the info stored in evSet to the kq
	{
		close (kq);
		return (error("kevent start"));
	}

	struct kevent evList[32]; // stores the incoming events that need to be processed 
	int nr_events;
	int i;
	int fd; // tenporary storage - gets overriden once the info gets passed to kq
	struct sockaddr_storage socket_addr;
	// struct sockaddr - stores IPv6, struct sockaddr_in - stores IPv4 , struct sockaddr_storage - stores both IPv4 and IPv6
	socklen_t socklen = sizeof(socket_addr);

	int loop1 = 0;
	while (1)
	{
		printf("WHILE LOOP: %d\n", loop1);
		nr_events = kevent(kq, NULL, 0, evList, 32, NULL);
		printf("NR EVENTS: %d\n", nr_events);

		if (nr_events < 1)
			return (error_kq(kq, "number events"));

		for (i = 0; i < nr_events; i++)
		{
			printf("incoming connection ...\n");
			fd = evList[i].ident; // file descriptor of the incoming request
			printf("I: %d\n", i);

			if (evList[i].flags & EV_EOF)  // end connection ???? 
			{
				printf("connection closed\n");
				EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);    /// is it necessary???????
                if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // pass info 
					return (error_kq(kq, "kevent"));
				close(fd); // socket is automatically removed from kq by the kernel
			}

			else if (fd == listening_socket) // incoming connection from a new client
			{
				printf("FD: %d\n", fd);
				fcntl(fd, F_SETFL, O_NONBLOCK);  // add the non-block property
				// accept creates a new client socket which inherits the properties of the original socket. 
				fd = accept(evList[i].ident, (struct sockaddr *)&socket_addr, &socklen);
				if (fd == -1)
					return (error_kq(kq, "accept"));
				EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL); // fill evSet
				if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // pass info 
					return (error_kq(kq, "kevent"));
				EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL); // fill evSet
				if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // pass info 
					return (error_kq(kq, "kevent"));
			}

			else if (evList[i].filter == EVFILT_READ)
			{
				char buffer[1024];
				recv(fd, &buffer, sizeof(buffer), 0);
				printf("buffer received and read: %s\n", buffer);
				printf("loop1: %d\n", loop1);

				send(fd, "Hello back from read!\n", 22, 0);
				// char *buffer1;
				// char *loop;
				// loop = itoa(loop1, buffer1, 10);
				// send(fd, loop, strlen(loop), 0);
			}

			else if (evList[i].filter == EVFILT_WRITE)
			{
				printf("write loop - loop1: %d\n", loop1);

				send(fd, "HTTP/2 200 OK\n", 14, 0);
				send(fd, "Content-Lenght: 24\n", 19, 0);
				send(fd, "\n", 1, 0);
				send(fd, "Hello back from write!\n", 23, 0);
				EV_SET(&evSet, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);    /// is it necessary???????
                if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // pass info 
					return (error_kq(kq, "kevent write"));
				// close(fd);
				// send(fd, evList[i].udata, sizeof(evList[i].udata), 0);
				// if (send(fd, evList[i].udata, sizeof(evList[i].udata), 0) == -1)  ??????????
			}
		}
		loop1++;
		// if (loop1 == 20)
		// 	break ;
	}
	close(kq); // necessary??
	freeaddrinfo(addr);
	return (0);
}



// struct kevent {
// 	uintptr_t       ident;          /* identifier for this event */
// 	int16_t         filter;         /* filter for event */
// 	uint16_t        flags;          /* general flags */
// 	uint32_t        fflags;         /* filter-specific flags */
// 	intptr_t        data;           /* filter-specific data */
// 	void            *udata;         /* opaque user data identifier */
// };

// The filter field specifies the type of kernel event. If it is either EVFILT_READ or EVFILT_WRITE, kqueue works similar to epoll. 
// In this case, the ident field represents a file descriptor. The ident field may represent other types of identifiers, 
// such as process ID and signal number, depending on the filter type.

// The udata field is passed in and out of the kernel unchanged, and is not used in any way. The usage of this
// field is entirely application dependent, and is provided as a way to efficiently implement a function dispatch
// routine, or otherwise add an application identifier to the kevent structure