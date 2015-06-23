/*
 * unix_io.h
 *
 *  Created on: Mar 14, 2015
 *      Author: adam
 */

#ifndef UNIX_IO_H_
#define UNIX_IO_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ruros.h>
int create_server_socket(std::string name);
int connect_server(std::string name);
class UnixIO: public ruros::RawIO
{
public:
	int fd;
	UnixIO(int fd){this->fd=fd;};
	~UnixIO(){};
	int read(uint8_t* buffer, size_t size)
	{
		int r=::read(fd,buffer,size);
		return r;
	}
	int write(const uint8_t* buffer, size_t size)
	{
		int w=::write(fd,buffer,size);
		return w;
	}
};





#endif /* UNIX_IO_H_ */
