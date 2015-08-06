#include "dbgout.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef DEBUG

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 3
#endif

#if (DEBUG_LEVEL > 5)
#error DEBUG_LEVEL must be in range from 0 (no debugs) to 5 (all debugs)
#endif

static void dbg_print(int lvl, const char *fmt, va_list args)
{
	if (lvl <= DEBUG_LEVEL)
	{
		char *p = NULL;
		switch (lvl)
		{
			case 1:
				p = (char*) FONT_COLOR(C_BLACK) BG_COLOR_HI(C_PURPLE) "  WTF: "
                    FONT_RESET FONT_COLOR(C_BLACK) BG_COLOR_HI(C_RED) " ";
				break;

			case 2:
				p = (char*) FMT_COLOR(C_WHITE, C_RED) FONT_BOLD "  ERR: "
                    FONT_RESET " ";
				break;

			case 3:
				p = (char*) FMT_COLOR(C_WHITE, C_BLUE) FONT_BOLD " WARN: "
                    FONT_RESET " ";
				break;

			case 4:
			case 5:
				p = (char*) FONT_COLOR(C_BLACK) BG_COLOR_HI(C_YELLOW) " INFO: "
                    FONT_RESET " ";
				break;

			default:
				p = (char*)"  ???: ";
				break;
		}

		size_t len = strlen(fmt) + 64;
		char *buff = new char[len];

		strcpy(buff, p);
		strcat(buff, fmt);
		strcat(buff, " " FONT_RESET "\n");
		vfprintf(stderr, buff, args);

		ASSERT(sizeof(buff) < len);
		delete [] buff;
	}
}

void _dbg_info_ext(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(5, fmt, args);
	va_end(args);
}

void _dbg_info(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(4, fmt, args);
	va_end(args);
}

void _dbg_warn(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(3, fmt, args);
	va_end(args);
}

void _dbg_err(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(2, fmt, args);
	va_end(args);
}

void _dbg_wtf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	dbg_print(1, fmt, args);
	va_end(args);
}

static int next_threadid = 0;
static __thread int threadid = -1;
static __thread int blocklvl = 0;

_dbg_info_block::_dbg_info_block(const char *fmt, ...) : str(NULL)
{
    if (threadid == -1)
        threadid = next_threadid++;

	va_list args;
	va_start(args, fmt);
    vasprintf(&str, fmt, args);
	va_end(args);
    print("++");
    blocklvl++;
}

_dbg_info_block::~_dbg_info_block()
{
    blocklvl--;
    print("--");
    free(str);
}

void _dbg_info_block::print(const char *pr)
{
    fprintf(stderr, FMT_COLOR(C_BLACK, C_YELLOW) " BLCK: " FONT_RESET " "
           FONT_COLOR_HI("%c") BG_COLOR(C_BLACK) "%2d: %*s %s" FONT_RESET "\n",
           '2'+(threadid%5), threadid, blocklvl*4+2, pr, str);
}

#endif // DEBUG
