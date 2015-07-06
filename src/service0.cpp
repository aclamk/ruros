/*
 * source.x.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: adam
 */
#include <string>
#include <vector>
#include "ruros-priv.h"
#include "serialize.h"
#include <stdio.h>

//using namespace ruros;
//namespace service0serverside
//{
//#include "service0.server.cpp"
//}
//namespace ruros
//{
namespace ruros
{

namespace service0serverside
{
Result validate(const std::vector<std::string>& rq_funcs, uint16_t& service_id, bool& accepted)
{
	//check if all requested functions are implemented
	Thread* thr=Thread::getCurrent();
	Connection* conn=thr->getUsedConnection();
	std::vector<ServiceServerSide*>::iterator it;
	bool b=false;
	//TODO critical section for operation on connection!
	for(size_t i=0;i<published_services.size();i++)
	{
		b=published_services[i]->check_functions(rq_funcs);
		//printf("c=%d i=%d b=%d\n",published_services.size(),i,b);
		if(b==true)
		{
			service_id=i;//conn->server_services.size();
			conn->server_services.push_back(published_services[i]);
			//printf("SIZE=%d\n",conn->server_services.size());
			break;
		}
	}
	accepted=b;
	return Success;
}

#include "service0.server.cpp"
}

namespace service0clientside
{
#include "service0.client.cpp"
}

}


namespace ruros
{
bool validateService(Connection* conn, ServiceClientSide* scs,uint16_t& service_id)
{
	Result res;
	Thread* thr=Thread::getCurrent();
	Connection* saved_conn=thr->getSelectedConnection();
	thr->setSelectedConnection(conn);
	bool accepted;
	//res=Service0_validateService(scs->get_required_functions(),&accepted);
	res=service0clientside::validate(
			scs->get_required_functions(), service_id, accepted);
	if(res!=Success) accepted=false;
	thr->setSelectedConnection(saved_conn);
	return accepted;
}
}
