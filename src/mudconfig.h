#ifndef __MUDCONFIG_H__
#define __MUDCONFIG_H__
/********************************************************************
* File: mudconfig.h                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "location.h"

/*******************************************************************/

#define MAX_CFG_STRLEN  255

#define DFLT_NAME      "Shadow-99"
#define DFLT_LIB       "lib"
#define DFLT_PORT      1111
#define DFLT_MAX_USERS 16
#define DFLT_AINDEX    "world/index"

#define DFLT_ROOM_MSTART "start@temple@stonehill"  /* mortals start room */
#define DFLT_ROOM_WSTART "start@temple@stonehill"  /* wizards start room */
#define DFLT_ROOM_SANC   "start@temple@stonehill"  /* the sanctuary room,
													  for recalls */

#define FILE_WELLCOME  "text/wellcome.txt"   /* the connect greeting */
#define FILE_WMOTD     "text/wmotd.txt"      /* wizards MOTD */
#define FILE_MOTD      "text/motd.txt"       /* normal MOTD */
#define FILE_RULES     "text/rules.txt"      /*the rules for this MUD */
#define FILE_BGINFO    "text/background.txt" /* some background info abouth
												this MUD */

/* BEATS_PER_SEC should never be set below 4!  If you think the MUD takes
 * to much processor time, I recommend you lower it to 5.
 * Also remember to change all other BEATS_PER_* constants if you change it.
 */
#define BEATS_PER_SEC        10  /* the number of ticks per second */
#define BEATS_PER_TICK      600  /* 600 heartbeats per tick,
									i.e. once every minute */
#define BEATS_PER_ROUND      20  /* fighting rounds are two seconds long */
#define BEATS_PER_MONSTER    50  /* monster activity every 5 second */
#define BEATS_PER_AUTOSAVE 6000  /* autosave every 10 minutes */

/*******************************************************************/

struct cfgdata
{
	/* these fields are the same as in the config file */
	char mudname[MAX_CFG_STRLEN + 1];
	char libdir[MAX_CFG_STRLEN + 1];
	char admail[MAX_CFG_STRLEN + 1];
	char admname[MAX_CFG_STRLEN + 1];
	int  masterport;
	int  maxusers;
	char room_mstart[MAX_CFG_STRLEN + 1];
	char room_wstart[MAX_CFG_STRLEN + 1];
	char room_sanc[MAX_CFG_STRLEN + 1];
	char areaindex[MAX_CFG_STRLEN + 1];

	/* the following fields does not exist in the config file */
	char *wellcome;  /* the wellcome text when users connect */
	char *wmotd;     /* MOTD for wizards */
	char *motd;      /* the normal MOTD */
	char *rules;     /* rules for new players to read */
	char *bginfo;    /* background information about this MUD */
	char *menu;      /* the MUD login menu */

	Location *mstart; /* mortal start position */
	Location *wstart; /* wizard start position */
	Location *sanc;   /* sanctuary location */
};

extern struct cfgdata mudconfig;

bool cfg_setdef(void);
bool cfg_readcfg(int ac, char *av[]);
bool cfg_parseargs(int ac, char *av[]);
bool cfg_checkenv(void);

/*******************************************************************/
#endif /* __MUDCONFIG_H__ */
