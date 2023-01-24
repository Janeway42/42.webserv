#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>

#include <fcntl.h>

#include <sys/event.h>
// --------- cpp -----------
#include "kqueue.hpp"


typedef std::vector<requestStorage>::iterator vectIt;

// -------------------------

int error(std::string str)
{
	std::cout << "failed" << str << std::endl;
	return (EXIT_FAILURE);
}

int errorKq(int kq, std::string str)
{
	close(kq);
	return (error(str));
}

vectIt getLocation(std::vector<requestStorage> storage, int fd)
{
	vectIt it;
	for (it = storage.begin(); it < storage.end(); it++)
	{
		if (it->fd == fd)
			return (it);
	}
	return (it);
}

void prepareRequest(requestStorage* storage, vectIt location)
{
	// extract from location->buffer the method 
	// get post delete methods 
	// method is an int assigned to the type of request 
	switch(method)
	{
		case 1:
			prepareGet();
			break ;
		case 2:
			preparePost();
			break ;
		case 3:
			prepareDelete();
			break ;
		default:
			std::cout << "error request page";
	}
}

int main()
{

	struct addrinfo hints;
	struct addrinfo *addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", "8080", &hints, &addr) != 0)
		return (error("getaddrinfo"));

	int listening_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (listening_socket == -1)
		return (error("socket"));
	fcntl(listening_socket, F_SETFL, O_NONBLOCK);

	int socket_on = 1;
	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_on, sizeof(socket_on));
	if (bind(listening_socket, addr->ai_addr, addr->ai_addrlen) == -1)
		return (error("bind"));
	if (listen(listening_socket, 5) == -1)
		return (error("listen"));

	// -------------------------- kqueue initialization ---------------------------------------------------------

	int kq;
	struct kevent evSet;

	kq = kqueue();
	if (kq == -1)
		return (error("kqueue"));
	EV_SET(&evSet, listening_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		close (kq);
		return (error("kevent start"));
	}

	struct kevent evList[32];
	int nr_events;
	int i;
	int fd;
	struct sockaddr_storage socket_addr;
	socklen_t socklen = sizeof(socket_addr);
	std::vector <requestStorage> storage;

	while (1)
	{
		nr_events = kevent(kq, NULL, 0, evList, 32, NULL);
		printf("NR EVENTS: %d\n", nr_events);

		if (nr_events < 1)
			return (errorKq(kq, "number events"));

		for (i = 0; i < nr_events; i++)
		{
			printf("incoming connection ...\n");
			fd = evList[i].ident;

			// find the storage location in the vector, if none return -1
			vectIt location = getLocation(storage, fd);
			

			if (evList[i].flags & EV_EOF)
			{
				printf("connection closed\n");
				if (location != storage.end())
					storage.erase(location);
				close(fd); // socket is automatically removed from kq by the kernel

			}

			else if (fd == listening_socket) // incoming connection from a new client
			{
				printf("FD: %d\n", fd);
				fcntl(fd, F_SETFL, O_NONBLOCK);  // add the non-block property
				// accept creates a new client socket which inherits the properties of the original socket. 
				fd = accept(evList[i].ident, (struct sockaddr *)&socket_addr, &socklen);
				if (fd == -1)
					return (errorKq(kq, "accept"));
				EV_SET(&evSet, fd, EVFILT_READ | EVFILT_WRITE, EV_ADD, 0, 0, NULL); // fill evSet
				if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) // pass info 
					return (errorKq(kq, "kevent"));
			}

			else if (evList[i].filter == EVFILT_READ)
			{
				char buffer[1024];
				recv(fd, &buffer, sizeof(buffer), 0);
				// store the info in the buffer storage
				if (location == storage.end())
				{
					requestStorage temp;
					temp.fd == fd;
					temp.buffer = buffer;
					storage.push_back(temp);
				}
				else
					location->buffer.append(buffer);

				// if the reading is complete process the rwquest and preppare the response
				if (readComplete())
					prepareRequest(storage, location);

				printf("buffer received and read: %s\n", buffer);
				send(fd, "message back: Hello to you too!\n", 19, 0);
			}

			else if (evList[i].filter == EVFILT_WRITE)
			{
				// does write go in packages? do I need a while loop? 
				printf("write loop\n");
				switch(location->method)
				{
					case 1:
						processGet();
						break ;
					case 2:
						processPost();
						break ;
					case 3:
						processDelete();
						break ;
					default:
						std::cout << "error process page";
				}
				// send(fd, evList[i].udata, sizeof(evList[i].udata), 0);
				// // if (send(fd, evList[i].udata, sizeof(evList[i].udata), 0) == -1)  ??????????
			}
		}
	}
	return (EXIT_SUCCESS);
}