/********************************************************************
* File: cmd_move.cc                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "location.h"
#include "character.h"
#include "interp.h"

/*******************************************************************/

bool move(Character *ch, const int dir)
{
	if (!ch)
		return false;  /* TODO:  log this condition? */
	if (!ch->m_parent || !ch->m_parent->is_location())
	{
		ch->write("How can you move when you don't are anywhere?\r\n");
		return false;
	}

	Location *loc = (Location *) ch->m_parent;
	return loc->move(ch, dir);
}

/*******************************************************************/

ACMD(do_move)
{
	switch (scmd)
	{
	case DIR_NORTH:
	case DIR_EAST:
	case DIR_SOUTH:
	case DIR_WEST:
	case DIR_UP:
	case DIR_DOWN:
		move(ch, scmd);
		break;
	default:
		break;
	}
}

/*******************************************************************/

static const bool find_door(int argc, char **argv, Character *ch, Door **door)
{
	Location *l = (Location *) ch->m_parent;
	Door     *d = NULL;
	int       e;

	if (argc == 3 &&
		(strcasecmp(argv[1], "door") == 0 || strcasecmp(argv[1], "exit") == 0))
	{
		static const char *exits[] = {
			"north",
			"east",
			"south",
			"west",
			"up",
			"down",
		};

		for (e = 0; e < N_EXITS && d == NULL; e++)
		{
			if (argv[2][0] == exits[e][0])
			{
				if (argv[2][1] != 0)
				{
					if (strcasecmp(argv[2], exits[e]) == 0)
						d = (Door *) l->m_exits[e];
				}
				else
					d = (Door *) l->m_exits[e];
			}
			if (d != NULL && !l->m_exits[e]->is_door())
			{
				ch->write("That exit is not a door.\r\n");
				return false;
			}
		}
	}
	else
	{
		for (e = 0; e < N_EXITS; e++)
		{
			if (l->m_exits[e] &&
				l->m_exits[e]->is_door() &&l->m_exits[e]->is_name(argv[1]))
			{
				d = (Door *) l->m_exits[e];
				break;
			}
		}
	}

	if (d == NULL)
	{
		ch->write("No such door or exit around.\r\n");
		return false;
	}

	*door = d;
	return true;
}

static const bool find_lock(int argc, char **argv, Character *ch, Lock **lock)
{
	Door     *d   = NULL;
	Lock     *l   = NULL;
	Location *loc = (Location *) ch->m_parent;
	int       e;

	/* search for a named lock */
	for (e = 0; e < N_EXITS && l == NULL; e++)
	{
		if ((d = (Door *) loc->m_exits[e]) != NULL &&
			loc->m_exits[e]->is_door())
		{
			if (d->m_lock && d->m_lock->is_name(argv[1]))
			{
				l = d->m_lock;
				break;
			}
		}
	}

	if (l == NULL)
	{
		/* no lock found, look for door with a lock */
		if (!find_door(argc, argv, ch, &d))
			return false;

		if ((l = d->m_lock) == NULL)
		{
			ch->write("%s does not have any lock.\r\n",
					  strcap(fname(d->get_alias())));
			return false;
		}
	}

	if (l->m_parent && l->m_parent->is_door())
	{
		d = (Door *) l->m_parent;
		if (d->is_open())
		{
			ch->write("%s is open, and can not be locked or unlocked.\r\n",
					  strcap(fname(d->get_alias())));
			return false;
		}
	}

	*lock = l;
	return true;
}

static const bool set_door(Character *ch, Door *door, const bool opn)
{
	char *openclose = opn ? "open" : "closed";

	if (door->m_open == opn)
	{
		ch->write("It is allready %s.\r\n", openclose);
		return false;
	}

	if (!ch->is_wizard() && !(opn ? door->on_open(ch) : door->on_close(ch)))
		return false;

	char *desc = opn ? door->m_opend : door->m_closed;

	if (desc)
		ch->write(desc);
	else
		ch->write("You %s the %s.\r\n", openclose, fname(door->get_alias()));

	openclose = opn ? "opened" : "closed";
	if (ch->m_parent == door->m_dir1)
	{
		door->m_dir2->write("The %s is %s from the other side.\r\n",
							fname(door->get_alias()), openclose);
	}
	else if (ch->m_parent == door->m_dir2)
	{
		door->m_dir1->write("The %s is %s from the other side.\r\n",
							fname(door->get_alias()), openclose);
	}

	door->m_open = opn;
	return true;
}

static const bool set_lock(Character *ch, Lock *l, const bool lock)
{
	char *locks = lock ? "locked" : "unlocked";

	if (( lock &&  l->is_locked()) ||
		(!lock && !l->is_locked()))
	{
		ch->write("The %s is already %s.\r\n", fname(l->get_alias()), locks);
		return false;
	}

	/* search for a key */
	if (!ch->is_wizard() && l->m_key)
	{
		/* search the character inventory for the key */
		Object *o = NULL;

		for (o = ch->m_children; o; o = o->m_sibling)
		{
			if (o->is_key())
			{
				if (o == l->m_key ||
					strcasecmp(o->get_name(), l->m_key->get_name()) == 0)
				{
					break;
				}
			}
		}

		if (o == NULL)
		{
			ch->write("You don't have the proper key.\r\n");
			return false;
		}
	}

	if (!ch->is_wizard())
	{
		if (( lock && !l->on_lock  (ch)) ||
			(!lock && !l->on_unlock(ch)))
		{
			/* it is assumed that on_unlock/on_lock
			 * writes a failure description
			 */
			return false;
		}
	}

	/* everything is ok, unlock it */
	l->m_locked = lock;

	char *desc = lock ? l->m_lockd : l->m_unlockd;
	if (desc)
		ch->write(desc);
	else
	{
		locks = lock ? "lock" : "unlock";
		ch->write("You %s the %s.\r\n", locks, fname(l->get_alias()));
	}

	return true;
}

/*******************************************************************/

ACMD(do_lock)
{
	if (argc < 2)
	{
		ch->write("Lock what?\r\n");
		return;
	}

	Lock *lock;

	/* find the exit */
	if (!find_lock(argc, argv, ch, &lock))
		return;

	/* now we have a lock, lock it */
	set_lock(ch, lock, true);
}

ACMD(do_unlock)
{
	if (argc < 2)
	{
		ch->write("Lock what?\r\n");
		return;
	}

	Lock *lock;

	/* find the exit */
	if (!find_lock(argc, argv, ch, &lock))
		return;

	/* now we have a lock, unlock it */
	set_lock(ch, lock, false);
}

ACMD(do_open)
{
	Door *door;

	if (!find_door(argc, argv, ch, &door))
		return;

	if (door->m_lock && door->m_lock->m_locked)
	{
		/* TODO:  unlock the door automaticly? */
		ch->write("The %s is locked.\r\n", fname(door->get_alias()));
		return;
	}

	set_door(ch, door, true);
}

ACMD(do_close)
{
	Door *door;

	if (!find_door(argc, argv, ch, &door))
		return;

	set_door(ch, door, false);
}

/*******************************************************************/

ACMD(do_goto)
{
	if (!argc)
	{
		ch->write("Goto where?\r\n");
		return;
	}

	Object *o = find_object(argv[1]);
	if (o == NULL || !o->is_location())
	{
		ch->write("No such location found: %s\r\n", argv[1]);
		return;
	}

	ch->move(o);
	ch->m_parent->describe(ch, DESC_LONG);
}

/*******************************************************************/
