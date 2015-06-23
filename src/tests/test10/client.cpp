#include <ruros.h>
#include <vector>
#include <unix_io.h>
#include <stdio.h>

#include "serialize.h"
using namespace ruros;
#include "serv1.client.cpp"
#include "serv2.client.cpp"
#include "serv3.client.cpp"

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
	b=requestService(conn, &serv3_client_side);
	if(!b) return -4;

	int i;
	int iloc=1;
	int irpc=1;
	for(i=0;i<1000;i++)
	{
		Add(irpc,3,irpc);
		iloc=iloc+3;
		if(irpc!=iloc)
			return -5;
		Sub(irpc,i,irpc);
		iloc=iloc-i;
		if(irpc!=iloc)
			return -6;
		Mult(irpc,i,irpc);
		iloc=iloc*i;
		if(irpc!=iloc)
			return -7;
		Div(irpc,3,irpc);
		iloc=iloc/3;
		if(irpc!=iloc)
			return -8;
		Shr(irpc,2,irpc);
		iloc=iloc >> 2;
		if(irpc!=iloc)
			return -9;
		Shl(irpc,3,irpc);
		iloc=iloc << 3;
		if(irpc!=iloc)
			return -10;
	}
    return 0;
}
