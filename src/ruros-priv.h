/*
 * tru-priv.h
 *
 *  Created on: Feb 26, 2015
 *      Author: adam
 */

#ifndef TRU_PRIV_H_
#define TRU_PRIV_H_



#include <semaphore.h>
#include <string>
#include <vector>

#include <stdio.h>

#if 0
#define _COMPILING_RUROS
namespace ruros
{
class Connection;
class ConnectionRef
{
public:
	ConnectionRef(Connection* conn);
	ConnectionRef(const ConnectionRef& from);
	ConnectionRef();
	~ConnectionRef();
	ConnectionRef& operator= (const ConnectionRef& from);
	Connection* get();
public:
	Connection* conn;
};
}
#endif
#include "ruros.h"

namespace ruros
{


typedef enum
{
	CmdCall=1,
	CmdReturn=2,
	CmdException
} Commands;


class Thread
{
public:
	static Thread* getCurrent();
	//void call(Connection* conn,uint8_t* msg,size_t size);

	void prewait();
	void cancelwait();
	//provided message is stripped from size part, only payload
	void wait(std::string& recv_data);

	//void handleCall();
	static void* worker_startup(void* arg);
	void worker_loop();
	Result clientCallwaitReturn(Connection* conn, std::string& io_data);
	Result handleCall(Connection* conn,std::string& data_in);


	void setOriginalTid(uint16_t t);
	uint16_t getOriginalTid();
	//Connection* getConnection();

	void setSelectedConnection(Connection* conn);
	Connection* getSelectedConnection();

	Connection* getUsedConnection();
	void setUsedConnection(Connection* conn);

	sem_t wakeup;
	std::string wakeup_data;
    Connection* wakeup_conn;
	bool is_worker;
	int recursion_count;
	uint16_t original_tid;
	Connection* used_connection;
	Connection* selected_connection;

	static pthread_key_t thread_key;
	static sem_t worker_created_sem;

	Result debug_beforecall_res;
	ServiceClientSide* debug_beforecall_serv;
	std::string debug_beforecall_func;
	std::string debug_beforecall_params;

	ServiceClientSide* debug_oncall_serv;

};
/*
class ServiceServer
{
public:
	void dispatch_call(Thread* thr, Connection* conn, uint8_t callid, std::string data);
	Connection* getConnection();
	//sets conn if only one Peer implements service, otherwise conn remains unset
	Result acquirePeer(Connection** conn);
	uint8_t Id();
	bool checkConnection(Connection* conn);
	Result acquireConnection(Connection* *conn);
	bool check_function(std::string f);
};





class Connection
{
public:
	bool setRawIO(RawIO* io);
	bool registerServer(ServiceServer* sserver);
	bool connectClient(ServiceClient* sclient);
};

class Connection_priv : public Connection
{
	Result send_message(const std::string& data);
	Result read_message(std::string& data);
	//bool do_read(uint8_t* &data,size_t &size);
	//Service* find_service(uint8_t service_id);
	void ref();
	void unref();
	ServiceServer* findServer(uint8_t service_id);
};
*/

}
/*
 * SERVER:
 * 1. create empty ruros R (singleton)
 * 2. service *discover* is automatically added to R
 * 3. register service S to R
 * 4. somehow wait for connection (type RawIO)
 * 5. create connection C from RawIO
 * 6. attach C to R
 *
 * CLIENT:
 * 1. create empty ruros R (singleton)
 * 2. connect to server (type RawIO)
 * 3. create connection C from RawIO
 * 4. request service S on C
 */

namespace ruros
{



#include "ruros_service.h"

bool checkConnection(ServiceClientSide* scs, Connection* conn);
bool acquireConnection(ServiceClientSide* scs, Connection** conn);

//Result wait(Thread* thr,std::string& data_out);
void addWaitingThread(Thread* thr);
void removeWaitingThread(Thread* thr);
Thread* popWaitingThread(uint16_t tid);

Thread* acquireWaitingThread(uint16_t tid);
Thread* releaseWaitingThread(uint16_t tid);
Thread* findWaitingThread(uint16_t tid);



void addFreeWorker(Thread* thr);
Thread* getFreeWorker();

//pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

//Thread* getCurrentThread();
ServiceServerSide* findService(uint8_t service_id);

struct ServiceClientSideID
{
	uint8_t id;
	ServiceClientSide* service;
};

class Connection
{
public:
	Connection():io(NULL),ref_cnt(0),incoming_calls_cnt(0),state(Active),lock(PTHREAD_MUTEX_INITIALIZER){};
	bool setRawIO(RawIO* io);
	//bool registerServer(ServiceServerSide* sserver);
	bool requestService(ServiceClientSide* scs);

	//msg must not contain size, only payload
	Result sendMessage(const std::string msg);
	//provided msg will not contain size, only payload
	Result recvMessage(std::string& msg);
	//finds service by ID. this is used to find which service was meant by client
	ServiceServerSide* getService(uint8_t service_id);
	RawIO* io;

	//retrieves ID that was returned for this service by server on handshake
	uint8_t getServiceID(ServiceClientSide* scs);


	ConnectionRef GetNULLConnectionRef();
	void try_on_client_cleanup();
	//void signalDisconnection();
	//lists services that we requested over this connection
	std::vector<ServiceClientSideID> client_services;
	//lists services that we accepted to provide
	std::vector<ServiceServerSide*> server_services;
	void ref();
	void unref();
	volatile int ref_cnt; //reference counter for connection. when ref_cnt==0 it means object is no more referenced
	void call_in();
	void call_out();
	volatile int incoming_calls_cnt; //number of client requests that are currently handled by server. used to signal 'cleanup'
	typedef enum { Closed=1, Active=2, ClosedCleanedup=3 } State;
	State state;
	pthread_mutex_t lock;

	void* input_processor();
	static void* input_processor_tampoline(void* arg);

};


//list of connections
extern std::vector<Connection*> connections;
extern std::vector<ServiceServerSide*> published_services;
};


#endif /* TRU_PRIV_H_ */
