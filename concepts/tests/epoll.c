#include <sys/epoll.h>
#include <unistd.h>

int main()
{
	int epoll_fd = epoll_create(0);

	if (epoll_fd == -1)
	{
		printf("failed to create epoll\n");
		return (1);
	}

	if (close(epoll_fd) == -1)
	{
		printf("failed to close epoll\n");
		return (1);
	}
	return (0);
}