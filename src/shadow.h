#ifndef __SHADOW_H__
#define __SHADOW_H__
/********************************************************************
* File: shadow.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>

/* this must be the last 'system' include file */
#include <dmalloc.h>

#include "mudconfig.h"
#include "string.h"
#include "files.h"
#include "server.h"

/*******************************************************************/

#ifndef PATH_MAX
#  ifdef _POSIX_PATH_MAX
#    define PATH_MAX _POSIX_PATH_MAX
#  else
#    define PATH_MAX 255
#  endif
#endif

#if defined(HAVE_LRAND48)
#  define RANDOM lrand48
#elif defined(HAVE_RANDOM)
#  define RANDOM random
#else
#  define RANDOM rand
#endif

typedef enum log_e
{
	LOG_SYSERR = 0, /* a coding error, not sent to wizards */
	LOG_INFO,       /* general information */
	LOG_LOGIN,      /* user login/logout information */
} log_t;

#ifdef __GNUC__
void log(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void syslog(const log_t type, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
#else
void log(const char *fmt, ...);
void syslog(const log_t type, const char *fmt, ...)
#endif

#define out_of_memory() \
do { \
	log("Out of memory (%s:%d)\n", __FILE__, __LINE__); \
	abort(); \
} while (0)

/* This macro is taken from CircleMUD 3.0 patchlevel 14 */
#define REMOVE_FROM_LIST(item, head, next, type) \
    do {                                    \
		type *temp;                         \
		if ((item) == (head))               \
			head = (item)->next;            \
		else {                              \
			temp = (type *) head;           \
			while (temp && (temp->next != (item))) \
				temp = (type *) temp->next; \
			if (temp)                       \
				temp->next = (item)->next;  \
		}                                   \
	} while (0)

#define NULLSTR(str)  ((str) == NULL ? "" : (str))

/*******************************************************************/

#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

static inline int min(int a, int b) { return a < b ? a : b; }
static inline int max(int a, int b) { return a > b ? a : b; }

int number(int from, int to);
int dice(register int num, register int size);

/*******************************************************************/
#endif /* __SHADOW_H__ */
