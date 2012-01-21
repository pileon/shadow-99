#ifndef __MONSTER_H__
#define __MONSTER_H__
/********************************************************************
* File: monster.h                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "character.h"

/*******************************************************************/

class Character;

class Monster : public Character
{
public:
	Monster();
	virtual ~Monster();

	virtual void init(void);
	virtual const bool is_monster(void) const { return true; }

	virtual const bool is_name(const char *names)
		{ return ::isname(names, get_alias()); }

	virtual void on_action(void);

public:
	int     m_flags;   /* monster flags */
	Object *m_initial; /* where to place on zone reset */

protected:
	virtual void on_clone(Object *clone) const { *(Monster *) clone = *this; }
};

#define MF_SENTINEL 0x0001  /* monster should not vander around */
#define MF_MEMORY   0x0002  /* the monster remembers their opponents */

/*******************************************************************/
#endif /* __MONSTER_H__ */
