#ifndef __DBGOUT_H__
#define __DBGOUT_H__

#ifndef DEBUG

#define ASSERT(...)
#define DBG_CRASH
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


#define DBG_CRASH               (*(int*)NULL = 0x69)
#define ASSERT(x) \
    if (!(x)) \
    { \
        DBG_ERR(FMT_HIGH(" Assertion '" #x "' failed ") " %s:%u: %s", \
                __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        DBG_CRASH; \
    }

#define DBG_WTF 	_dbg_wtf
#define DBG_ERR 	_dbg_err
#define DBG_WARN	_dbg_warn
#define DBG_INFO    _dbg_info
#define DBG_INFO_EXT _dbg_info_ext

#define DBG_BLOCK           _dbg_info_block DBG_MAKE_UNIQUE(__dbg_block_)
#define DBG_BLOCK_FUNC()    DBG_BLOCK(__PRETTY_FUNCTION__)
#define DBG_BLOCK_METHOD()  DBG_BLOCK("%s %p", __PRETTY_FUNCTION__, this)

/*run-time context for module*/
static int __dbg__level=0;
static bool __dbg__traceblocks=false;
static bool __dbg__mono=true;

/*run-time context for thread*/
int __dbg__threadid_last __attribute__((weak)) = 0;
__thread int __dbg__threadid = -1;
__thread int __dbg__blocklvl __attribute__((weak)) = 0;

#ifdef DBG_MODULE_NAME
static const char* __dbg__debug_env=DBG_MODULE_NAME"_DEBUG";
#else
static const char* __dbg__debug_env="DEBUG";
#endif

static void __dbg__read_env() __attribute__((constructor));
static void __dbg__read_env()
{
   char* p=getenv(__dbg__debug_env);
   if(p!=NULL)
   {
      while(*p!=0)
      {
         if(*p>='0' && *p<='5') __dbg__level=*p-'0';
         else if(*p == 't') __dbg__traceblocks=true;
         else if(*p == 'c') __dbg__mono=false;
         else
         {
            fprintf(stderr,"Unknown control char '%c' in $%s\n",*p,__dbg__debug_env);
         }
         p++;
      }
   }
}

class _dbg_info_block
{
public:
    _dbg_info_block(const char *fmt, ...);
    ~_dbg_info_block();
private:
    void print(const char *pr);
    char *str;
};

_dbg_info_block::_dbg_info_block(const char *fmt, ...) : str(NULL)
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

_dbg_info_block::~_dbg_info_block()
{
   if(str!=NULL)
   {
      __dbg__blocklvl--;
      print("--");
      free(str);
   }
}

void _dbg_info_block::print(const char *pr)
{
   if(__dbg__mono)
      fprintf(stderr, " BLCK:  %2d: %*s %s\n",
              __dbg__threadid, __dbg__blocklvl*3+2, pr, str);
   else
      fprintf(stderr, FMT_COLOR(C_BLACK, C_YELLOW) " BLCK: " FONT_RESET " "
              FONT_COLOR_HI("%c") BG_COLOR(C_BLACK) "%2d: %*s %s" FONT_RESET "\n",
              '2'+(__dbg__threadid%5),__dbg__threadid, __dbg__blocklvl*3+2, pr, str);
}

static void _dbg_err(const char *fmt, ...);

static void dbg_print(int lvl, const char *fmt, va_list args)
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
		strcpy(buff, p);
        if(__dbg__mono)
        {
           strcat(buff, "%2d:%*c%s \n");
           vasprintf(&str, fmt, args);
           fprintf(stderr, buff, __dbg__threadid, __dbg__blocklvl*3+1, ' ', str);
        }        
        else
        {
           strcat(buff, FONT_COLOR_HI("%c") BG_COLOR(C_BLACK) "%2d:%*c%s " FONT_RESET "\n");
           vasprintf(&str, fmt, args);
           fprintf(stderr, buff, '2'+(__dbg__threadid%5), __dbg__threadid, __dbg__blocklvl*3+1, ' ', str);
        }
        free(str);
	}
}

static inline void _dbg_info_ext(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    dbg_print(5, fmt, args);
	va_end(args);
}

static inline void _dbg_info(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(4, fmt, args);
	va_end(args);
}

static inline void _dbg_warn(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(3, fmt, args);
	va_end(args);
}

static inline void _dbg_err(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(2, fmt, args);
	va_end(args);
}

static inline void _dbg_wtf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(1, fmt, args);
	va_end(args);
}

#endif // DEBUG
#endif // __DBGOUT_H__
