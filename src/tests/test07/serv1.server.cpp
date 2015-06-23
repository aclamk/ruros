Result Register();
Result Add(const int& a,const int& b,int& result);
Result serv1_dispatch(std::string& __data)
{
    uint8_t __func_id;
    Result __res=Success;
    if(__data.size()>0)
    {
        __func_id=__data[0];
        __data=__data.substr(1);
    }
    else
    {__res=CannotResolve;return __res;}
    switch(__func_id)
    {
    case 0:
    {
        if(__res==Success) __res=Register();
        __data.resize(0);
    break;
    }
    case 1:
    {
        int a;
        int b;
        int result;
        if(__res==Success) __res=deserialize_int(__data,a);
        if(__res==Success) __res=deserialize_int(__data,b);
        if(__res==Success) __res=Add(a,b,result);
        __data.resize(0);
        if(__res==Success) __res=serialize_int(__data,result);
    break;
    }
    }
return __res;
}

bool serv1_check_functions(const std::vector<std::string>& func_specs)
{
    for(size_t i=0;i<func_specs.size();i++)
    {
        if(func_specs[i]=="Register()") continue;
        if(func_specs[i]=="Add(in int a,in int b,out int result)") continue;
        return false;
    }
    return true;
}
struct ServiceServerSide serv1_server_side =
{
    "serv1",
    serv1_check_functions,/*check_functions*/
    NULL,/*accept_connection*/
    serv1_dispatch,/*dispatch_call*/
    NULL,/*on_client_disconnect*/
    NULL/*on_client_cleanup*/
};

