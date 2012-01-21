/********************************************************************
* File: monster.cc                                Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "monster.h"
#include "location.h"

/*******************************************************************/

Monster::Monster()
	: Character()
{
	m_flags = 0;

	add_property("sentinel", &m_flags, MF_SENTINEL);
	add_property("memory"  , &m_flags, MF_MEMORY  );
}

Monster::~Monster()
{
}

void Monster::init(void)
{
	Character::init();
}

/*******************************************************************/

/* This function (called from the heartbeat functions) moves monsters
 * around, check if aggressive monsters should attack, and other
 * such things.
 */
void Monster::on_action(void)
{
	if (!(m_flags & MF_SENTINEL))
	{
		/* move around in a random directory */
		int       dir = number(0, N_EXITS - 1);
		Location *loc = (Location *) m_parent;

		if (loc->m_exits[dir] != NULL && !(loc->m_flags & RF_NOMON))
			loc->move(this, dir);
	}
}

/*******************************************************************/
