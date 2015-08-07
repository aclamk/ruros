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
      _dbg_info_ext("about go invoke recursion with %d",x);
      _dbg_info("recursion continues");   
      recursion(x);
   }
   else
      _dbg_info("recursion reached end");   
}

void* func(void* ignored=0)
{
   _dbg_wtf("IMPOSSIBLE HAPPENED %d=%d",3,2);
   _dbg_err("not enough free mem=%d bytes",7);
   _dbg_warn("program went to sleep for %d seconds",1000000);
   _dbg_info("thread execution continues %s %s %s","happily","ever","after");   
   _dbg_info_ext("just calculated '+' for arguments %d and %d",42,69);

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
   func();
   sleep(1);
}
