/*
 * client.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: adam
 */

#include <semaphore.h>
#include "ruros-priv.h"
#include <string>
#include <stdio.h>
namespace ruros
{

//invoked by generated client stubs
Result client_call(ServiceClientSide* serv,/*uint8_t callid,*/std::string& io_data,std::string* __debug)
{
	Result res;
	//get current thread; this may be either new or worker thread
	Thread* thr=Thread::getCurrent();
	Connection* conn=NULL;
	Connection* conn_selected=thr->getSelectedConnection(); //this is connection selected by user
	Connection* conn_used=thr->getUsedConnection(); //this is connection that this call originated from

	//if connection is selected AND service spans over selected connection
	if(conn_selected!=NULL && checkConnection(serv,conn_selected) )
	{
		//then use it
		conn=conn_selected;
		conn->ref();
	}
	else
	{
		if(conn_used!=NULL && checkConnection(serv,conn_used))
		{
			conn=conn_used;
			conn->ref();
		}
		else
		{
			//this service does not operate on selected connection
			if(!acquireConnection(serv,&conn))
				res=CannotResolve;
			//conn comes here already referenced
		}
	}
	//printf("conn=%p\n",conn);
	if(conn!=NULL)
	{
		//prepare header
		uint32_t length=io_data.size()+4;
		uint16_t tid=thr->getOriginalTid();
		uint8_t buf[9];
		buf[0]=(length>>24)&0xff;
		buf[1]=(length>>16)&0xff;
		buf[2]=(length>>8)&0xff;
		buf[3]=(length>>0)&0xff;
		buf[4]=(tid>>8)&0xff;
		buf[5]=(tid>>0)&0xff;
		buf[6]=CmdCall;
		buf[7]=conn->getServiceID(serv);
		//printf("serviceid=%d\n",buf[7]);
		//buf[8]=callid;

		//conn->register_wakeup(thr);
		//io_data=std::string(buf,9)+io_data;
		//printf("call begin=%d\n",thr->original_tid);
		//thr->prewait();
		if(acquireWaitingThread(tid)==NULL)//TODO create simpler function for this
		{
			addWaitingThread(thr);
			thr->recursion_count=1;
		}
		//printf("CC cal tid=%d conn=%p thr=%p cnt=%d\n",tid,conn,thr,thr->recursion_count);
		if(isdebug())
		{
			char buf[25];
			sprintf(buf,"o_tid=%d CALL ",tid);
			if(__debug)
				debug(buf+*__debug);
			else
				debug(buf);
		}
		res=conn->sendMessage(std::string((char*)buf,8)+io_data);//header,header_size,data,size);
		if(res==Success)
		{
			try
			{
				thr->setUsedConnection(conn);
				res=thr->clientCallwaitReturn(conn,io_data);
				releaseWaitingThread(tid);
				if(isdebug() && __debug!=NULL)
				{
					char buf[25];
					sprintf(buf,"o_tid=%d RET ",tid);
					*__debug=buf;
				}
				thr->setUsedConnection(conn_used);//restore previous used connection
				conn->unref();
			}
			catch(...)
			{
				//thr->cancelwait();
				releaseWaitingThread(tid);
				thr->setUsedConnection(conn_used);
				conn->unref();
				throw;
			}
			//if res==Success than io_data contains message
			if(res==Success)
			{
				//strip 3 bytes of Return response
				io_data=io_data.substr(3);
			}
		}
		else
		{
			//thr->cancelwait();
			releaseWaitingThread(tid);
		}
		//printf("call end=%d rec_cnt=%d\n",thr->original_tid,thr->recursion_count);

	}
	return res;
}

}


