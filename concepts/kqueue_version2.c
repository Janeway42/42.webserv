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

#define BUFFER_SIZE 1024


int error(char *str)
{
	printf("failed %s\n", str);
	return (1);
}

int error_kq(int kq, struct addrinfo *addr, char *str)
{
	close(kq);
	freeaddrinfo(addr);
	return (error(str));
}

int close_connection(int kq, struct kevent event, int filter)
{
	struct kevent evSet;

	printf("connection closed - read \n");
	EV_SET(&evSet, event.ident, filter, EV_DELETE, 0, 0, NULL); 
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) 
		return (1);
	close(event.ident); 
	return (0);
}

int main()
{

	// -------------------------- listening socket ---------------------------------------------------------

	struct addrinfo hints;
	struct addrinfo *addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC; 
	hints.ai_flags = AI_PASSIVE; 
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", "8080", &hints, &addr) != 0)
		return (error("getaddrinfo"));

	// we then use the info gathered with getaddrinfo in order to create the socket
	int listening_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (listening_socket == -1)
		return (error("socket"));
	fcntl(listening_socket, F_SETFL, O_NONBLOCK); // set the non-blocking property

	// set the socket options so that it can be reused locally 
	int socket_on = 1;
	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
	if (bind(listening_socket, addr->ai_addr, addr->ai_addrlen) == -1)
		return (error("bind"));
	// listen:  willingness to accept incoming connections and a queue limit for incoming connections 
	if (listen(listening_socket, 5) == -1)
		return (error("listen"));
	
	// -------------------------- kqueue initialization ---------------------------------------------------------

	int kq;
	struct kevent evSet;

	kq = kqueue(); // the file descriptor used to communicate 
	if (kq == -1)
		return (error("kqueue"));
	EV_SET(&evSet, listening_socket, EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_WRITE	, 0, NULL);
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) 
	{
		close (kq);
		return (error("kevent start"));
	}

	struct kevent evList[32];
	int i;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);

	int loop1 = 0;
	while (1)
	{
		printf("WHILE LOOP ------------------------------ %d\n", loop1);
		int nr_events = kevent(kq, NULL, 0, evList, 32, NULL);
		printf("NR EVENTS: %d\n", nr_events);

		if (nr_events < 1)
			return (error_kq(kq, addr, "number events"));

		for (i = 0; i < nr_events; i++)
		{
			printf("filter: %hd\n", evList[i].filter);

			if (evList[i].flags & EV_ERROR)
				printf("Event error\n");

			else if (evList[i].ident == listening_socket)
			{
				printf("START\n");
				
				int opt_value = 1;
				int fd = accept(evList[i].ident, (struct sockaddr *)&socket_addr, &socklen);
				fcntl(fd, F_SETFL, O_NONBLOCK);  
				setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
				if (fd == -1)
					return (error_kq(kq, addr, "accept"));
				EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL); 
				if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) 
					return (error_kq(kq, addr, "kevent add read"));
				EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL); 
				if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) 
					return (error_kq(kq, addr, "kevent add write"));				
			}
			
			else if (evList[i].filter == EVFILT_READ)
			{
				if (evList[i].flags & EV_EOF)
				{
					if (close_connection(kq, evList[i], EVFILT_READ) == 1)
						return (error_kq(kq, addr, "kevent read eof"));
				}
				else
				{
					printf("READ\n");
					char buffer[BUFFER_SIZE];
					int ret = recv(evList[i].ident, &buffer, sizeof(buffer), 0);
					if (ret < 0)
					{
						printf("error reading\n");
						close_connection(kq, evList[i], EVFILT_READ);
					}
					// if (ret == 0)
					// 	finished reading
					// else
					// 	append the new part of the message 
					printf("buffer received and read: %s\n", buffer);
					printf("bytes read: %d\n", ret);
					memset(&buffer, '\0', sizeof(buffer));
				}

			}

			else if (evList[i].filter == EVFILT_WRITE)
			{
				if (evList[i].flags & EV_EOF)
				{
					if (close_connection(kq, evList[i], EVFILT_WRITE) == 1)
						return (error_kq(kq, addr, "kevent read eof"));
				}
				else
				{
					printf("WRITE\n");

					send(evList[i].ident, "HTTP/2 200 OK\n", 14, 0);
					send(evList[i].ident, "Content-Length: 23\n", 19, 0);
					send(evList[i].ident, "\n", 1, 0);
					int ret = send(evList[i].ident, "Hello back from write!\n", 23, 0);
					// if (ret < 0)
					// 	error
					printf("ret: %d\n", ret);
					shutdown(evList[i].ident, SHUT_RDWR);  // not allowed && doesn't make a difference
					close(evList[i].ident);
					printf("closed connection from write\n");
				}				
			}
			printf("\n");
		}
		printf("\n");
		loop1++;
	}
	close(kq); 
	freeaddrinfo(addr);
	return (0);
}
