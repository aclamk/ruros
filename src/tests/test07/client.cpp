#include <ruros.h>
#include <vector>
#include <unix_io.h>
#include <stdio.h>

#include "serialize.h"
using namespace ruros;
#include "serv1.client.cpp"
#include "serv2.server.cpp"

Result Mult(const int& a,const int& b,int& result)
{
   result=a*b;
   return Success;
}

int main(int argc, char** argv)
{
	publishService(&serv2_server_side);
	int fd;
	fd=connect_server("./socket");
	UnixIO* io=new UnixIO(fd);
	ConnectionRef conn;
	conn=newConnection(io);
	if(conn.get()==NULL) return -1;
	bool b;
	b=requestService(conn, &serv1_client_side);
	if(!b) return -2;

	Register();
	sleep(1);
	int i;
	int iloc=1;
	int irpc=1;
	for(i=0;i<10000;i++)
	{
		Add(irpc,3,irpc);
		iloc=iloc*3;
		if(irpc!=iloc)
			return -3;
	}
    return 0;
}
