/*
 * tru.h
 *
 *  Created on: Feb 21, 2015
 *      Author: adam
 */

#ifndef TRU_H_
#define TRU_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <vector>
namespace ruros
{
typedef enum
{
	Success=0,
	Exception=1,
	Disconnected=2,
	CannotResolve=3,
	InternalError=4,
	SerializationError=5
} Result;

class RawIO
{
public:
	RawIO(){};
	virtual ~RawIO(){};
	virtual int read(uint8_t* buffer, size_t size)=0;
	virtual int write(const uint8_t* buffer, size_t size)=0;
};


class Connection;
//#ifndef _COMPILING_RUROS
class ConnectionRef
{
public:
	ConnectionRef(Connection* conn);
	ConnectionRef(const ConnectionRef& from);
	ConnectionRef();
	~ConnectionRef();
	ConnectionRef& operator= (const ConnectionRef& from);
	inline Connection* get(){return conn;};
private:
	Connection* conn;
	friend ConnectionRef newConnection(RawIO* io);
	friend ConnectionRef getUsedConnection();
	//friend class Connection;
};
//#endif

ConnectionRef getUsedConnection();
void setUsedConnection(ConnectionRef conn);


struct ServiceServerSide;
struct ServiceClientSide;
//
ConnectionRef newConnection(RawIO* io);
//makes service available for clients
void publishService(ServiceServerSide* serv);
//called by client to connect to server
bool requestService(ConnectionRef conn, ServiceClientSide* cli);

bool isdebug();
void debug(const std::string&);


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

//private method, used by generated code
Result client_call(ServiceClientSide* serv,std::string& io_data,std::string* __debug);

}

#endif /* TRU_H_ */
