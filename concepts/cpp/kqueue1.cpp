#include "Server.hpp"


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
	try
	{
		Server webserv = new Server();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}	
	
	try
	{
		webserv.runServer();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	









	int loop1 = 0;
	while (1)
	{
		printf("WHILE LOOP ------------------------------ %d\n", loop1);
		int nr_events = kevent(kq, NULL, 0, evList, 32, NULL);
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