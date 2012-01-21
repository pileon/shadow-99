#ifndef __INTERP_H__
#define __INTERP_H__
/********************************************************************
* File: interp.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "character.h"

/*******************************************************************/

typedef void (cmdfun_t)(Character *, int, char **, int, int);

#define ACMD(name) \
    void name(Character *ch, int argc, char **argv, int cmd, int scmd)

struct cmdentry
{
	char     *verb;    /* verb string */
	int       level;   /* min player/wizard level */
	bool      wiz;     /* true if wizard only */
	cmdfun_t *handler; /* normal command handler */
	int       scmd;
};

extern struct cmdentry cmdtable[];

/*******************************************************************/

/* do_look */
#define SCMD_LOOK  0
#define SCMD_EXAM  1
#define SCMD_READ  2

/* do_quit */
#define SCMD_QUIT  1

/* do_comm */
#define SCMD_ASK     0
#define SCMD_WHISPER 1

/* do_shutdown */
#define SCMD_SHUTDOWN 1

/*******************************************************************/
#endif /* __INTERP_H__ */
