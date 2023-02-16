#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>

#include <sys/event.h>
#include <sys/time.h>

#include <fcntl.h>

int error(char *str)
{
	printf("%s\n", str);
	return (1);
}

// add a new connection storing to the IP address
int connection_add(int fd)
{

}

int main()
{
	struct addrinfo *addr;
	struct addrinfo hints;

	// open a socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = PF_UNSPEC; // any supported protocol
	hints.ai_flags = AI_PASSIVE; // result from bind()
	hints.ai_socktype = SOCK_STREAM; // tcp
	if (getaddrinfo("127.0.0.1", "8080", &hints, &addr))  // get a list of IP addresses and port numbers for host hostname and service servname.
		return(error("failed getaddrinfo"));
	//int  socket(int domain, int type, int protocol);
	int listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (listen_socket == -1)
		return(error("failed socket"));
	int socket_on;
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on)); // set the option to localy reutilise the socket 	
	if (bind(listen_socket, addr->ai_addr, addr->ai_addrlen) == -1) 
		return(error("failed bind"));
	if (listen(listen_socket, 5) == -1)
		return (error("listen failed"));

	// initialize a queue
	int kq;
	struct kevent evSet; // temporary storage - gets overriden once the info gets passed to kq

	kq = kqueue();  // kqueue file descriptor 
	if (kq == -1)
		return (error("failed kqueue"));
	EV_SET(&evSet, listen_socket, EVFILT_READ, EV_ADD, 0, 0, NULL); // a macro that fills the kevent structure
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // adds the evSet info for this socket/fd in the kq storage
	{
		close(kq);
		return(error("failed kevent EVFILT_READ add"));
	}

	// The following loop waits for events and then uses any combination of meaningful conditions 
	// to determine what the event is, and what should be done about it. 

	struct kevent evList[32];
	int nr_events;
	int i; 
	int fd; // temporary storage - gets overriden once the info gets passed to kq
	struct sockaddr sock_addr;
	socklen_t socklen = sizeof(addr);


	while (1)
	{
	// 	int		kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
		nr_events = kevent(kq, NULL, 0, evList, 32, NULL);  // used to listen 
		if (nr_events < 1)
			return (error("failed kevent number events"));

		for (i = 0; i < nr_events; i++)
		{
			printf("incoming connection'''\n");
			fd = evList[i].ident; // file descriptor of the incoming request

			if (evList[i].flags & EV_EOF)  // if the connection exists and sends an EOF flag 
			{
				printf("connection closed\n");
				fd = evList[i].ident;
				EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);  // overkill ? 
				// ........
			}

			else if (evList[i].ident == listen_socket)  // incoming request from a new client 
			{
				fd = accept(evList[i].ident, (struct sockaddr *)&addr, &socklen); // returns the non negative file descriptor of the newly accepted socket
				if (fd == -1)
					return (error("failed accept"));  // SHOULD EXIT? OR JUST REFUSE CONNECTION AND MOVE ON? 
				fcntl(fd, F_SETFL, O_NONBLOCK);  // non blocking file descriptor/socket
				EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
				if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
				{
					close(kq);
					return(error("failed kevent EVFILT_READ add"));
				}
				send(fd, "hello!", 6, 0);
			}

			else if (evList[i].filter == EVFILT_READ)
			{
				char buffer[1024];
				recv(evList[i].ident, &buffer, sizeof(buffer), 0);
				printf("buffer received: %s\n", buffer);				
			}

			else if (evList[i].filter == EVFILT_WRITE)
			{
				
			}
		}
	}






	return (0);
}




// Server sockets are generally bound to well known names (ports, in this case) and they establish themselves with listen(). 
// That is where the real difference happens, as client sockets establish themselves with connect(). 
// Calling listen() on a socket causes the kernel's tcp/ip implementation to begin accepting connections sent to the socket's bound name (port). 
// This will happen whether or not you ever call accept().
// accept() simply gives your server a way to access and interact with the client sockets that have connected to your listening socket.
