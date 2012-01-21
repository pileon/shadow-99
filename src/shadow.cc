/********************************************************************
* File: shadow.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "user.h"
#include "player.h"

#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

/*******************************************************************/

int main(int argc, char *argv[])
{
	if (server.init(argc, argv))
		server.run();

	return 0;
}

/*******************************************************************/

static void log_time(void)
{
	/* NOTE:  this function isn't thread safe! */
	time_t     now1 = time(NULL);
	struct tm *now2 = localtime(&now1);
	char      *now3 = asctime(now2);

	now3[strlen(now3) - 1] = 0;  /* remove the newline */

	fprintf(stderr, "%s :: ", now3);
}

void log(const char *fmt, ...)
{
	va_list args;

	log_time();

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void syslog(const log_t type, const char *fmt, ...)
{
	va_list args;
	char    buf[512];
	User   *u;

	static const char *msgs[] = {
		"SYSERR",
		"INFO",
		"LOGIN",
	};

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	log("[%s] %s\n", msgs[type], buf);

	if (type != LOG_SYSERR)
	{
		for (u = server.m_users; u; u = u->m_next)
		{
			if (u->get_state() == User::S_PLAYING)
			{
				if (u->m_player && u->m_player->is_wizard())
					u->write("[%s] %s\r\n", msgs[type], buf);
			}
		}
	}
}

/*******************************************************************/
