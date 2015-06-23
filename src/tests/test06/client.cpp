#include <ruros.h>
#include <vector>
#include <unix_io.h>
#include <stdio.h>

#include "serialize.h"
using namespace ruros;
#include "serv1.client.cpp"
#include "serv2.client.cpp"

int main(int argc, char** argv)
{
	int fd;
	fd=connect_server("./socket");
	UnixIO* io=new UnixIO(fd);
	ConnectionRef conn;
	conn=newConnection(io);
	if(conn.get()==NULL) return -1;
	bool b;
	b=requestService(conn, &serv1_client_side);
	if(!b) return -2;
	b=requestService(conn, &serv2_client_side);
	if(!b) return -3;

	int i;
	int iloc=0;
	int irpc=0;
	for(i=0;i<1000;i++)
	{
		Add(irpc,3,irpc);
		Mult(irpc,5,irpc);
		iloc=iloc+3;
		iloc=iloc*5;
		if(irpc!=iloc)
			return -4;
	}
    return 0;
}
