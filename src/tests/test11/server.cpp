#include "ruros.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "unix_io.h"
#include <signal.h>
#include <sys/socket.h>

#include "serialize.h"
using namespace ruros;
volatile uint32_t status=0;

int client_fd=0;
namespace SERVER
{
#include "serv1.server.cpp"

Result Add(const int& a,const int& b,int& result)
{
	printf("Add %d %d\n",a,b);
	result=a+b;
	if(result==16) shutdown(client_fd,SHUT_RDWR);
	sleep(1);
	return Success;
}


bool accept_connection(Connection* conn)
{
	printf("accept connection\n");
	return true;
}


void client_disconnected(Connection* conn)
{
	printf("client disconnected\n");
	uint32_t x;
	x=__sync_or_and_fetch(&status,1);
	printf("x=%d\n",x);
	if((x&2) != 0) exit(-10); //client_disconnected must be before all_calls_finished
}


void all_calls_finished(Connection* conn)
{
	printf("all calls finished\n");
	uint32_t x;
	x=__sync_or_and_fetch(&status,2);
	if((x&1) != 1) exit(-11); //client_disconnected must be before all_calls_finished

}


}

namespace CLIENT
{

void server_disconnected(Connection* conn)
{
	printf("server disconnected\n");
	uint32_t x;
	x=__sync_or_and_fetch(&status,16);
}

#include "serv1.client.cpp"
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
		sleep(1);
	}
	return 0;
}

int main(int argc, char** argv)
{
	publishService(&SERVER::serv1_server_side);
	signal(SIGPIPE,SIG_IGN);
	pthread_create(&serv_thr,NULL,serv_thread,NULL);
	sleep(1);
	//int client_fd;
	client_fd=connect_server("./socket");
	UnixIO* io=new UnixIO(client_fd);
	ConnectionRef conn;
	conn=newConnection(io);
	if(conn.get()==NULL) return -1;
	bool b;
	b=requestService(conn, &CLIENT::serv1_client_side);
	if(!b) return -2;
	sleep(2);
	//Register();
	int i;
	int iloc=1;
	int irpc=1;
	for(i=0;i<5;i++)
	{
		printf("i=%d\n",i);
		CLIENT::Add(irpc,3,irpc);
		iloc=iloc+3;
		//if(irpc!=iloc)
		//	return -3;
	}
	printf("-------------------\n");
	sleep(2);
	if(status!=(1|2|16)) exit(-12);

    return 0;
}

