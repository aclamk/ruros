#define DBG_MODULE_NAME "TEST"
#include "dbgout.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <pthread.h>


void recursion(int x)
{   
   DBG_BLOCK("%d levels to go",x);
   usleep((random()%300)*1000);
   if(x>0)
   {
      x--;
      DBG(DUMP,"about go invoke recursion with %d",x);
      DBG(INFO,"recursion continues");   
      recursion(x);
   }
   else
      DBG(INFO,"recursion reached end");   
}

void* func(void* ignored=0)
{
   DBG(WTF,"IMPOSSIBLE HAPPENED %d=%d",3,2);
   DBG(ERR,"not enough free mem=%d bytes",7);
   DBG(WARN,"program went to sleep for %d seconds",1000000);
   DBG(INFO,"thread execution continues %s %s %s","happily","ever","after");   
   DBG(DUMP,"just calculated '+' for arguments %d and %d",42,69);

   recursion(7);
   return NULL;
}



int main(int argc, char** argv)
{
   pthread_create(new pthread_t, NULL,func, NULL);
   pthread_create(new pthread_t, NULL,func, NULL);
   pthread_create(new pthread_t, NULL,func, NULL);
   pthread_create(new pthread_t, NULL,func, NULL);
   pthread_create(new pthread_t, NULL,func, NULL);
   sleep(1);
   DBG_set_mono("TEST",true);
   func();
   sleep(1);
}
//extern "C" void __dbg__register_module(char* name, int* level, bool* traceblocks, bool* mono);
