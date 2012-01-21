/********************************************************************
* File: location.cc                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "object.h"
#include "location.h"
#include "character.h"

/*******************************************************************/

static const char *exits[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
};

static const char *revexits[] = {
	"south",
	"west",
	"north",
	"east",
	"down"
	"up",
};

/*******************************************************************/

Location::Location()
	: Object()
{
	memset(m_exits, 0, sizeof(m_exits));
	m_flags = 0;

	add_property("exit_n"   , &m_exits[DIR_NORTH]);
	add_property("exit_e"   , &m_exits[DIR_EAST]);
	add_property("exit_s"   , &m_exits[DIR_SOUTH]);
	add_property("exit_w"   , &m_exits[DIR_WEST]);
	add_property("exit_u"   , &m_exits[DIR_UP]);
	add_property("exit_d"   , &m_exits[DIR_DOWN]);
	add_property("nofight"  , &m_flags, RF_NOFIGHT);
	add_property("permdark" , &m_flags, RF_PERMDARK);
	add_property("nomonster", &m_flags, RF_NOMON);
}

Location::~Location()
{
}

/*******************************************************************/

bool Location::save(FILE *fl)
{
	if (!Object::save(fl))
		return false;

	const char *save_exits[] = {
		"exit_n",
		"exit_e",
		"exit_s",
		"exit_w",
		"exit_u",
		"exit_d",
	};
	int e;

	fprintf(fl, "\t/* Location properties */\n");
	for (e = 0; e < N_EXITS; e++)
	{
		if (m_exits[e] != NULL)
			fprintf(fl, "\t%s = %s;\n", save_exits[e], fullname(m_exits[e]));
	}
	fprintf(fl, "\n");

	return true;
}

/*******************************************************************/

bool Location::move(Character *ch, const int dir)
{
	if (!ch || !ch->m_parent || !ch->is_char())
		return false;

	Location  *old = (Location *) ch->m_parent;

	if (!m_exits[dir])
	{
		ch->write("There is no exit in that direction.\r\n");
		return false;
	}

	/* do the actual moving */
	if (!ch->move(m_exits[dir]))
		return false;

	if (old)
	{
		old->write("%s leaves %s\r\n",
				   strcap(ch->is_monster() ?
						  ch->get_short() : ch->get_name()),
				   exits[dir]);
	}

	char s[64];
	if (dir == DIR_UP)
		strcpy(s, "below");
	else if (dir == DIR_DOWN)
		strcpy(s, "above");
	else
		sprintf(s, "the %s", revexits[dir]);
	((Location *) ch->m_parent)->write(ch, "%s enters from %s.\r\n",
				   strcap(ch->is_monster() ?
						  ch->get_short() : ch->get_name()), s);

	ch->m_parent->describe(ch, DESC_LONG);

	return true;
}

void Location::describe(Character *to, const descr_t typ /* = DESC_SHORT */)
{
	if (!to || typ != DESC_LONG)
		return;

	if (to->is_wizard())
		to->write("&2%s &6[%s]&0\r\n", get_short(), fullname());
	else
		to->write("&2%s&0\r\n", get_short());

	to->write(get_descr());

	int   dir;
	char  s[64] = { 0 };
	Door *d;
	for (dir = 0; dir < N_EXITS; dir++)
	{
		if (m_exits[dir] != NULL)
		{
			if (m_exits[dir]->is_door())
			{
				d = (Door *) m_exits[dir];
				if ((d->m_flags & DF_HIDDEN) && !to->is_wizard())
					continue;
			}

			strcat(s, " ");
			strcat(s, strcap(exits[dir]));
		}
	}
	to->write("&2[Exits:%s ]&0\r\n", *s ? s : " None");

	/* show all items */
	Object *o;
	for (o = m_children; o; o = o->m_sibling)
	{
		if (o->is_item())
			o->describe(to, DESC_LONG);
	}

	/* show all characters */
	for (o = m_children; o; o = o->m_sibling)
	{
		if (o->is_char() && o != to)
			o->describe(to, DESC_SHORT);
	}
}

/*******************************************************************/

Door::Door()
	: Object()
{
	add_property("isopen"   , &m_open  );
	add_property("passdesc" , &m_passd );
	add_property("passfail" , &m_passf );
	add_property("opendesc" , &m_opend );
	add_property("closedesc", &m_closed);
	add_property("minlev"   , &m_minlev);
	add_property("lock"     , (Object **) &m_lock);
	add_property("item"     , (Object **) &m_item);
	add_property("dir1"     , (Object **) &m_dir1);
	add_property("dir2"     , (Object **) &m_dir2);
	add_property("hidden"   , &m_flags, DF_HIDDEN);
	add_property("wizonly"  , &m_flags, DF_WIZONLY);

	m_open   = true; /* the door is open by default */
	m_passd  = NULL;
	m_passf  = NULL;
	m_opend  = NULL;
	m_closed = NULL;
	m_lock   = NULL;
	m_item   = NULL;
	m_dir1   = NULL;
	m_dir2   = NULL;
	m_flags  = 0;
	m_minlev = 0;
}

Door::~Door()
{
	if (m_passd)
		delete [] m_passd;
	if (m_passf)
		delete [] m_passf;
	if (m_opend)
		delete [] m_opend;
	if (m_closed)
		delete [] m_closed;
}

bool Door::add_child(Object *o, Object *oldparen /* = NULL */)
{
	if (oldparen == NULL)
	{
		syslog(LOG_SYSERR, "Old parent NULL in door passing");
		return false;
	}

	if (!o->is_char())
	{
		syslog(LOG_SYSERR, "Object passing through door not a char");
		return false;
	}

	Character *ch = (Character *) o;

	if ((m_flags & DF_WIZONLY) && !ch->is_wizard())
	{
		ch->write("Only wizards can pass.\r\n");
		return false;
	}

	if (!is_open() && !ch->is_wizard())
	{
		ch->write("The %s is closed.\r\n", fname(m_alias));
		return false;
	}

	if (ch->m_level < m_minlev && !ch->is_wizard())
	{
		ch->write(m_passf);
		return false;
	}

	/* check if the user carries the right item */
	if (m_item && !ch->is_wizard())
	{
		Item *i;
		for (i = (Item *) ch->m_children; i; i = (Item *) i->m_sibling)
		{
			if (*i->get_name() == *m_item->get_name() &&
				strcasecmp(i->get_name(), m_item->get_name()) == 0)
			{
				break;
			}
		}

		if (i != NULL)
		{
			ch->write(m_passf);
			return false;
		}
	}

	if (!ch->is_wizard() && !on_pass(ch))
	{
		/* it is assumed that the on_pass()
		   function write a message if it fails */
		return false;
	}

	if (m_passd)
		ch->write(m_passd);
		
	if (oldparen == m_dir1)
		return m_dir2->add_child(ch);
	else if (oldparen == m_dir2)
		return m_dir1->add_child(ch);
	else
	{
		ch->write("You have discovered a bug in the MUD, "
				  "please notify a wizard about it.\r\n");
		syslog(LOG_SYSERR, "%s: Illegal door passing (by %s)",
			   get_name(), ch->get_name());
		//log("   [ch->m_parent = %s, m_dir1 = %s, m_dir2 = %s]\n",
		//	ch->m_parent->get_name(),
		//	m_dir1->get_name(), m_dir2->get_name());
		return false;
	}
}

/*******************************************************************/

Lock::Lock(void)
	: Object()
{
	add_property("locked"     , &m_locked);
	add_property("lockdesc"   , &m_lockd);
	add_property("lockwrong"  , &m_lockwd);
	add_property("unlockdesc" , &m_unlockd);
	add_property("unlockwrong", &m_unlockwd);
	add_property("pickdesc"   , &m_picksd);
	add_property("pickfail"   , &m_pickfd);
	add_property("key"        , (Object **) &m_key);
	add_property("flags"      , &m_flags);
	add_property("nopick"     , &m_flags, LF_NOPICK);

	m_key      = NULL;
	m_lockd    = NULL;
	m_lockwd   = NULL;
	m_unlockd  = NULL;
	m_unlockwd = NULL;
	m_picksd   = NULL;
	m_pickfd   = NULL;
}

Lock::~Lock(void)
{
	if (m_lockd)
		delete [] m_lockd;
	if (m_lockwd)
		delete [] m_lockwd;
	if (m_unlockd)
		delete [] m_unlockd;
	if (m_unlockwd)
		delete [] m_unlockwd;
	if (m_picksd)
		delete [] m_picksd;
	if (m_pickfd)
		delete [] m_pickfd;
}

/*******************************************************************/
