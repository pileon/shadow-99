/********************************************************************
* File: cmd_comm.cc                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "character.h"
#include "interp.h"

/*******************************************************************/

ACMD(do_say)
{
	char       buf[512];
	int        a;
	Object    *o;
	Character *to;

	if (argc == 1)
		ch->write("Say what?\r\n");
	else
	{
		*buf = 0;

		for (a = 1; a < argc; a++)
		{
			strcat(buf, " ");
			strcat(buf, argv[a]);
		}

		for (o = ch->m_parent->m_children; o; o = o->m_sibling)
		{
			if (!o->is_char())
				continue;
			if ((to = (Character *) o) != ch && to->can_hear())
			{
				to->write("%s says, '%s'\r\n",
						  strcap(ch->get_name()), buf + 1);
			}
		}

		ch->write("Ok\r\n");
	}
}

ACMD(do_tell)
{
	Character *to;
	char buf[512] = { 0, };
	int  a;

	if (argc < 3)
	{
		if (argc == 1)
			ch->write("Tell who what?\r\n");
		else
			ch->write("Tell %s what?\r\n", strcap(argv[1]));
		return;
	}

	/* find the character */
	for (to = server.m_chars; to; to = to->m_nextc)
	{
		if (to->is_name(argv[1]))
			break;
	}

	if (to == NULL)
	{
		ch->write("Could not find such a character.\r\n");
		return;
	}
	if (to == ch)
	{
		ch->write("Talking to yourself again?\r\n");
		return;
	}

	for (a = 2; a < argc; a++)
	{
		strcat(buf, " ");
		strcat(buf, argv[a]);
	}

	if (to->can_hear())
	{
		to->write("%s tells you, '%s'\r\n", strcap(ch->get_name()), buf + 1);
		ch->write("Ok\r\n");
	}
	else
		ch->write("%s can not hear you.\r\n", strdup(to->he()));
}

ACMD(do_shout)
{
	/* shout something all over the mud */
	Character *to;
	char buf[512] = { 0, };
	int  a;

	if (argc < 2)
	{
		ch->write("Shout we must, but shout what?\r\n");
		return;
	}

	for (a = 2; a < argc; a++)
	{
		strcat(buf, " ");
		strcat(buf, argv[a]);
	}

	for (to = server.m_chars; to; to = to->m_nextc)
	{
		if (to != ch && to->can_hear())
			to->write("%s shouts, '%s'\r\n", strcap(ch->get_name()), buf + 1);
	}
	ch->write("Ok\r\n");
}

ACMD(do_yell)
{
	/* yell something to all ppl in the same zone as ch */
	Character *to;
	char buf[512] = { 0, };
	int  a;

	if (argc < 2)
	{
		ch->write("Shout we must, but shout what?\r\n");
		return;
	}

	for (a = 2; a < argc; a++)
	{
		strcat(buf, " ");
		strcat(buf, argv[a]);
	}

	for (to = server.m_chars; to; to = to->m_nextc)
	{
		if (to != ch && to->m_zone == ch->m_zone && to->can_hear())
			to->write("%s yells, '%s'\r\n", strcap(ch->get_name()), buf + 1);
	}
	ch->write("Ok\r\n");
}

ACMD(do_comm)
{
	/* whisper something to someone, or ask someone something */
	const char *text;
	const char *comm;
	Character  *to;
	char buf[512] = { 0, };
	int  a;

	if (scmd == SCMD_WHISPER)
	{
		comm = "Whisper";
		text = "whispers to";
	}
	else if (scmd == SCMD_ASK)
	{
		comm = "Ask";
		text = "asks";
	}
	else
	{
		syslog(LOG_SYSERR, "illegal subcmd to do_comm");
		return;
	}

	if (argc < 3)
	{
		if (argc == 1)
			ch->write("%s who what?\r\n", comm);
		else
			ch->write("%s %s what?\r\n", comm, strcap(argv[1]));
		return;
	}

	/* find the character */
	for (to = (Character *) ch->m_parent->m_children; to;
		 to = (Character *) to->m_sibling)
	{
		if (to->is_name(argv[1]))
			break;
	}

	if (to == NULL)
	{
		ch->write("Could not find such a character.\r\n");
		return;
	}
	if (to == ch)
	{
		ch->write("Talking to yourself again?\r\n");
		return;
	}

	for (a = 2; a < argc; a++)
	{
		strcat(buf, " ");
		strcat(buf, argv[a]);
	}

	/* TODO:  broadcast a 'A whispers something to B' message to the room? */
	if (to->can_hear())
	{
		to->write("%s %s you, '%s'\r\n",
				  strcap(ch->get_name()), text, buf + 1);
		ch->write("Ok\r\n");
	}
	else
		ch->write("%s can not hear you.\r\n", strdup(to->he()));
}

/*******************************************************************/
