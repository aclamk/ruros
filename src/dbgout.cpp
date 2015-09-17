#include <sys/types.h>
#include <pthread.h>
#include <string>
#include <vector>
struct debug_module
{
      std::string name;
      int* level;
      bool* traceblocks;
      bool* mono;         
};
static std::vector<debug_module> *debug_modules=NULL;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

void __dbg__register_module(const char* name, int* level, bool* traceblocks, bool* mono)
{
   debug_module d;
   d.name=name;
   d.level=level;
   d.traceblocks=traceblocks;
   d.mono=mono;
   pthread_mutex_lock(&lock);
   if(debug_modules==NULL) debug_modules=new std::vector<debug_module>;
   debug_modules->push_back(d);
   pthread_mutex_unlock(&lock);
}

void DBG_set_level(const char* name,int level)
{
   pthread_mutex_lock(&lock);
   std::vector<debug_module>::iterator it;
   for(it=debug_modules->begin();it!=debug_modules->end();it++)
   {
      if(name==NULL||name==it->name)
      {
         *it->level=level;
      }
   }
   pthread_mutex_unlock(&lock);
}

void DBG_set_mono(const char* name,bool mono)
{
   pthread_mutex_lock(&lock);
   std::vector<debug_module>::iterator it;
   for(it=debug_modules->begin();it!=debug_modules->end();it++)
   {
      if(name==NULL||name==it->name)
      {
         *it->mono=mono;
      }
   }
   pthread_mutex_unlock(&lock);
}

void DBG_set_blocks(const char* name,bool blocks)
{
   pthread_mutex_lock(&lock);
   std::vector<debug_module>::iterator it;
   for(it=debug_modules->begin();it!=debug_modules->end();it++)
   {
      if(name==NULL||name==it->name)
      {
         *it->traceblocks=blocks;
      }
   }
   pthread_mutex_unlock(&lock);
}
