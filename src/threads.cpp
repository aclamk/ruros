/*
 * threads.cpp
 *
 *  Created on: Mar 6, 2015
 *      Author: adam
 */
#define DBG_MODULE_NAME "RUROS"
#include "dbgout.h"

#include "ruros-priv.h"
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
namespace ruros
{
pthread_key_t Thread::thread_key;
long gettid()
{
    pid_t tid;
    tid = syscall(SYS_gettid);
    return tid;
}

sem_t Thread::worker_created_sem;


Thread::Thread():
   wakeup_conn(NULL),
   is_worker(false),
   recursion_count(0),
   original_tid(0),
   used_connection(NULL),
   selected_connection(NULL)      
{
   sem_init(&wakeup,0,0);
}
   
Thread* Thread::getCurrent()
{
	void* t;
	t=pthread_getspecific(thread_key);
	if(t!=NULL)
	{
		return (Thread*)t;
	}
	//this is caller thread that has just decided to call something
	Thread* thr=new Thread();
	thr->recursion_count=0;
	thr->original_tid=gettid();
	pthread_setspecific(thread_key,thr);
	return thr;
}

void Thread::prewait()
{
	addWaitingThread(this);
}
void Thread::cancelwait()
{
	removeWaitingThread(this);
}
void Thread::wait(std::string& recv_data)
{
    DBG(INFO,"Thread %p waiting",this);
	//addWaitingThread(this);
	sem_wait(&wakeup);
	recv_data=wakeup_data;
    DBG(INFO,"Thread %p woke up",this);
}

void Thread::worker_loop()
{
	recursion_count=0;
	original_tid=gettid();
	pthread_setspecific(thread_key,this);
	sem_post(&worker_created_sem);
	std::string recv_data;
	Connection* conn;
	bool keep_working=true;
	while(keep_working)
	{
		wait(recv_data);
        DBG(INFO,"Worker for tid=%d",original_tid);
		//conn=getUsedConnection();
        conn=wakeup_conn;
        setUsedConnection(conn);

		if(recv_data.size()>0)
		{
            DBG_ASSERT(conn!=NULL);
			handleCall(conn,recv_data);
			//printf("X\n");
			conn->unref();
		}
		else
			keep_working=false;
		//ruros::addFreeWorker(this);
	}

}

void* Thread::worker_startup(void* arg)
{
	((Thread*)arg)->worker_loop();
	return NULL;
}


Result Thread::clientCallwaitReturn(Connection* conn, std::string& io_data)
{
	Result res;
	//std::string data;
	again:
	//ruros::wait(thr,data);
	this->wait(io_data);
	if(io_data.size()>0)
	{
		//thread_id is already read from message
		//it was used to wake up this thread
		uint8_t command=io_data[2];
		//printf("clientcallwaitr %p cmd=%d\n",this,command);
		switch(command)
		{
			case CmdCall:
			{
				//backcall, this call may be from any registered service
				//Connection* used_connection=this->getUsedConnection();
                Connection* used_connection=wakeup_conn;
                setUsedConnection(used_connection);
				res=this->handleCall(used_connection,io_data);
				//printf("XXXXX\n");
				used_connection->unref();
				if(res==Success)
					goto again;
				break;
			}
			case CmdReturn:
				//this is actual return
				res=Success;
				break;
			case CmdException:
				res=Exception;
				break;
            default:
                DBG_ASSERT(0&&"garbled command");
		}
		//res=Success;
	}
	else
	{
		//exited without data, it must mean disconnection
		res=Disconnected;
	}
	return res;
}

/*this is responsible for execution of calls from clients*/
Result Thread::handleCall(Connection* conn,std::string& io_data)
{
	//thread is already selected
	//the command must be call
	Result res=Exception;
	uint8_t service_id;

	conn->ref();
	conn->call_in();
	//uint8_t call_id;
	assert(io_data[2]==CmdCall);
	service_id=io_data[3];
	io_data=io_data.substr(4);
	//call_id=data[4];
	ServiceServerSide* service_called=conn->getService(service_id);
    DBG_ASSERT(service_called!=NULL);
    DBG(INFO,"CALL tid=%d conn=%p service=%s",original_tid,conn,service_called->name);
	//printf("conn=%p service_id=%d sss=%p %s\n",conn,service_id,service_called,service_called->name);
	try
	{
		res=service_called->dispatch_call(/*this, conn, */io_data);//may cause exceptions
	}
	catch(...)
	{
		//construct exception reply instead of return
		uint32_t length=0+3;
		uint16_t tid=this->getOriginalTid();
		uint8_t buf[7];
		buf[0]=(length>>24)&0xff;
		buf[1]=(length>>16)&0xff;
		buf[2]=(length>>8)&0xff;
		buf[3]=(length>>0)&0xff;
		buf[4]=(tid>>8)&0xff;
		buf[5]=(tid>>0)&0xff;
		buf[6]=CmdException;
		//TODO check result
		releaseWaitingThread(tid);
		conn->sendMessage(std::string((char*)buf,7));
		//we have to isolate subcalls, this is the same as "goto again" but inside exception handler
		this->clientCallwaitReturn(conn,io_data);
		conn->call_out();
		conn->unref();
		throw;
	}
	//note. this is accessible only when exception does not happen
	//process_call(serv);
	if(res==Success)
	{
		//send 'Return' response
		uint32_t length=io_data.size()+3;
		uint16_t tid=this->getOriginalTid();
		uint8_t buf[7];
		buf[0]=(length>>24)&0xff;
		buf[1]=(length>>16)&0xff;
		buf[2]=(length>>8)&0xff;
		buf[3]=(length>>0)&0xff;
		buf[4]=(tid>>8)&0xff;
		buf[5]=(tid>>0)&0xff;
		buf[6]=CmdReturn;
		//printf("CC ret tid=%d conn=%p\n",tid,conn);
        DBG(INFO,"RET  tid=%d conn=%p",tid,conn);
		releaseWaitingThread(tid);
		//printf("CC ret tid=%d conn=%p thr=%p cnt=%d\n",tid,conn,this,this->recursion_count);
		res=conn->sendMessage(std::string((char*)buf,7)+io_data);
        if(res!=Success) DBG(ERR,"Error sending message conn=%p",conn);
		//printf("OUT %p %d thr=%p\n",conn,tid,this);
	}
	conn->call_out();
	conn->unref();
	return res;
}



Connection* Thread::getUsedConnection()
{
	return used_connection;
}
void Thread::setUsedConnection(Connection* conn)
{
	used_connection=conn;
}

void Thread::setOriginalTid(uint16_t tid)
{
	original_tid=tid;
}
uint16_t Thread::getOriginalTid()
{
	return original_tid;
}
void Thread::setSelectedConnection(Connection* conn)
{
	selected_connection=conn;
}
Connection* Thread::getSelectedConnection()
{
	return selected_connection;
}

}
