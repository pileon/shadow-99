/********************************************************************
* File: interpreter.cc                            Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "player.h"
#include "interp.h"
#include "location.h"

/*******************************************************************/

static bool split_command(const char *cmdline, const int cmdlen,
						  char ***argv, int *argc);
static bool call_handlers(Character *ch, const int cmd, int argc, char **argv,
						  const bool before);

/*******************************************************************/

ACMD(do_quit)
{
	if (scmd != SCMD_QUIT)
	{
		ch->write("If you want to quit, then say so.\r\n");
		return;
	}

	if (!ch->is_player())
	{
		ch->write("But only players can quit, not monsters.\r\n");
		return;
	}

	Player *plr = (Player *) ch;
	plr->save();
	plr->m_user->set_state(User::S_MENU);
	plr->write(mudconfig.menu);
}

ACMD(do_shutdown)
{
	if (ch->is_monster())
	{
		ch->write("Like a monster like you would be allowed such things!\r\n");
		return;
	}

	if (scmd != SCMD_SHUTDOWN)
	{
		ch->write("If you want to shutdown the mud, say so!\r\n");
		return;
	}

	if (ch->is_archwiz() && ch->m_level >= 99)
	{
		log("Shutdown by %s\n", ch->get_name());
		write_all("%s is going down.\r\n", mudconfig.mudname);
		server.shutdown();
	}
}

/* prototypes for all commands in the MUD */
ACMD(do_look);
ACMD(do_move);
ACMD(do_score);
ACMD(do_who);
ACMD(do_info);
ACMD(do_say);
ACMD(do_tell);
ACMD(do_shout);
ACMD(do_yell);
ACMD(do_comm);
ACMD(do_lock);
ACMD(do_unlock);
ACMD(do_open);
ACMD(do_close);
ACMD(do_get);
ACMD(do_drop);
ACMD(do_give);
ACMD(do_rusage);
ACMD(do_goto);
ACMD(do_count);
ACMD(do_commands);
ACMD(do_toggle);

struct cmdentry cmdtable[] = {

	/* these commands should be placed first because:
	 * 1. they are used pretty often, so access is faster
	 * 2. so their abbreviations take precedence
	 */
	{ "north"   , 1 , false, do_move, DIR_NORTH },
	{ "east"    , 1 , false, do_move, DIR_EAST  },
	{ "south"   , 1 , false, do_move, DIR_SOUTH },
	{ "west"    , 1 , false, do_move, DIR_WEST  },
	{ "up"      , 1 , false, do_move, DIR_UP    },
	{ "down"    , 1 , false, do_move, DIR_DOWN  },

	/* verb     level wizonly func      subcommand */
	{ "who"     , 1 , false, do_who     , 0 },
	{ "qui"     , 1 , false, do_quit    , 0 },
	{ "quit"    , 1 , false, do_quit    , SCMD_QUIT },
	{ "score"   , 1 , false, do_score   , 0 },
	{ "shutdow" , 99, true , do_shutdown, 0 },
	{ "shutdown", 99, true , do_shutdown, SCMD_SHUTDOWN },
	{ "info"    , 1 , false, do_info    , 0 },
	{ "say"     , 1 , false, do_say     , 0 },
	{ "tell"    , 1 , false, do_tell    , 0 },
	{ "shout"   , 1 , false, do_shout   , 0 },
	{ "yell"    , 1 , false, do_yell    , 0 },
	{ "whisper" , 1 , false, do_comm    , SCMD_WHISPER },
	{ "ask"     , 1 , false, do_comm    , SCMD_ASK },
	{ "look"    , 1 , false, do_look    , SCMD_LOOK },
	{ "examine" , 1 , false, do_look    , SCMD_EXAM },
	{ "read"    , 1 , false, do_look    , SCMD_READ },
	{ "lock"    , 1 , false, do_lock    , 0 },
	{ "unlock"  , 1 , false, do_unlock  , 0 },
	{ "open"    , 1 , false, do_open    , 0 },
	{ "close"   , 1 , false, do_close   , 0 },
	{ "get"     , 1 , false, do_get     , 0 },
	{ "drop"    , 1 , false, do_drop    , 0 },
	{ "give"    , 1 , false, do_give    , 0 },
	/*{ "rusage"  , 50, true , do_rusage  , 0 },*/
	{ "goto"    , 2 , true , do_goto    , 0 },
	{ "count"   , 1 , true , do_count   , 0 },
	{ "commands", 1 , false, do_commands, 0 },
	{ "toggle"  , 1 , false, do_toggle  , 0 },

	/* end-of-table marker, this entry must allways be last */
	{ NULL, 0, false, NULL, 0 }
};

/*******************************************************************/

void Player::interpret(const char *cmdline, const int linelen)
{
	if (!m_parent)
	{
		syslog(LOG_SYSERR, "Player with no parent in interpreter (%s)",
			   get_name());
		return;
	}

	/*
	 * All command handlers have the following prototype:
	 *
	 * void cmdhandler(Player *plr, int argc, char **argv, int cmdidx);
	 *
	 * Player *plr     : The player issuing the command
	 * int argc        : Size of the argv array
	 * char **argv     : Array of arguments, argv[0] is the command
	 * int cmdidx      : Index into the global command table.
	 *
	 * The ACMD macro provides such a prototype.
	 */

	char **argv;   /* argument values/vector */
	int    argc;   /* argument count */
	char  *cmd;    /* points to argv[0] after parsing */
	int    idx;    /* index into command table */
	size_t cmdlen; /* length of the command string */

	argc = 0;
	argv = NULL;

	/* strip all leading and ending spaces */
	strip_spaces((char **) &cmdline);

	/* split the command into arguments */
	if (!split_command(cmdline, linelen, &argv, &argc))
		goto interp_what;  /* something failed */

	cmd    = argv[0];
	cmdlen = strlen(cmd);

	/* find the command table entry */
	for (idx = 0; ; idx++)
	{
		if (cmdtable[idx].verb == NULL)
			break;  /* no more entries after this one */

		if (*cmdtable[idx].verb == *cmd &&
			strlen(cmdtable[idx].verb) >= cmdlen &&
			strncasecmp(cmdtable[idx].verb, cmd, cmdlen) == 0)
		{
			/* found the command */
			break;
		}
	}

	/* make sure the player can use this command */
	if (cmdtable[idx].verb != NULL)
	{
		if (cmdtable[idx].level > m_level)
			goto interp_what;
		if (cmdtable[idx].wiz && !is_wizard())
			goto interp_what;
	}
	else
		goto interp_what;

	/* call all before handlers */
	if (!call_handlers(this, idx, argc, argv, true))
		goto interp_done;

	/* now execute the command */
	if (cmdtable[idx].verb != NULL)
	{
		if (cmdtable[idx].handler != NULL)
			cmdtable[idx].handler(this, argc, argv, idx, cmdtable[idx].scmd);
	}

	/* call all after handlers */
	if (!call_handlers(this, idx, argc, argv, false))
		goto interp_done;

interp_done:
	/* free the command and all it's arguments */
	if (argv)
	{
		while (argc && argc--)
		{
			if (argv[argc])
				delete [] argv[argc];
		}
		delete [] argv;
	}
	return;

interp_what:
	write("Huh?!?\r\n");
	goto interp_done;
}

/*******************************************************************/

/* split a line into an array, one entry for each word
 * note that this function is nontrivial, so don't mess around with it!
 */
static bool split_command(const char *cmdline, const int /*cmdlen*/,
						  char ***argv, int *argc)
{
	/* TODO:  everything inside '"' pairs should be considered one word */

	char *p1, *p2;
	int   l, a;

	/* 1: count the number of arguments */
	*argc = 1;
	p2 = (char *) cmdline;
	while ((p1 = strpbrk(p2, " \t\0")) != NULL)
	{
		*argc = *argc + 1;
		p2 = p1 + 1;
	}

	/* 2: allocate the argument array */
	if ((*argv = new char * [*argc + 1]) == NULL)
		out_of_memory();

	/* 3: copy all arguments to the array */
	a  = 0;
	p2 = (char *) cmdline;
	while ((p1 = strpbrk(p2, " \t\0")) != NULL)
	{
		if (p1 == p2 || !*p1)
			break;

		/* allocate the string */
		l = p1 - p2 + 1;
		if (((*argv)[a] = new char [l + 1]) == NULL)
			out_of_memory();

		/* copy and terminate the string */
		memcpy((*argv)[a], p2, l);
		(*argv)[a][l - 1] = 0;
		a++;

		p2 = p1 + 1;
	}

	/* 4: get the last argument */
	if (p2 != NULL)
	{
		(*argv)[a] = STRDUP(p2);
		a++;
	}
	(*argv)[a] = NULL;  /* terminate the array */

	return true;
}

/*******************************************************************/

#define CALL_HANDLER(o) \
    do { \
		if (before) \
		{ \
			if (!o->before(ch, argc, argv, cmd, cmdtable[cmd].scmd)) \
				return false; \
		} \
		else \
		{ \
			if (!o->after(ch, argc, argv, cmd, cmdtable[cmd].scmd)) \
				return false; \
		} \
	} while (0)

static bool call_handlers(Character *ch, const int cmd, int argc, char **argv,
						  const bool before)
{
	Object *o;

	/* in the players equipment and inventory */
	for (o = ch->m_children; o; o = o->m_sibling)
	{
		if (o->is_item())
			CALL_HANDLER(o);
	}

	/* in the players environment (items only) */
	for (o = ch->m_parent->m_children; o; o = o->m_sibling)
	{
		if (o->is_item())
			CALL_HANDLER(o);
	}

	/* in the players environment (monsters only) */
	for (o = ch->m_parent->m_children; o; o = o->m_sibling)
	{
		if (o->is_monster())
			CALL_HANDLER(o);
	}

	/* in the players location */
	CALL_HANDLER(ch->m_parent);

	return true;
}

#undef CALL_HANDLER

/*******************************************************************/
