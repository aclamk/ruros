#ifndef __DBGOUT_H__
#define __DBGOUT_H__
/* Colorful and simple logging utility.
   Original version Michal Szymaniak.
   Modified for single header file Adam Kupczyk */
#ifndef DEBUG

static inline bool DBG(int lvl,const char *fmt, ...) {return false;};
static inline bool DBG(const char *fmt, ...) {return false;};
static inline bool DBG(int lvl) {return false;};

#define DBG_BLOCK(...)
#define DBG_BLOCK_FUNC()
#define DBG_BLOCK_METHOD()

#else

//#include <typeinfo>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define DBG_COMPILED_LEVEL 5

#define _DBG_CONCATENATE(x, y)  x##y
#define DBG_CONCATENATE(x, y)   _DBG_CONCATENATE(x, y)

#define __FONT_FMT(f)		"\e[" f "m"

#define FONT_COLOR(c)		__FONT_FMT("3" c)
#define FONT_COLOR_HI(c)	__FONT_FMT("9" c)
#define BG_COLOR(c)			__FONT_FMT("4" c)
#define BG_COLOR_HI(c)		__FONT_FMT("10" c)
#define FMT_COLOR(f, b)		FONT_COLOR(f) BG_COLOR(b)

#define FONT_RESET		__FONT_FMT("0")
#define FONT_BOLD		__FONT_FMT("1")
#define FONT_UNDERLINE	__FONT_FMT("4")

#define C_BLACK			"0"
#define C_RED			"1"
#define C_GREEN			"2"
#define C_YELLOW		"3"
#define C_BLUE			"4"
#define C_PURPLE		"5"
#define C_CYAN			"6"
#define C_WHITE			"7"

#define FMT_HIGH(s)		FONT_COLOR(C_BLACK) BG_COLOR_HI(C_RED) s FONT_RESET
#define FMT_FUNC(s)		FONT_COLOR(C_BLACK) BG_COLOR_HI(C_CYAN) "[" s "]" FONT_RESET
#define FMT_MODULE(s)	FONT_COLOR(C_BLACK) BG_COLOR_HI(C_GREEN) "[" s "]" FONT_RESET

enum
{
   NONE=-1, //used to disable ALL logs 
   WTF=0,   //terrible failure of fundamental behaviour, including internal flow assertions.
   ERR=1,   //failure of system processing, lack of necessary resources for which there is no redundancy handling
   FAIL=2,  //detraction from successfull processing but action is still properly handled
   WARN=3,  //unexpected or abnormal behaviour, including input. processed properly
   INFO=4,  //logging of processing and decision paths
   DUMP=5   //dumping data that is being processed
};

/*run-time context for all modules*/
bool __dbg__withtime __attribute__((weak)) = false;
bool __dbg__withtime_precise __attribute__((weak)) = false;

/*run-time context for this module*/
static int __dbg__level=0;
static bool __dbg__traceblocks=false;
static bool __dbg__mono=true;

/*run-time context for thread*/
int __dbg__threadid_last __attribute__((weak)) = 0;
__thread int __dbg__threadid __attribute__((weak)) = -1;
__thread int __dbg__blocklvl __attribute__((weak)) = 0;



static void __dbg__print(int lvl, const char *fmt, va_list args); //unconditional print, internal.
static inline bool DBG(int lvl,const char *fmt, ...) 
{
   if(lvl<=DBG_COMPILED_LEVEL && lvl<= __dbg__level)
   {
      va_list args;
      va_start(args, fmt);
      __dbg__print(lvl, fmt, args);
      va_end(args);
      return true;
   }
   else
      return false;
}

static inline bool DBG(const char *fmt, ...) 
{
   int lvl=INFO;
   if(lvl<=DBG_COMPILED_LEVEL && lvl<= __dbg__level)
   {
      va_list args;
      va_start(args, fmt);
      __dbg__print(lvl, fmt, args);
      va_end(args);
      return true;
   }
   else
      return false;
}

static inline bool DBG(int lvl)
{
   return lvl<=DBG_COMPILED_LEVEL && lvl<= __dbg__level;
}

#define DBG_CRASH() (*(int*)NULL = 0x69)
#define DBG_ASSERT(x) do {                      \
    if (!(x)) { \
        DBG(ERR,FMT_HIGH(" Assertion '" #x "' failed ") " %s:%u: %s",   \
                __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        DBG_CRASH();                                      \
    } } while(false)



#define DBG_BLOCK(...)      __dbg__info_block DBG_CONCATENATE(__dbg__block_,__LINE__) (__VA_ARGS__)
#define DBG_BLOCK_FUNC()    __dbg__info_block DBG_CONCATENATE(__dbg__block_,__LINE__) (__PRETTY_FUNCTION__)
#define DBG_BLOCK_METHOD()  __dbg__info_block DBG_CONCATENATE(__dbg__block_,__LINE__) ("%s %p", __PRETTY_FUNCTION__, this)

void __dbg__register_module(const char* name, int* level, bool* traceblocks, bool* mono) __attribute__ ((weak));
void DBG_set_level(const char* name,int level);
void DBG_set_mono(const char* name,bool mono);
void DBG_set_blocks(const char* name,bool blocks);



static void __dbg__read_env() __attribute__((constructor));
static void __dbg__read_env()
{
#ifdef DBG_MODULE_NAME
   const char* varname[2]={"DEBUG","DEBUG_" DBG_MODULE_NAME};
#else
   const char* varname[1]={"DEBUG"};
#endif
   for(size_t i=0;i<sizeof(varname)/sizeof(*varname);i++)
   {
      char* p=getenv(varname[i]);
      if(p!=NULL)
      {
         while(*p!=0)
         {
            if(*p>='0' && *p<='5') __dbg__level=*p-'0';
            else if(*p == 'b') __dbg__traceblocks=true;
            else if(*p == 'c') __dbg__mono=false;
            else if(*p == 't') __dbg__withtime=true;
            else if(*p == 'T') __dbg__withtime=__dbg__withtime_precise=true;
            else
            {
               fprintf(stderr,"Unknown control char '%c' in $%s\n",*p,varname[i]);
            }
            p++;
         }
      }
   }
   if(__dbg__register_module != NULL)
      __dbg__register_module(DBG_MODULE_NAME, &__dbg__level, &__dbg__traceblocks, &__dbg__mono);
}

static void inline __dbg__formattimenow(char strtime[sizeof("01:02:03.456789  ")])
{
   struct timeval tv;
   struct tm time;
   if(__dbg__withtime)
   {
      gettimeofday(&tv,NULL);
      localtime_r(&tv.tv_sec,&time);
      if(__dbg__withtime_precise)
         sprintf(strtime,"%2.2d:%2.2d:%2.2d.%6.6ld ",time.tm_hour,time.tm_min,time.tm_sec,tv.tv_usec);
      else
         sprintf(strtime,"%2.2d:%2.2d:%2.2d ",time.tm_hour,time.tm_min,time.tm_sec);
   }
   else
      strtime[0]='\0';
}

namespace {
class __dbg__info_block
{
public:
    __dbg__info_block(const char *fmt, ...);
    ~__dbg__info_block();
private:
    void print(const char *pr);
    char *str;
};

__dbg__info_block::__dbg__info_block(const char *fmt, ...) : str(NULL)
{
   if(__dbg__traceblocks)
   {
      if (__dbg__threadid == -1)
         __dbg__threadid = __dbg__threadid_last++;
      va_list args;
      va_start(args, fmt);
      vasprintf(&str, fmt, args);
      va_end(args);
      print("++");
      __dbg__blocklvl++;
   }
   else
      str=NULL;
}

__dbg__info_block::~__dbg__info_block()
{
   if(str!=NULL)
   {
      __dbg__blocklvl--;
      print("--");
      free(str);
   }
}

void __dbg__info_block::print(const char *pr)
{
   char strtime[sizeof("01:02:03.456789  ")];
   __dbg__formattimenow(strtime);
   if(__dbg__mono)
      fprintf(stderr, "%sBLCK:  %2d: %*s %s\n",
              strtime,__dbg__threadid, __dbg__blocklvl*3+2, pr, str);
   else
      fprintf(stderr, "%s" FMT_COLOR(C_BLACK, C_YELLOW) " BLCK: " FONT_RESET " "
              FONT_COLOR_HI("%c") BG_COLOR(C_BLACK) "%2d: %*s %s" FONT_RESET "\n",
              strtime,'2'+(__dbg__threadid%5),__dbg__threadid, __dbg__blocklvl*3+2, pr, str);
}
}
static void __dbg__print(int lvl, const char *fmt, va_list args)
{
	if (WTF<=lvl && lvl <= DUMP)
	{
       if (__dbg__threadid == -1)
          __dbg__threadid = __dbg__threadid_last++;

		const char *p = NULL;
        if(__dbg__mono)
        {
           static const char* headers[DUMP-WTF+1]=
              {
                 "  WTF:  ",
                 "  ERR:  ",
                 " FAIL:  ",
                 " WARN:  ",
                 " INFO:  ",
                 " DUMP:  "
              };
           p=headers[lvl];
        }
        else
        {
           static const char* headers[DUMP-WTF+1]=
              {
                 FONT_COLOR(C_BLACK) BG_COLOR_HI(C_PURPLE) "  WTF: "
                 FONT_RESET FONT_COLOR(C_BLACK) BG_COLOR_HI(C_RED) " ",

                 FMT_COLOR(C_WHITE, C_RED) FONT_BOLD "  ERR: " FONT_RESET " ",
                 
                 FMT_COLOR(C_WHITE, C_RED) FONT_BOLD " FAIL: " FONT_RESET " ",

                 FMT_COLOR(C_WHITE, C_BLUE) FONT_BOLD " WARN: " FONT_RESET " ",

                 FONT_COLOR(C_BLACK) BG_COLOR_HI(C_YELLOW) " INFO: " FONT_RESET " ",
                 FONT_COLOR(C_BLACK) BG_COLOR_HI(C_YELLOW) " DUMP: " FONT_RESET " "
              };
           p=headers[lvl];
        }
		size_t len = 100;
		char buff[len];
        char* str=NULL;

        char strtime[sizeof("01:02:03.456789  ")];
        __dbg__formattimenow(strtime);

        strcpy(buff, "%s");
		strcpy(buff+2, p);
        
        if(__dbg__mono)
        {
           strcat(buff, "%2d:%*c%s \n");
           if(vasprintf(&str, fmt, args))
              fprintf(stderr, buff, strtime, __dbg__threadid, __dbg__blocklvl*3+1, ' ', str);
        }        
        else
        {
           strcat(buff, FONT_COLOR_HI("%c") BG_COLOR(C_BLACK) "%2d:%*c%s " FONT_RESET "\n");
           if(vasprintf(&str, fmt, args))
              fprintf(stderr, buff, strtime, '2'+(__dbg__threadid%5), __dbg__threadid, __dbg__blocklvl*3+1, ' ', str);
        }
        free(str);
	}
}

#endif // DEBUG
#endif // __DBGOUT_H__
