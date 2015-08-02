/*
 * gen.cpp
 *
 *  Created on: Apr 2, 2015
 *      Author: adam
 */
#include "gen.h"
#include <stdio.h>
std::string string(int i)
{
	char str[11];
	sprintf(str,"%d",i);
	return str;
}


const gen::Option* getOption(const gen::Service& service,const std::string& name)
{
	int i;
	for(i=0;i<service.options.size();i++)
	{
		if(service.options[i].name==name) return &service.options[i];
	}
	return NULL;
}

std::string generate_function_prototype(const gen::Function& function)
{
	std::string out;
	out+=
	"Result "+function.name+"(";
	for(size_t i=0;i<function.args.size();i++)
	{
		if(function.args[i].dir==gen::d_in)
			out+="const "+function.args[i].type+"& "+function.args[i].name;
		else
			out+=function.args[i].type+"& "+function.args[i].name;
		if(i!=function.args.size()-1) out+=",";
	}
	out+=");\n";
	return out;
}

std::string generate_function_dispatch(const gen::Function& function)
{
	std::string out;
	int i,k;
	out+=
	"    {\n";
	for(i=0;i<function.args.size();i++)
		out+=
		"        "+function.args[i].type+" "+function.args[i].name+";\n";
	for(i=0;i<function.args.size();i++)
		if(function.args[i].dir==gen::d_in || function.args[i].dir==gen::d_inout)
			out+=
			"        if(__res==Success) __res=deserialize<"+function.args[i].type+">(__data,"+function.args[i].name+");\n";
	out+="        if(__res==Success) __res="+function.name+"(";
	for(i=0;i<function.args.size();i++)
	{
			if(i!=0) out+=",";
		out+=function.args[i].name;
	}
	out+=");\n";
	out+="        __data.resize(0);\n";
	for(i=0;i<function.args.size();i++)
		if(function.args[i].dir==gen::d_out || function.args[i].dir==gen::d_inout)
			out+=
			"        if(__res==Success) __res=serialize<"+function.args[i].type+">(__data,"+function.args[i].name+");\n";
	out+=
	"    break;\n"
	"    }\n";
	return out;
}

std::string generate_dispatch(const gen::Service& service)
{
	std::string out;
	for(size_t f=0;f<service.functions.size();f++)
	{
		out+=generate_function_prototype(service.functions[f]);
	}
	out+=
	"Result "+service.name+"_dispatch(std::string& __data)\n"
	"{\n"
	"    uint8_t __func_id;\n"
	"    Result __res=Success;\n"
	"    if(__data.size()>0)\n"
	"    {\n"
	"        __func_id=__data[0];\n"
	"        __data=__data.substr(1);\n"
	"    }\n"
	"    else\n"
	"    {__res=CannotResolve;return __res;}\n"
	"    switch(__func_id)\n"
	"    {\n";
	int f;
	for(f=0;f<service.functions.size();f++)
	{
		out+=
		"    case "+string(f)+":\n";
		out+=generate_function_dispatch(service.functions[f]);
	}
	out+=
	"    }\n"
	"return __res;\n"
	"}\n";
	return out;
}


std::string generate_client_stubs(const gen::Service& service)
{
	std::string out;
	int f;
	for(f=0;f<service.functions.size();f++)
	{
		const gen::Function& func=service.functions[f];
		out+=
		"Result "+func.name+"(";
		int i;
		for(i=0;i<func.args.size();i++)
		{
			if(i!=0) out+=",";
			if(func.args[i].dir==gen::d_in)
				out+="const "+func.args[i].type+"& "+func.args[i].name;
			if(func.args[i].dir==gen::d_out)
				out+=func.args[i].type+"& "+func.args[i].name;
			if(func.args[i].dir==gen::d_inout)
				out+=func.args[i].type+"& "+func.args[i].name;
		}
		out+=
		")\n"
		"{\n"
		"    Result __res=Success;\n";
		if(service.debug)
           out+="    std::string __debug;\n";
        out+=
		"    std::string __data;\n"
		"    __data+=(char)"+string(f)+";\n";
		for(i=0;i<func.args.size();i++)
		{
			if(func.args[i].dir==gen::d_in||func.args[i].dir==gen::d_inout)
			{
				out+="    if(__res==Success) __res=serialize<"+func.args[i].type+">(__data,"+func.args[i].name+");\n";
			}
		}
        if(service.debug)
        {
           out+=
           "    if(isdebug())\n"
           "    {\n"
           "        std::string __s;\n";
           for(i=0;i<func.args.size();i++)
           {
              if(func.args[i].dir==gen::d_in||func.args[i].dir==gen::d_inout)
              {
                 out+="        __s+=\""+func.args[i].name+"=\"+"+
                 "tostring<"+func.args[i].type+">("+func.args[i].name+")+\" \";\n";
              }
           }
           out+="        __debug=\""+service.name+"::"+func.name+"( \"+__s+\")\";\n";
           //out+="        __debug.scs="+service.name+"_client_side;\n";
           out+="    }\n";
        }
        if(service.debug)
           out+=
           "    if(__res==Success) __res=client_call(&"+service.name+"_client_side,__data,&__debug);\n";
        else
           out+=
           "    if(__res==Success) __res=client_call(&"+service.name+"_client_side,__data,NULL);\n";
           
		for(i=0;i<func.args.size();i++)
		{
			if(func.args[i].dir==gen::d_out||func.args[i].dir==gen::d_inout)
			{
						out+="    if(__res==Success) __res=deserialize<"+func.args[i].type+">(__data,"+func.args[i].name+");\n";
			}
		}
        if(service.debug)
        {
           out+=
           "    if(isdebug())\n"
           "    {\n"
           "        std::string __s;\n";
           for(i=0;i<func.args.size();i++)
           {
              if(func.args[i].dir==gen::d_out||func.args[i].dir==gen::d_inout)
              {
                 out+="        __s+=\""+func.args[i].name+"=\"+"+
                 "tostring<"+func.args[i].type+">("+func.args[i].name+")+\" \";\n";
              }
           }
           out+="        debug(__debug+\" "+service.name+"::"+func.name+"( \"+__s+\")\");\n";
           out+="    }\n";
        }
		out+=
		"    return __res;\n"
		"}\n"
		"\n";
	}
	return out;
}


std::string func_specs(const gen::Function& func)
{
	std::string out;
	out+="\""+func.name+"(";
	int k;
	for(k=0;k<func.args.size();k++)
	{
		if(func.args[k].dir==gen::d_in)	out+="in";
		if(func.args[k].dir==gen::d_out)out+="out";
		if(func.args[k].dir==gen::d_inout)out+="inout";
		out+=" "+func.args[k].type+" "+func.args[k].name;
		if(k!=func.args.size()-1) out+=",";
	}
	out+=")\"";
	return out;
}

std::string namespace_name(const std::string name)
{
	size_t t=0;
	std::string str;
	if((t=name.rfind("::"))!=std::string::npos)
		str=name.substr(t+2);
	else
		str=name;
	return str;
}
std::string namespace_open(const std::string name)
{
	size_t t=0;
	size_t s;
	std::string str;
	while((s=name.find("::",t))!=std::string::npos)
	{
		str+="namespace "+name.substr(t,s-t)+" { ";
		t=s+2;
	}
	return str;
}
std::string namespace_close(const std::string name)
{
	size_t t=0;
	size_t s;
	std::string str;
	while((s=name.find("::",t))!=std::string::npos)
	{
		str+="}";
		t=s+2;
	}
	return str;
}

std::string generate_service_server_side(const gen::Service& service)
{
	std::string out;
	const gen::Option* opt;
	out+=
	"bool "+service.name+"_check_functions(const std::vector<std::string>& func_specs)\n"
	"{\n"
	"    for(size_t i=0;i<func_specs.size();i++)\n"
	"    {\n";
	for(int i=0;i<service.functions.size();i++)
	{
		const gen::Function& func=service.functions[i];
		out+=
		"        if(func_specs[i]=="+func_specs(func)+") continue;\n";
	}
	out+=
	"        return false;\n"
	"    }\n"
	"    return true;\n"
	"}\n";

	opt=getOption(service,"server accept");
	if(opt!=NULL)
		out+=
		namespace_open(opt->value)+"\n"
		"bool "+namespace_name(opt->value)+"(Connection* conn);\n"+
		namespace_close(opt->value)+"\n";
	opt=getOption(service,"server disconnect");
	if(opt!=NULL)
		out+=
		namespace_open(opt->value)+"\n"
		"void "+namespace_name(opt->value)+"(Connection* conn);\n"+
		namespace_close(opt->value)+"\n";
	opt=getOption(service,"server cleanup");
	if(opt!=NULL)
		out+=
		namespace_open(opt->value)+"\n"
		"void "+namespace_name(opt->value)+"(Connection* conn);\n"+
		namespace_close(opt->value)+"\n";

	out+=
	"struct ServiceServerSide "+service.name+"_server_side =\n"
	"{\n";
	out+=
	"    \""+service.name+"\",\n";
	out+=
	"    "+service.name+"_check_functions,/*check_functions*/\n";
	opt=getOption(service,"server accept");
	if(opt!=NULL)
	out+="    "+opt->value+",/*accept_connection*/\n";
	else
	out+="    NULL,/*accept_connection*/\n";

	out+="    "+service.name+"_dispatch,/*dispatch_call*/\n";

	opt=getOption(service,"server disconnect");
	if(opt!=NULL)
	out+="    "+opt->value+",/*on_client_disconnect*/\n";
	else
	out+="    NULL,/*on_client_disconnect*/\n";

	opt=getOption(service,"server cleanup");
	if(opt!=NULL)
	out+="    "+opt->value+"/*on_client_cleanup*/\n";
	else
	out+="    NULL/*on_client_cleanup*/\n";
	out+=
	"};\n";
	return out;
}


std::string generate_service_client_side(const gen::Service& service)
{
	std::string out;
	out+=
	"static std::vector<std::string> "+service.name+"_get_required_functions()\n"
	"{\n"
	"    std::vector<std::string> res;\n";
	for(int i=0;i<service.functions.size();i++)
	{
		const gen::Function& func=service.functions[i];
		out+=
		"    res.push_back("+func_specs(func)+");\n";
	}
	out+=
	"    return res;\n"
	"}\n";
	out+=
	"ServiceClientSide "+service.name+"_client_side =\n"
	"{\n"
	"    \""+service.name+"\",\n"+
	"    "+service.name+"_get_required_functions,\n";
	const gen::Option* opt;
	opt=getOption(service,"client disconnect");
	if(opt!=NULL)
		out+=
		"    "+opt->value+"/*on_server_disconnect*/\n";
	else
		out+=
		"    NULL/*on_server_disconnect*/\n";
	out+=
	"};\n";
	return out;
}
/*
struct ServiceServerSide Service0_server_side =
{
	.check_functions = NULL,
	.accept_connection = NULL,
	.dispatch_call = Service0_dispatch_call,
	.on_client_disconnect = Service0_on_client_disconnect,
	.on_client_cleanup = Service0_on_client_cleanup
};
*/


