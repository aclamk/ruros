/*
 * message_processor.cpp
 *
 *  Created on: Feb 17, 2015
 *      Author: adam
 */

#define DBG_MODULE_NAME "RUROS"
#include "dbgout.h"

#include <stdlib.h>
#include <semaphore.h>
#include "ruros-priv.h"
#include <stdio.h>
#include <assert.h>
/*
 * message:
 * length(4bytes)
 * content
 *
 * content:
 * byte 0: 0-register 1-call 2-return 3-exception
 *
 * register:
 * ?
 *
 *
 * call:
 * byte 0: caller_id(tid)
 * byte 2: CALL=1
 * byte 3: service_id
 * byte 4: function_id
 * bytes... params
 *
 * return:
 * byte 0: caller_id(tid)
 * byte 2: RETURN=2
 * ------: service is fixed at call
 * ------: function_id is fixed at call
 * bytes... params
 *
 *
 *
 */

namespace ruros
{
void* Connection::input_processor_tampoline(void* arg)
{
	return ((Connection*)arg)->input_processor();
}

void* Connection::input_processor()
{
	ref();
	std::string data;
	size_t size;
	Result read_res=Success;
	while(Success==(read_res=this->recvMessage(data)))
	{
		Thread* thr;
		//read success
		uint16_t caller_tid;
		caller_tid=((uint16_t)data[0])<<8 | (uint8_t)data[1];
		uint8_t command=data[2];
		switch(command)
		{
		case CmdCall:
            DBG(DUMP,"Thread %d calls",caller_tid);
			//execute call incoming from remote end
			//this is either new call, and we have to wake up any worker thread
			//or it is callback that has to wake specific thread
			thr=acquireWaitingThread(caller_tid);
			if(thr==NULL)
			{
				//no such waiting thread, elect some worker
				thr=getFreeWorker();
                if(thr==NULL)
                {
                    DBG(ERR,"Worker thread unavailable");
                }
				//printf("got free worker %p is_worker=%d\n",thr,thr->is_worker);
				//TODO!!! handle problem if thread cannot be found
				thr->setOriginalTid(caller_tid);
				addWaitingThread(thr);
				thr->recursion_count=1;
			}

			thr->wakeup_data=data;
			//thr->setUsedConnection(this);
            thr->wakeup_conn=this;
			this->ref();
			//printf("IN conn=%p tid=%d\n",this,caller_tid);
			sem_post(&thr->wakeup);
			//no more processing here...
			break;

		case CmdReturn:
            DBG(DUMP,"Thread %d return",caller_tid);            
			//some thread must be waiting to receive
			thr=findWaitingThread(caller_tid);
			//thr=ruros::releaseWaitingThread(caller_tid);
            DBG_ASSERT(thr!=NULL&&"no thread waiting for return");
			//wyscig miedzy releasewaiting a obudzeniem threada, ktos mu moze wpisac
			thr->wakeup_data=data;
			sem_post(&thr->wakeup);
			break;
		default:
            DBG(ERR,"Received garbled command\n");
			//TODO add handling
			assert(0);
		}
	}
	//this means disconnected

	std::vector<ServiceClientSideID>::iterator it_c;
	it_c=this->client_services.begin();
	while(it_c!=client_services.end())
	{
		if(it_c->service->on_server_disconnected!=NULL)
			it_c->service->on_server_disconnected(this);
		it_c++;
	}

	std::vector<ServiceServerSide*>::iterator it_s;
	it_s=this->server_services.begin();
	while(it_s!=server_services.end())
	{
		if((*it_s)->on_client_disconnect!=NULL)
			(*it_s)->on_client_disconnect(this);
		it_s++;
	}
	try_on_client_cleanup();
	unref();

	return NULL;
}


void Connection::ref()
{
	//TODO
	int cnt;
	cnt=__sync_add_and_fetch(&ref_cnt,1);
	assert(cnt>0);
	//printf("++conn=%p cnt=%d\n",this,cnt);
}
void Connection::unref()
{
	int cnt;
	cnt=__sync_sub_and_fetch(&ref_cnt,1);
	assert(cnt>=0);
	//printf("--conn=%p cnt=%d\n",this,cnt);
	if(cnt==0)
	{
		//TODO
		//delete, possibly..
	}

}

void Connection::call_in()
{
	int cnt;
	cnt=__sync_add_and_fetch(&incoming_calls_cnt,1);
	//printf("++conn=%p calls=%d\n",this,cnt);
	assert(cnt>0);

}
void Connection::call_out()
{
	int cnt;
	cnt=__sync_sub_and_fetch(&incoming_calls_cnt,1);
	//printf("--conn=%p calls=%d\n",this,cnt);
	assert(cnt>=0);
	if(cnt==0)
	{
		//emit cleanup function
		try_on_client_cleanup();
	}
}

void Connection::try_on_client_cleanup()
{
	//printf("XXXXXX state=%d\n",state);
	pthread_mutex_lock(&lock);
	if(state==Closed)
	{
		if(incoming_calls_cnt==0)
		{
           //printf("calling for cleanup %d\n",this->server_services.size());
			for(size_t i=0;i<this->server_services.size();i++)
			{
				if(server_services[i]->on_client_cleanup!=NULL)
					server_services[i]->on_client_cleanup(this);
			}
            state=ClosedCleanedup;
		}
	}
	pthread_mutex_unlock(&lock);
}


static Result read_all(RawIO* io,uint8_t* buffer,size_t size)
{
	Result res=Success;
	int rcnt=0;
	int r;
	while(rcnt<size)
	{
		r=io->read(buffer+rcnt,size-rcnt);
		if(r<=0)
		{
			res=Disconnected;
			break;
		}
		rcnt+=r;
	}
	if(0)
	{
	for(int i=0;i<rcnt;i++)
		printf("r:%2.2x ",buffer[i]);
	printf("\n");
	}
	return res;
}
static Result write_all(RawIO* io, const uint8_t* buffer,size_t size)
{
	Result res=Success;
	int rcnt=0;
	int r;
	while(rcnt<size)
	{
		r=io->write(buffer+rcnt,size-rcnt);
		if(r<=0)
		{
			res=Disconnected;
			break;
		}
		rcnt+=r;
	}
	if(0)
	{
	for(int i=0;i<rcnt;i++)
		printf("w:%2.2x ",buffer[i]);
	printf("\n");
	}
	return res;
}
Result Connection::sendMessage(const std::string msg)
{
	Result res;
	if(state==Closed) return Disconnected;
	res=write_all(io,(uint8_t*)msg.c_str(),msg.size());
	if(res==Disconnected)
	{
		pthread_mutex_lock(&lock);
		if(state==Active)
			state=Closed;
		pthread_mutex_unlock(&lock);
		//try_on_client_cleanup();
	}
	return res;
}
//provided msg will not contain size, only payload
Result Connection::recvMessage(std::string& msg)
{
	Result res;
	uint8_t size_h[4];
	uint32_t size;
	if(state==Closed) return Disconnected;
	res=read_all(io,size_h,4);
	if(res==Success)
	{
		size=(size_h[0]<<24)+(size_h[1]<<16)+(size_h[2]<<8)+(size_h[3]<<0);
		uint8_t message[size];
		res=read_all(io,message,size);
		if(res==Success)
		{
			msg=std::string((char*)message,size);
		}
	}

	if(res==Disconnected)
	{
		pthread_mutex_lock(&lock);
		if(state==Active)
			state=Closed;
		pthread_mutex_unlock(&lock);
		//try_on_client_cleanup();
	}
	return res;
}
bool validateService(Connection* conn, ServiceClientSide* scs,uint16_t& service_id);
bool Connection::requestService(ServiceClientSide* scs)
{
	//Thread::getCurrent()->is_worker;
	bool accepted;
	uint16_t service_id;
	accepted=validateService(this,scs,service_id);
	if(accepted)
	{
		ServiceClientSideID scs_id;
		scs_id.service=scs;
		scs_id.id=service_id;//TODO
		client_services.push_back(scs_id);
	}
	//printf("conn=%p accepted=%d service_id=%d\n",this,accepted,service_id);
	return accepted;
}

uint8_t Connection::getServiceID(ServiceClientSide* scs)
{
	uint8_t res=0;
	//TODO critical section
	int i;
	int c=client_services.size();
	for(i=0;i<c;i++)
	{
		if(client_services[i].service==scs)
		{
			res=client_services[i].id;
			break;
		}
	}
	return res;
}
ServiceServerSide* Connection::getService(uint8_t service_id)
{
	ServiceServerSide* sss=NULL;
	/*if(service_id<server_services.size())
		sss=server_services[service_id];*/
	if(service_id<published_services.size())
		sss=published_services[service_id];
	return sss;
}
#if 0
void Connection::signalDisconnection()
{
	//TODO LOCK!
	printf("SIGNAL DISCONNECTION\n");
	for(size_t i=0;i<this->server_services.size();i++)
	{
		if(server_services[i]->on_client_disconnect!=NULL)
			server_services[i]->on_client_disconnect(this);
	}

	for(size_t i=0;i<this->client_services.size();i++)
	{
		if(client_services[i].service->on_server_disconnected!=NULL)
			client_services[i].service->on_server_disconnected(this);
	}
}
#endif
}
/*

Client call:
+ thr
SEND
wait return
- thr

Reader thread:
1) RECV, call
+ thr
wakeup
2) RECV, return
wakeup

Worker:
wait call
dispatch
- thr
SEND



 */

