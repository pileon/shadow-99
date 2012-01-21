/********************************************************************
* File: cmd_item.cc                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "item.h"
#include "character.h"
#include "interp.h"

/*******************************************************************/

ACMD(do_get)
{
	Object   *o;
	Location *l = (Location *) ch->m_parent;

	/* get an item from the ground, or from a container */
	if (argc == 1)
	{
		ch->write("Get what?\r\n");
		return;
	}

	/* does the char wants it from a container? */
	/* NOT IMPLEMENTED YET! */

	/* check in the players equipment and inventory */
	for (o = ch->m_children; o; o = o->m_sibling)
	{
		if (o->is_item() && o->is_name(argv[1]))
		{
			ch->write("You allready have the %s.\r\n", argv[1]);
			return;
		}
	}

	/* ok, do pick up the thing from the ground */
	for (o = ch->m_parent->m_children; o; o = o->m_sibling)
	{
		if (o->is_item() && o->is_name(argv[1]))
			break;
	}
	if (o == NULL)
	{
		ch->write("There doesn't seem to be any %s around.\r\n", argv[1]);
		return;
	}

	if (o->move(ch))
	{
		ch->write("Ok, picked up.\r\n");
		l->write(ch, "%s picks up %s.\r\n",
				 strcap(ch->get_name()), o->get_short());
	}
}

ACMD(do_drop)
{
	/* drop an item on the ground */
	Object   *o = NULL;
	Location *l = (Location *) ch->m_parent;

	/* search for the object */
	for (o = ch->m_children; o; o = o->m_sibling)
	{
		if (o->is_item() && o->is_name(argv[1]))
			break;
	}
	if (o == NULL)
	{
		ch->write("You don't have that item.\r\n");
		return;
	}

	if (o->move(l))
	{
		ch->write("Ok, dropped.\r\n");
		l->write(ch, "%s drops %s.\r\n",
				 strcap(ch->get_name()), o->get_short());
	}
}

ACMD(do_give)
{
	/* give an item to another character */
	Item      *i = NULL;
	Object    *o = NULL;
	Location  *l = (Location *) ch->m_parent;
	Character *c = NULL;

	if (argc < 3)
	{
		if (argc == 1)
			ch->write("Give what to who?\r\n");
		else if (argc == 2)
			ch->write("But who do you want to give it to?\r\n");
		return;
	}

	if (ch->is_name(argv[2]))
	{
		ch->write("Giving yourself stuff is very productive indeed.\r\n");
		return;
	}

	/* find the item in the players inventory */
	for (o = ch->m_children; o; o = o->m_sibling)
	{
		if (o->is_item() && o->is_name(argv[1]))
			break;
	}
	if ((i = (Item *) o) == NULL)
	{
		ch->write("You don't seem to have any '%s' on you.\r\n", argv[1]);
		return;
	}
	if (i->is_equiped())
	{
		ch->write("Better remove %s from your equipment first.\r\n",
				  i->get_short());
		return;
	}

	/* ok, now we have an item, find the char to give it to */
	for (o = l->m_children; o; o = o->m_sibling)
	{
		if (o->is_char() && o->is_name(argv[2]) && o != ch)
			break;
	}
	if ((c = (Character *) o) == NULL)
	{
		ch->write("%s doesn't seem to be around.\r\n", strcap(argv[2]));
		return;
	}

	if (i->move(c))
	{
		ch->write("Ok.\r\n");
		c ->write("%s gives you %s.\r\n",
				  strcap(ch->get_name()), i->get_short());

		Character *c2;
		char       n1[128], n2[128];
		strcpy(n1, strcap(ch->get_name()));
		strcpy(n2, strcap(c ->get_name()));

		for (o = ch->m_parent->m_children; o; o = o->m_sibling)
		{
			if (o->is_char() && o != ch && o != c)
			{
				c2 = (Character *) o;
				c2->write("%s gives %s %s.\r\n", n1, n2, i->get_short());
			}
		}
	}
}

/*******************************************************************/
