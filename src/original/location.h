#ifndef __LOCATION_H__
#define __LOCATION_H__
/********************************************************************
* File: location.h                                Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "item.h"

/*******************************************************************/

class Character;

enum
{
	DIR_NORTH,
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST,
	DIR_UP,
	DIR_DOWN,
};
#define N_EXITS 6

class Location : public Object
{
public:
	Location();
	virtual ~Location();

	virtual const bool is_location(void) const { return true; }
	virtual const bool save(FILE *fl);

	virtual const bool move(Character *ch, const int dir);
	virtual void describe(Character *to, const descr_t typ = DESC_SHORT);

	virtual const bool on_enter(Character *ch) { return true; }
	virtual const bool on_leave(Character *ch) { return true; }

	virtual const bool rem_child(Object *child)
		{
			if (child->is_char() && !on_leave((Character *) child))
				return false;
			else
				return Object::rem_child(child);
		}
	virtual const bool add_child(Object *child, Object *oldparen = NULL)
		{
			if (child->is_char() && !on_enter((Character *) child))
				return false;
			else
				return Object::add_child(child);
		}

	Object *m_exits[N_EXITS];
	int     m_flags;  /* see the RF_* below */

	virtual void write(const char *fmt, ...)
		{
			if (!fmt || !*fmt)
				return;
			va_list args;
			va_start(args, fmt);
			vwrite(fmt, args);
			va_end(args);
		}
	virtual void vwrite(const char *fmt, va_list args)
		{
			Object *o;
			for (o = m_children; o; o = o->m_sibling)
			{
				if (o->is_char())
					o->vwrite(fmt, args);
			}
		}
	virtual void write(Object *ch, const char *fmt, ...)
		{
			if (!fmt || !*fmt)
				return;
			va_list args;
			va_start(args, fmt);
			vwrite(ch, fmt, args);
			va_end(args);
		}
	virtual void vwrite(Object *ch, const char *fmt, va_list args)
		{
			Object *o;
			for (o = m_children; o; o = o->m_sibling)
			{
				if (o->is_char() && o != ch)
					o->vwrite(fmt, args);
			}
		}

protected:
	virtual void on_resolve(Object *o, Property *p)
		{
			Object::on_resolve(o, p);
			if (o && o->is_door())
			{
				o->m_parent  = this;
				o->m_sibling = m_children;
				m_children   = o;
			}
		}
};

#define RF_NOFIGHT  0x01  /* can not fight in this location */
#define RF_PERMDARK 0x02  /* location is allways dark */
#define RF_NOMON    0x04  /* monsters are not allowed */

/*******************************************************************/

class Lock : public Object
{
public:
	Lock(void);
	virtual ~Lock(void);

	virtual const bool is_lock(void) const { return true; }
	const bool is_locked(void) const { return m_locked; }

	/* event functions, to allow lock customization
	 * if they return true, the event is allowed to happen
	 */
	virtual const bool on_lock  (Character *ch) { return true; }
	virtual const bool on_unlock(Character *ch) { return true; }
	virtual const bool on_pick  (Character *ch) { return true; }

	virtual const bool is_name(const char *names)
		{ return ::isname(names, get_short()); }

public:
	bool  m_locked;  /* true if the lock is locked */
	int   m_flags;   /* lock flags */
	Key  *m_key;     /* key to lock/unlock this lock */
	char *m_lockd;   /* lock description */
	char *m_lockwd;  /* wrong key lock description */
	char *m_unlockd; /* unlock description */
	char *m_unlockwd;/* wrong key unlock description */
	char *m_picksd;  /* pick success description */
	char *m_pickfd;  /* pick failure description */
};

/* lock flags */
#define LF_NOPICK   0x01  /* lock is not pickable */
#define LF_EASYPICK 0x02  /* lock is easily pickable */
#define LF_HARDPICK 0x04  /* lock is very hard to pick */

/*******************************************************************/

class Door : public Object
{
public:
	Door();
	virtual ~Door();

	virtual const bool is_door(void) const { return true; }
	virtual const bool add_child(Object *child, Object *oldparen = NULL);
	const bool is_open  (void) const { return  m_open; }
	const bool is_closed(void) const { return !m_open; }

	/* event functions, to allow door customization
	 * if they return true, the event is allowed to happen
	 */
	virtual const bool on_open (Character *ch) { return true; }
	virtual const bool on_close(Character *ch) { return true; }
	virtual const bool on_pass (Character *ch) { return true; }

	virtual const bool is_name(const char *names)
		{ return ::isname(names, get_short()); }

public:
	bool      m_open;   /* true if the door is open */
	int       m_flags;  /* door flags */
	char     *m_passd;  /* successfull pass description */
	char     *m_passf;  /* failed pass description */
	char     *m_opend;  /* open desription */
	char     *m_closed; /* close description */
	Lock     *m_lock;   /* the lock used on this door, if any */
	int       m_minlev; /* minimum level to pass door */
	Item     *m_item;   /* item needed to pass */
	Location *m_dir1; /* */
	Location *m_dir2; /* */

protected:
	virtual void on_resolve(Object *o, Property *p)
		{
			if (o && o->is_lock())
			{
				o->m_parent  = this;
				o->m_sibling = m_children;
				m_children   = o;
			}
		}
};

/* door flags (used by Door.m_flags) */
#define DF_HIDDEN  0x01  /* the door is hidden when looking in the room */
#define DF_WIZONLY 0x02  /* only wizards can pass */

/*******************************************************************/
#endif /* __LOCATION_H__ */
