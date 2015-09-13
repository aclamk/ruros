//============================================================================
// Name        : tru.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#define DBG_MODULE_NAME "RUROS"
#include "dbgout.h"

#include <stdio.h>
#include <stdlib.h>
#include "ruros-priv.h"
#include <pthread.h>

/*
 * ClientA -> Server
 * ClientB -> Server
 *
 * 1. auto connection resolving
 * Client1 <=> Server
 * Client2 <=> Server
 * Client1 -> Server
 * Client1 <- Server
 *
 * 2. manual callback resolving
 * Client1 <=> Server
 * Client2 <=> Server
 * Client1 -> Server
 *            set-connection
 * Client1 <- Server
 *            unset-connection
 *
 * 3. client prematurely disconnected, client removed
 * Client1 <=> Server
 * Client2 <=> Server
 * Client1 -> Server
 *    ## Client1 disconnected ##
 * Client2 <- Server !!!!!
 *
 * 4. client prematurely disconnected, client kept
 * Client1 <=> Server
 * Client2 <=> Server
 * Client1 -> Server
 *         ref Client1
 *    ## Client1 disconnected ##
 *         <- Server cannot call Client1
 *         unref Client1
 */


namespace ruros
{

static std::vector<Thread*> waiting_threads;
static std::vector<Thread*> free_workers;
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
std::vector<Connection*> connections;
std::vector<ServiceServerSide*> published_services;

namespace service0serverside
{
extern struct ServiceServerSide Service0_server_side;
}
namespace service0clientside
{
extern struct ServiceClientSide Service0_client_side;
}


ConnectionRef::ConnectionRef(Connection* _conn): conn(_conn)
{
	if(conn!=NULL) conn->ref();
}
ConnectionRef::ConnectionRef(const ConnectionRef& from)
{
	conn=from.conn;
	if(conn!=NULL) conn->ref();
}
ConnectionRef::ConnectionRef()
{
	conn=NULL;
}
ConnectionRef::~ConnectionRef()
{
	if(conn!=NULL) conn->unref();
}
ConnectionRef& ConnectionRef::operator= (const ConnectionRef& from)
{
	if(this != &from)
	{
		if(conn!=NULL) conn->unref();
		conn=from.conn;
		if(conn!=NULL) conn->ref();
	}
	return *this;
}


ConnectionRef newConnection(RawIO* io)
{
	pthread_mutex_lock(&lock);
	Connection* conn=new Connection();
	conn->io=io;
	conn->ref();
	connections.push_back(conn);
	pthread_t thread;
	if(pthread_create(&thread,NULL,Connection::input_processor_tampoline,conn)!=0)
	{
		conn->unref();//deactivate connection
		connections.pop_back();
		conn=NULL;
	}
	if(conn!=NULL)
	{
		//special resolver service injected to ID 0
		ServiceClientSideID x={0,&service0clientside::Service0_client_side};
		conn->client_services.push_back(x);
		conn->server_services.push_back(&service0serverside::Service0_server_side);
	}
	pthread_mutex_unlock(&lock);
	return ConnectionRef(conn);
}

bool checkConnection(ServiceClientSide* scs, Connection* conn)
{
	int i;
	int c=conn->client_services.size();
	for(i=0;i<c;i++)
	{
		if(conn->client_services[i].service==scs)
			return true;
	}
	return false;
}
//TODO 1. use locking 2. if multiple connections match, error
bool acquireConnection(ServiceClientSide* scs, Connection** out_conn)
{
	int i;
	int c=connections.size();
	for(i=0;i<c;i++)
	{
		Connection* conn=connections[i];
		int ii;
		int cc=conn->client_services.size();
		for(ii=0;ii<cc;ii++)
		{
			if(conn->client_services[ii].service==scs)
			{
				*out_conn=conn;
				conn->ref();
				return true;
			}
		}
	}
	return false;
}


bool requestService(Connection* conn, ServiceClientSide* cli)
{
	bool b;
	b=conn->requestService(cli);
	return b;
}

bool requestService(ConnectionRef conn, ServiceClientSide* cli)
{
	bool b=false;
	if(conn.get()!=NULL)
		b=(conn.get())->requestService(cli);
	return b;
}

void publishService(ServiceServerSide* serv)
{
	//TODO critical section
	if(published_services.size()==0)
		published_services.push_back(&service0serverside::Service0_server_side);
	published_services.push_back(serv);
	//printf("published %s at index %d\n",serv->name,published_services.size()-1);
}


bool (*thread_create)(void* (*thread_func)(void* arg), void* arg)=NULL;

//TODO following need synchonizations
void addWaitingThread(Thread* thr)
{
	pthread_mutex_lock(&lock);
	//printf("+tid=%d\n",thr->original_tid);
	waiting_threads.push_back(thr);
	pthread_mutex_unlock(&lock);
}

void removeWaitingThread(Thread* thr)
{
	pthread_mutex_lock(&lock);
	std::vector<Thread*>::iterator it;
	it=waiting_threads.begin();
	while(it!=waiting_threads.end())
	{
		if(*it==thr)
		{
			waiting_threads.erase(it);
			break;
		}
		it++;
	}
	pthread_mutex_unlock(&lock);
}
/*wakeup threads waiting on specified connection; this happens only on disconnection*/
void wakeupWaitingThreads(Connection* conn)
{
	pthread_mutex_lock(&lock);
	Thread* thr;
	std::vector<Thread*>::iterator it;
	it=waiting_threads.begin();
	while(it!=waiting_threads.end())
	{
		thr=*it;
		if(thr->used_connection==conn)
		{
			thr->wakeup_data="";
			sem_post(&thr->wakeup);
			waiting_threads.erase(it);
			it=waiting_threads.begin();
			continue;
		}
		it++;
	}
	pthread_mutex_unlock(&lock);
}

Thread* popWaitingThread(uint16_t tid)
{
	Thread* thr=NULL;
	std::vector<Thread*>::iterator it;
	pthread_mutex_lock(&lock);
	it=waiting_threads.begin();
	while(it!=waiting_threads.end())
	{
		if((*it)->original_tid==tid)
		{
			thr=*it;
			waiting_threads.erase(it);
			break;
		}
		it++;
	}
	pthread_mutex_unlock(&lock);
	//printf("popped waiting %p\n",thr);
	return thr;
}

Thread* acquireWaitingThread(uint16_t tid)
{
	Thread* thr=NULL;
	std::vector<Thread*>::iterator it;
	pthread_mutex_lock(&lock);
	it=waiting_threads.begin();
	while(it!=waiting_threads.end())
	{
		if((*it)->original_tid==tid)
		{
			thr=*it;
			thr->recursion_count++;
			//waiting_threads.erase(it);
			break;
		}
		it++;
	}
	pthread_mutex_unlock(&lock);
	//printf("popped waiting %p\n",thr);
	return thr;
}

Thread* releaseWaitingThread(uint16_t tid)
{
	Thread* thr=NULL;
	std::vector<Thread*>::iterator it;
	pthread_mutex_lock(&lock);
	it=waiting_threads.begin();
	while(it!=waiting_threads.end())
	{
		if((*it)->original_tid==tid)
		{
			thr=*it;
			thr->recursion_count--;
			if(thr->recursion_count==0)
			{
				waiting_threads.erase(it);
                if(thr->is_worker)
                   free_workers.push_back(thr);//addFreeWorker(thr);
			}
			break;
		}
		it++;
	}
	pthread_mutex_unlock(&lock);
	//printf("popped waiting %p\n",thr);
	return thr;
}


Thread* findWaitingThread(uint16_t tid)
{
	Thread* thr=NULL;
	std::vector<Thread*>::iterator it;
	pthread_mutex_lock(&lock);
	it=waiting_threads.begin();
	while(it!=waiting_threads.end())
	{
		if((*it)->original_tid==tid)
		{
			thr=*it;
			break;
		}
		it++;
	}
	pthread_mutex_unlock(&lock);
	//printf("popped waiting %p\n",thr);
	return thr;
}


void addFreeWorker(Thread* thr)
{
	pthread_mutex_lock(&lock);
	free_workers.push_back(thr);
	//printf("fw size=%d\n",free_workers.size());
	//for(size_t i=0;i<free_workers.size();i++)
	//	printf("FW %d %p\n",i,free_workers[i]);
	pthread_mutex_unlock(&lock);
}
Thread* getFreeWorker()
{
	Thread* thr;
	pthread_mutex_lock(&lock);
	if(!free_workers.empty())
	{
       //printf("fw size=%d\n",free_workers.size());
		thr=free_workers.back();
		free_workers.pop_back();
        //printf("GOT existing thr=%p\n",thr);
	}
	else
	{
		//TODO create worker
		//TODO need critical section
		thr=new Thread();
        thr->is_worker=true;
		if(thread_create!=NULL)
		{
			bool b;
			b=thread_create(Thread::worker_startup, thr);
			if(b) sem_wait(&Thread::worker_created_sem);
		}
		else
		{
			pthread_t* pt=new pthread_t;
			if(pthread_create(pt,NULL,Thread::worker_startup, thr)==0)
			{
				sem_wait(&Thread::worker_created_sem);
			}
		}
        DBG_INFO("Worker thread %p created",thr);
	}
	pthread_mutex_unlock(&lock);
    DBG_INFO_EXT("Worker thread %p picked",thr);
	return thr;
}

ConnectionRef getUsedConnection()
{
	Thread* thr=Thread::getCurrent();
	Connection* conn;
	conn=thr->used_connection;
	return ConnectionRef(conn);
}

bool isdebug()
{
	return true;
}
void debug(const std::string& s)
{
	printf("%s\n",s.c_str());
}

void init() __attribute__((constructor));
void init()
{
	sem_init(&Thread::worker_created_sem,0,0);
	pthread_key_create(&Thread::thread_key,NULL);
}
}
