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
	fd=connect_server("./socket");
	UnixIO* io=new UnixIO(fd);
	ConnectionRef conn;
	conn=newConnection(io);
	if(conn.get()==NULL) return -1;
	bool b;
	b=requestService(conn, &Calc_client_side);
	if(!b) return -2;
	int i;
	int ii=0;
	for(i=0;i<1000;i++)
	{
		Add(ii,1,ii);
		//printf("%d\n",ii);
	}
	if(ii!=1000)
		return -3;
    return 0;
}
