#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <string>

#include "../ruros.h"
int create_server_socket(std::string name)
{
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;
	pid_t child;

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return -1;
	}

	unlink(name.c_str());

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path), "%s",name.c_str());

	if(bind(socket_fd,
			(struct sockaddr *) &address,
			sizeof(struct sockaddr_un)) != 0)
	{
		printf("bind() failed\n");
		close(socket_fd);
		return -1;
	}
	return socket_fd;
}

int connect_server(std::string name)
{
	struct sockaddr_un address;
	int  socket_fd, nbytes;
	char buffer[256];

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return -1;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path), "%s",name.c_str());

	if(connect(socket_fd,
			(struct sockaddr *) &address,
			sizeof(struct sockaddr_un)) != 0)
	{
		printf("connect() failed\n");
		close(socket_fd);
		return -1;
	}
	return socket_fd;
}



