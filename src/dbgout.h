#ifndef __DBGOUT_H__
#define __DBGOUT_H__

#ifdef DEBUG

#include <typeinfo>
#include <assert.h>
#include <unistd.h>

#define _DBG_CONCATENATE(x, y)  x##y
#define DBG_CONCATENATE(x, y)   _DBG_CONCATENATE(x, y)
#define DBG_MAKE_UNIQUE(x)      DBG_CONCATENATE(x, __LINE__)

#ifdef DEBUG_BW
#define __FONT_FMT(f)
#else
#define __FONT_FMT(f)		"\e[" f "m"
#endif

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

void _dbg_info_ext(const char *fmt, ...);
void _dbg_info(const char *fmt, ...);
void _dbg_warn(const char *fmt, ...);
void _dbg_err(const char *fmt, ...);
void _dbg_wtf(const char *fmt, ...);

class _dbg_info_block
{
public:
    _dbg_info_block(const char *fmt, ...);
    ~_dbg_info_block();
private:
    void print(const char *pr);
    char *str;
};

#endif // DEBUG


#if defined (DEBUG)
#define DBG_CRASH               (*(int*)NULL = 0x69)
//#define ASSERT(...)             assert(__VA_ARGS__)
#define ASSERT(x) \
    if (!(x)) \
    { \
        DBG_ERR(FMT_HIGH(" Assertion '" #x "' failed ") " %s:%u: %s", \
                __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        DBG_CRASH; \
    }
#define ASSERT_MTX_LOCKED(m)    m._dbg_assert_locked()
#else
#define ASSERT(...)
#define ASSERT_MTX_LOCKED(...)
#define DBG_CRASH
#endif

#if defined(DEBUG) && (DEBUG_LEVEL >= 1)
#define DBG_WTF 	_dbg_wtf
#else
#define DBG_WTF(...)
#endif

#if defined(DEBUG) && (DEBUG_LEVEL >= 2)
#define DBG_ERR 	_dbg_err
#else
#define DBG_ERR(...)
#endif

#if defined(DEBUG) && (DEBUG_LEVEL >= 3)
#define DBG_WARN	_dbg_warn
#else
#define DBG_WARN(...)
#endif

#if defined(DEBUG) && (DEBUG_LEVEL >= 4)
#define DBG_INFO            _dbg_info
#define DBG_BLOCK           _dbg_info_block DBG_MAKE_UNIQUE(__dbg_block_)
#define DBG_BLOCK_FUNC()    DBG_BLOCK(__PRETTY_FUNCTION__)
#define DBG_BLOCK_METHOD()  DBG_BLOCK("%s %p", __PRETTY_FUNCTION__, this)
#else
#define DBG_INFO(...)
#define DBG_BLOCK(...)
#define DBG_BLOCK_FUNC()
#define DBG_BLOCK_METHOD()
#endif

#if defined(DEBUG) && (DEBUG_LEVEL >= 5)
#define DBG_INFO_EXT	_dbg_info_ext
#else
#define DBG_INFO_EXT(...)
#endif

#endif

#define UNUSED(x) (void)x
