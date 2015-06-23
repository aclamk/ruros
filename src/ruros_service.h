/*
 * ruros_api.h
 *
 *  Created on: May 1, 2015
 *      Author: adam
 */

#ifndef RUROS_SERVICE_H_
#define RUROS_SERVICE_H_

#include <ruros.h>
#include <string>
#include <vector>
namespace ruros
{
struct ServiceServerSide
{
	//service's name
	const char* name;
	//checks if service contains required functions
	bool (*check_functions)(const std::vector<std::string>& func_specs);
	//ask service if will accept client (some may accept only one)
	bool (*accept_connection)(Connection* conn);
	//function to handle calls
	Result (*dispatch_call)(/*Thread* thr, Connection* conn, */std::string& data);
	//notifies when client disconnected (communication lost)
	void (*on_client_disconnect)(Connection* conn);
	//notifies when all calls originating from disconnected client were exited
	void (*on_client_cleanup)(Connection* conn);
};

/*
class ServiceServerSide
{
public:
	virtual bool check_function(std::string f);
	virtual void dispatch_call(Thread* thr, Connection* conn, uint8_t callid, std::string data);
	Connection* getConnection();
	//sets conn if only one Peer implements service, otherwise conn remains unset
	Result acquirePeer(Connection** conn);
	uint8_t Id();
	bool checkConnection(Connection* conn);
	Result acquireConnection(Connection* *conn);
	//checks if this service supports requested function f

};
*/
struct ServiceClientSide
{
	const char* name;
	//lists functions that are required for this client to run
	std::vector<std::string> (*get_required_functions)();
	void (*on_server_disconnected)(Connection* conn);

};
}
#endif /* RUROS_API_H_ */
