#include <ruros.h>
#include <vector>
#include <unix_io.h>
#include <stdio.h>

#include "serialize.h"
using namespace ruros;
#include "add.client.cpp"

int main(int argc, char** argv)
{
	int fd;
	fd=connect_server("./test01_socket");
	UnixIO* io=new UnixIO(fd);
	ConnectionRef conn;
	conn=newConnection(io);
	printf("a1\n");
	bool b;
	b=requestService(conn, &Calc_client_side);
	printf("a2 %d\n",b);
    int cc;
    Add(10,20,cc);
    printf("a3 %d\n",cc);
    if(cc==30) return 0;
    return -1;
}
