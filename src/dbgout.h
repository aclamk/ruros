#ifndef __DBGOUT_H__
#define __DBGOUT_H__
/* Colorful and simple logging utility.
   Original version Michal Szymaniak.
   Modified for single header file Adam Kupczyk */
#ifndef DEBUG

#define DBG_ASSERT(...)
#define DBG_CRASH()
#define DBG_WTF(...)
#define DBG_ERR(...)
#define DBG_WARN(...)
#define DBG_INFO(...)
#define DBG_INFO_EXT(...)

#define DBG_BLOCK(...)
#define DBG_BLOCK_FUNC()
#define DBG_BLOCK_METHOD()



#else

#include <typeinfo>
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


#define _DBG_CONCATENATE(x, y)  x##y
#define DBG_CONCATENATE(x, y)   _DBG_CONCATENATE(x, y)
#define DBG_MAKE_UNIQUE(x)      DBG_CONCATENATE(x, __LINE__)

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


#define DBG_CRASH() (*(int*)NULL = 0x69)
#define DBG_ASSERT(x) do {                      \
    if (!(x)) { \
        DBG_ERR(FMT_HIGH(" Assertion '" #x "' failed ") " %s:%u: %s", \
                __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        DBG_CRASH();                                      \
    } } while(false)

#define DBG_WTF 	__dbg__wtf
#define DBG_ERR 	__dbg__err
#define DBG_WARN	__dbg__warn
#define DBG_INFO    __dbg__info
#define DBG_INFO_EXT __dbg__info_ext

#define DBG_BLOCK           __dbg__info_block DBG_MAKE_UNIQUE(__dbg__block_)
#define DBG_BLOCK_FUNC()    DBG_BLOCK(__PRETTY_FUNCTION__)
#define DBG_BLOCK_METHOD()  DBG_BLOCK("%s %p", __PRETTY_FUNCTION__, this)

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
         sprintf(strtime,"%2.2d:%2.2d:%2.2d.%6ld ",time.tm_hour,time.tm_min,time.tm_sec,tv.tv_usec);
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
static void __dbg__err(const char *fmt, ...);

static void __dbg__print(int lvl, const char *fmt, va_list args)
{
	if (lvl <= __dbg__level)
	{
       if (__dbg__threadid == -1)
          __dbg__threadid = __dbg__threadid_last++;

		const char *p = NULL;
        if(__dbg__mono)
           switch (lvl)
           {
              case 1:
                 p = "  WTF:  ";
                 break;
              case 2:
                 p = "  ERR:  ";
                 break;
              case 3:
                 p = " WARN:  ";
                 break;
              case 4:
              case 5:
                 p = " INFO:  ";
                 break;

              default:
                 p = "  ???:  ";
                 break;
           }
        else
           switch (lvl)
           {
              case 1:
                 p = FONT_COLOR(C_BLACK) BG_COLOR_HI(C_PURPLE) "  WTF: "
                 FONT_RESET FONT_COLOR(C_BLACK) BG_COLOR_HI(C_RED) " ";
                 break;

              case 2:
                 p = FMT_COLOR(C_WHITE, C_RED) FONT_BOLD "  ERR: "
                 FONT_RESET " ";
                 break;

              case 3:
                 p = FMT_COLOR(C_WHITE, C_BLUE) FONT_BOLD " WARN: "
                 FONT_RESET " ";
                 break;

              case 4:
              case 5:
                 p = FONT_COLOR(C_BLACK) BG_COLOR_HI(C_YELLOW) " INFO: "
                 FONT_RESET " ";
                 break;

              default:
                 p = "  ???:  ";
                 break;
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

static inline void __dbg__info_ext(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    __dbg__print(5, fmt, args);
	va_end(args);
}

static inline void __dbg__info(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__dbg__print(4, fmt, args);
	va_end(args);
}

static inline void __dbg__warn(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__dbg__print(3, fmt, args);
	va_end(args);
}

static inline void __dbg__err(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__dbg__print(2, fmt, args);
	va_end(args);
}

static inline void __dbg__wtf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__dbg__print(1, fmt, args);
	va_end(args);
}

#endif // DEBUG
#endif // __DBGOUT_H__
