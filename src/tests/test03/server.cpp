#include "ruros.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "unix_io.h"

#include <ruros.h>
#include "serialize.h"
using namespace ruros;
#include "add.server.cpp"

Result Add(const int& a,const int& b,int& result)
{
   result=a+b;
   return Success;
}

int main(int argc, char** argv)
{
	publishService(&Calc_server_side);
	int fd,fda;
	fd=create_server_socket("./socket");
	listen(fd,10);
	while(true)
	{
		fda=accept(fd,NULL,0);
		UnixIO* io=new UnixIO(fda);
		ConnectionRef conn;
		conn=newConnection(io);
	}
	return 0;
}
