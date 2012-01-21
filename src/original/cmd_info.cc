/********************************************************************
* File: cmd_info.cc                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "player.h"
#include "item.h"
#include "interp.h"
#include <sys/resource.h>

/*******************************************************************/

static void perform_read(Character *ch, int argc, char **argv)
{
}

static void perform_exam(Character *ch, int argc, char **argv)
{
	Object *o = NULL;
	Item   *i = NULL;

	/* first go through ch's equipment list */
	for (o = ch->m_children; o; o = o->m_sibling)
	{
		if (o->is_item())
		{
			i = (Item *) o;
			if (i->is_equiped() && i->is_name(argv[1]))
				break;
		}
	}

	if (o == NULL)
	{
		/* search ch's inventory list */
		for (o = ch->m_children; o; o = o->m_sibling)
		{
			if (o->is_item())
			{
				i = (Item *) o;
				if (!i->is_equiped() && i->is_name(argv[1]))
					break;
			}
		}
	}

	if (o == NULL)
	{
		/* and finaly search the location ch is in */
		for (o = ch->m_parent->m_children; o; o = o->m_sibling)
		{
			if (o->is_name(argv[1]))
				break;
		}
	}

	if (o != NULL)
		o->describe(ch, DESC_LONG);
	else
		ch->write("Couldn't find any '%s'.\r\n", argv[1]);
}

static void perform_look(Character *ch, int argc, char **argv)
{
	if (argc == 1)
		ch->m_parent->describe(ch, DESC_LONG);
	else if (argc == 2 || argc == 3)
	{
		if (argc == 3)
		{
			if (strcasecmp(argv[1], "at") != 0)
			{
				ch->write("Usage: look [[at] <item|character>]\r\n");
				return;
			}
			argc--;
			argv++;
		}

		perform_exam(ch, argc, argv);
	}
	else
		ch->write("Usage: look [[at] <item|character>]\r\n");
}

/*******************************************************************/

ACMD(do_look)
{
	switch (scmd)
	{
	case SCMD_LOOK:
		perform_look(ch, argc, argv);
		break;
	case SCMD_EXAM:
		perform_exam(ch, argc, argv);
		break;
	case SCMD_READ:
		perform_read(ch, argc, argv);
		break;
	default:
		syslog(LOG_SYSERR, "Illegal do_look subcommand: %d", scmd);
		break;
	}
}

ACMD(do_score)
{
	ch->write("Your score is: 0  (Duh!)\r\n");
}

ACMD(do_info)
{
	ch->write("Stats: Strength : %-5d Inteligence : %-5d Wisdom  : %-5d\r\n"
			  "       Dexterity: %-5d Constitution: %-5d Charisma: %-5d\r\n",
			  ch->m_stats[STAT_STR], ch->m_stats[STAT_INT],
			  ch->m_stats[STAT_WIS], ch->m_stats[STAT_DEX],
			  ch->m_stats[STAT_CON], ch->m_stats[STAT_CHA]);
	ch->write("\r\n");
	ch->write("You have %d/%d hitpoints, %d/%d movepoints "
			  "and %d/%d manapoints.\r\n",
			  ch->m_curhit, ch->m_maxhit, ch->m_curmov, ch->m_maxmov,
			  ch->m_curman, ch->m_maxman);
}

ACMD(do_who)
{
	User   *u;
	Player *p;
	int     n;

	ch->write("\r\nPeople playing on %s:\r\n", mudconfig.mudname);
	ch->write("-------------------------------\r\n");

	for (u = server.m_users, n = 0; u; u = u->m_next, n++)
	{
		/* if the user doesn't have a player, don't list him */
		if ((p = u->m_player) == NULL)
			continue;

		/* If the player is actually playing, include him in the list.
		 * The previous state of a user is only set by the pager and
		 * editor, and will be S_PLAYING if the user was playing before
		 * the editing/paging.
		 */
		if (u->get_state()      == User::S_PLAYING ||
			u->get_prev_state() == User::S_PLAYING)
		{
			if (p->is_wizard())
				ch->write("[WIZ] %s\r\n", p->m_name);
			else
				ch->write("[%3d] %s\r\n", p->m_level, p->m_name);
		}
		else
			n--;  /* either the player has no user, or is not playing */
	}

	if (n == 1)
		ch->write("\r\nOnly you are playing at the moment.\r\n");
	else if (n > 1)
		ch->write("\r\n%d players are playing right now.\r\n\r\n", n);
	else
		ch->write("\r\nHuh?!?  No players?!?\r\n\r\n");
}

/*******************************************************************/

ACMD(do_rusage)
{
	/* NOTE :  I have no idea if this works! */
	/* NOTE2:  the user time/system time calsulations are incorrect */

	struct rusage ru;

	getrusage(RUSAGE_SELF, &ru);
	ch->write("rusage: user time   : %ld.%ld sec, system time: %ld.%ld sec\r\n"
			  "        max res size: %ld kbytes\r\n"
			  "        # input ops : %ld, # output ops: %ld\r\n"
			  "        # context sw: %ld\r\n",
			  ru.ru_utime.tv_sec,
			  (ru.ru_utime.tv_sec * 1000000 + ru.ru_utime.tv_usec) / 1000000,
			  ru.ru_stime.tv_sec,
			  (ru.ru_stime.tv_sec * 1000000 + ru.ru_stime.tv_usec) / 1000000,
			  ru.ru_maxrss,
			  ru.ru_inblock, ru.ru_oublock, ru.ru_nvcsw + ru.ru_nivcsw);
}

ACMD(do_count)
{
	if (argc < 2)
	{
		ch->write("Count what?\r\n");
		return;
	}

	bool m = false;  /* monsters? */
	bool i = false;  /* items? */
	bool l = false;  /* locations? */
	bool c = false;  /* characters? */
	bool p = false;  /* players? */
	bool w = false;  /* wizards? */
	bool a = false;  /* all? */

	if (is_abbrev(argv[1], "monsters"))
		m = true;
	else if (is_abbrev(argv[1], "items"))
		i = true;
	else if (is_abbrev(argv[1], "locations"))
		l = true;
	else if (is_abbrev(argv[1], "characters"))
		c = true;
	else if (is_abbrev(argv[1], "players"))
		p = true;
	else if (is_abbrev(argv[1], "wizards"))
		w = true;
	else if (is_abbrev(argv[1], "objects") || is_abbrev(argv[1], "all"))
		a = true;
	else
	{
		ch->write("That is not a legal thing to count.\r\n");
		return;
	}

	/* now do the count */
	Object *o;
	int     cnt;
	for (cnt = 0, o = server.m_objects; o; o = o->m_nexto)
	{
		if ((m && o->is_monster ()) ||
			(i && o->is_item    ()) ||
			(l && o->is_location()) ||
			(c && o->is_char    ()) ||
			(p && o->is_player  ()) ||
			(w && o->is_wizard  ()) || a)
		{
			cnt++;
		}
	}

	char *type = (m ? "monster"   : i ? "item"   : l ? "location" :
				  c ? "character" : p ? "player" : w ? "wizard"   :
				  a ? "object"    : "unknown");
	ch->write("There are %d %s%s\r\n", cnt, type, cnt != 1 ? "s" : "");
}

/*******************************************************************/

ACMD(do_commands)
{
	extern struct cmdentry cmdtable[];
	int i;

	ch->write("The following commands are available for you:\r\n\r\n");

	for (i = 0; ; i++)
	{
		if (cmdtable[i].verb == NULL)
			break;  /* no more entries after this one */
		if (cmdtable[i].wiz && !ch->is_wizard())
			continue;
		if (cmdtable[i].level > ch->m_level)
			continue;

		ch->write("%15s", cmdtable[i].verb);

		if (((i + 1) % 4) == 0)
			ch->write("\r\n");
	}
	ch->write("\r\n");
}

/*******************************************************************/

#define ONOFF(b)     ((b) ? "on"   : "off"  )
#define TRUEFALSE(b) ((b) ? "true" : "false")

static void show_toggles(Player *p)
{

	p->write("Current settings:\r\n"
			 "  Colour: %-10s\r\n",
			 ONOFF(p->m_colour));
}

#undef TRUEFALSE
#undef ONOFF

ACMD(do_toggle)
{
	if (!ch->is_player())
	{
		ch->write("Only players have settings.\r\n");
		return;
	}
	Player *p = (Player *) ch;

	if (argc < 2)
	{
		show_toggles(p);
		return;
	}

	if (is_abbrev(argv[1], "colour"))
		p->m_colour = !p->m_colour;
	else
	{
		p->write("Unknown toggle: %s\r\n", argv[1]);
		return;
	}

	p->save();
}

/*******************************************************************/
