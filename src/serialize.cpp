#include <serialize.h>

#include <ruros.h>
#include <stdio.h>
using namespace ruros;
namespace ruros
{
   template<> ruros::Result serialize<>(std::string& data,const uint32_t& x)
   {
      unsigned char buf[4];
      buf[0]=(x>>24)&0xff;
      buf[1]=(x>>16)&0xff;
      buf[2]=(x>>8)&0xff;
      buf[3]=(x>>0)&0xff;
      data+=std::string((char*)buf,4);
      return Success;
   }
   template<> ruros::Result deserialize<>(std::string& data,uint32_t& x)
   {
      if(data.size()<4)
         return CannotResolve;
      uint8_t *d=(uint8_t*)data.c_str();
      x=(d[0]<<24)+(d[1]<<16)+(d[2]<<8)+(d[3]<<0);
      data=data.substr(4);
      return Success;
   }
   template<> std::string tostring<>(const uint32_t& x)
   {
      char c[15];
      sprintf(c,"0x%8.8x",x);
      return (char*)c;
   }

   
   
   template<> ruros::Result serialize<>(std::string& data,const int& x)
   {
      unsigned char buf[4];
      unsigned int y=x;
      buf[0]=(y>>24)&0xff;
      buf[1]=(y>>16)&0xff;
      buf[2]=(y>>8)&0xff;
      buf[3]=(y>>0)&0xff;
      data+=std::string((char*)buf,4);
      return Success;
   }
   template<> ruros::Result deserialize<>(std::string& data,int& x)
   {
      if(data.size()<4)
         return CannotResolve;
      uint8_t *d=(uint8_t*)data.c_str();
      x=(d[0]<<24)+(d[1]<<16)+(d[2]<<8)+(d[3]<<0);
      data=data.substr(4);
      return Success;
   }
   template<> std::string tostring<>(const int& x)
   {
      char c[12];
      sprintf(c,"%d",x);
      return (char*)c;
   }

   
   
   template<> ruros::Result serialize<>(std::string& data,const std::string& x)
   {
      Result res=Success;
      if(res==Success)res=serialize<int>(data,(int)x.size());
      data+=x;
      return res;
   }

   template<> ruros::Result deserialize<>(std::string& data,std::string& x)
   {
      Result res=Success;
      int count;
      if(res==Success)res=deserialize<int>(data,count);
      if(res==Success)
      {
         if(data.size()>=count)
         {
			x=data.substr(0,count);
			data=data.substr(count);
         }
         else
			res=InternalError;
      }
      return res;
   }

   template<> std::string tostring<>(const std::string& x)
   {
      std::string s;
      s="\""+x+"\"";
      return s;
   }


   
   template<> ruros::Result serialize<>(std::string& data,const vector_of_string& x)
   {
      Result res=Success;
      int count=x.size();
      if(res==Success)res=serialize<int>(data,x.size());
      for(size_t i=0;i<count&&res==Success;i++)
         res=serialize<std::string>(data,x[i]);
      return res;
   }
   
   template<> ruros::Result deserialize<>(std::string& data,vector_of_string& x)
   {
      Result res=Success;
      x.resize(0);
      int count;
      if(res==Success)res=deserialize<int>(data,count);
      for(size_t i=0;i<count&&res==Success;i++)
      {
         std::string s;
         res=deserialize<std::string>(data,s);
         x.push_back(s);
      }
      return res;
   }

   template<> std::string tostring<>(const vector_of_string& x)
   {
      std::string s="[";
      for(size_t i=0;i<x.size();i++)
      {
         if(i==0)
			s+=x[i];
         else
			s+=","+x[i];
      }
      s+="]";
      return s;
   }


   
   template<> ruros::Result serialize<>(std::string& data,const uint16_t& x)
   {
      unsigned char buf[2];
      buf[0]=(x>>8)&0xff;
      buf[1]=(x>>0)&0xff;
      data+=std::string((char*)buf,2);
      return Success;
   }
   
   template<> ruros::Result deserialize<>(std::string& data,uint16_t& x)
   {
      if(data.size()<2)
         return CannotResolve;
      x=(data[0]<<8)+(data[1]<<0);
      data=data.substr(2);
      return Success;
   }

   template<> std::string tostring<>(const uint16_t& x)
   {
      char c[11];
      sprintf(c,"%u",x);
      return (char*)c;
   }


   
   template<> ruros::Result serialize<>(std::string& data,const bool& x)
   {
      if(x)
         data+='\001';
      else
         data+='\000';
      return Success;
   }
   
   template<> ruros::Result deserialize<>(std::string& data,bool& x)
   {
      if(data.size()==0)
         return CannotResolve;
      if(data[0]=='\001') x=true;
      else if(data[0]=='\000') x=false;
      else return CannotResolve;
      return Success;
   }

   template<> std::string tostring<>(const bool& x)
   {
      std::string s;
      if(x) s="true"; else s="false";
      return s;
   }

}
