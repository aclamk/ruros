static std::vector<std::string> serv1_get_required_functions()
{
    std::vector<std::string> res;
    res.push_back("Register()");
    res.push_back("Add(in int a,in int b,out int result)");
    return res;
}
ServiceClientSide serv1_client_side =
{
    "serv1",
    serv1_get_required_functions,
    NULL/*on_server_disconnect*/
};

Result Register()
{
    Result __res=Success;
    std::string __debug;
    std::string __data;
    __data+=(char)0;
    if(isdebug())
    {
        std::string __s;
        __debug="serv1::Register( "+__s+")";
    }
    if(__res==Success) __res=client_call(&serv1_client_side,__data,&__debug);
    if(isdebug())
    {
        std::string __s;
        debug(__debug+" serv1::Register( "+__s+")");
    }
    return __res;
}

Result Add(const int& a,const int& b,int& result)
{
    Result __res=Success;
    std::string __debug;
    std::string __data;
    __data+=(char)1;
    if(__res==Success) __res=serialize_int(__data,a);
    if(__res==Success) __res=serialize_int(__data,b);
    if(isdebug())
    {
        std::string __s;
        __s+="a="+tostring_int(a)+" ";
        __s+="b="+tostring_int(b)+" ";
        __debug="serv1::Add( "+__s+")";
    }
    if(__res==Success) __res=client_call(&serv1_client_side,__data,&__debug);
    if(__res==Success) __res=deserialize_int(__data,result);
    if(isdebug())
    {
        std::string __s;
        __s+="result="+tostring_int(result)+" ";
        debug(__debug+" serv1::Add( "+__s+")");
    }
    return __res;
}


