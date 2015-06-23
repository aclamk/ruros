static std::vector<std::string> serv2_get_required_functions()
{
    std::vector<std::string> res;
    res.push_back("Mult(in int a,in int b,out int result)");
    return res;
}
ServiceClientSide serv2_client_side =
{
    "serv2",
    serv2_get_required_functions,
    NULL/*on_server_disconnect*/
};

Result Mult(const int& a,const int& b,int& result)
{
    Result __res=Success;
    std::string __debug;
    std::string __data;
    __data+=(char)0;
    if(__res==Success) __res=serialize_int(__data,a);
    if(__res==Success) __res=serialize_int(__data,b);
    if(isdebug())
    {
        std::string __s;
        __s+="a="+tostring_int(a)+" ";
        __s+="b="+tostring_int(b)+" ";
        __debug="serv2::Mult( "+__s+")";
    }
    if(__res==Success) __res=client_call(&serv2_client_side,__data,&__debug);
    if(__res==Success) __res=deserialize_int(__data,result);
    if(isdebug())
    {
        std::string __s;
        __s+="result="+tostring_int(result)+" ";
        debug(__debug+" serv2::Mult( "+__s+")");
    }
    return __res;
}


