#include "ruros.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "unix_io.h"

#include "serialize.h"
using namespace ruros;
namespace BB
{
Result Mult(const int& a,const int& b,int& result);
}

namespace AA
{
#include "serv1.server.cpp"
#include "serv2.client.cpp"

Result Add(const int& a,const int& b,int& result)
{
   //printf("Add %d %d\n",a,b);
	AA::Mult(a,b,result);
	return Success;
}
}

namespace BB
{
#include "serv1.client.cpp"
#include "serv2.server.cpp"

Result Mult(const int& a,const int& b,int& result)
{
   result=a*b;
   //printf("Mult %d %d\n",a,b);
   return Success;
}
}

pthread_t serv_thr;


void* serv_thread(void* arg)
{

	int fd,fda;
	fd=create_server_socket("./socket");
	listen(fd,10);
	while(true)
	{
		fda=accept(fd,NULL,0);
		UnixIO* io=new UnixIO(fda);
		ConnectionRef conn;
		conn=newConnection(io);
		bool b;
		sleep(1);
		b=requestService(conn, &AA::serv2_client_side);
		if(!b) exit(-1);
	}
	return 0;
}

int main(int argc, char** argv)
{
	publishService(&AA::serv1_server_side);
	publishService(&BB::serv2_server_side);

	pthread_create(&serv_thr,NULL,serv_thread,NULL);
	sleep(1);
	int fd;
	fd=connect_server("./socket");
	UnixIO* io=new UnixIO(fd);
	ConnectionRef conn;
	conn=newConnection(io);
	if(conn.get()==NULL) return -1;
	bool b;
	b=requestService(conn, &BB::serv1_client_side);
	if(!b) return -2;
	sleep(2);
	//Register();
	int i;
	int iloc=1;
	int irpc=1;
	for(i=0;i<10000;i++)
	{
		BB::Add(irpc,3,irpc);
		iloc=iloc*3;
		if(irpc!=iloc)
			return -3;
	}
    return 0;
}

