#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	struct pollfd mypoll;
	char name[250];
	int count = 0;

	//init poll
	memset(&mypoll, 0, sizeof(mypoll));
	mypoll.fd = 0;
	mypoll.events = POLLIN;

	printf("Type name \n");

	while (1)
	{
		poll(&mypoll, 1, 1); 
		// printf("test\n");
		// poll(&mypoll, 1, -1); // blocks indefinetly
		if (mypoll.revents & POLLIN)
		{
			read(0, name, sizeof(name));
			printf("Hi, %s\n", name);
			break ;
		}
		else
			count++;
	}

	printf("Count is %d\n", count);

	return (0);

}