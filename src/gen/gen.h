/*
 * gen.h
 *
 *  Created on: Apr 1, 2015
 *      Author: adam
 */
#include <string>
#include <vector>
namespace gen
{
	typedef enum
	{
		d_in,d_out,d_inout
	} ArgDir;

struct YYpos
{
	std::string file;
	int line;
	int col;
};

struct Arg
{
	ArgDir dir;
	std::string type;
	std::string name;
	YYpos pos;
};

struct Function
{
	std::string name;
	std::vector<Arg> args;
	YYpos pos;
};

struct Option
{
	std::string name;
	std::string value;
	YYpos pos;
};

struct Service
{
	std::string name;
	std::vector<Function> functions;

	std::vector<Option> options;
/*
	std::string client_name;
	std::string client_disconnect;
	std::string server_name;
	std::string server_disconnect;
	std::string server_cleanup;
	std::string server_accept;
*/
};



}
