/********************************************************************
* File: heartbeat.cc                              Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "server.h"
#include "monster.h"
#include "user.h"
#include "player.h"

/*******************************************************************/

void Server::heartbeat(void)
{
	m_beats++;

	if ((m_beats % BEATS_PER_TICK) == 0)
		round_tick();
	if ((m_beats % BEATS_PER_MONSTER) == 0)
		round_monster();
	if ((m_beats % BEATS_PER_ROUND) == 0)
		round_violence();
	if ((m_beats % BEATS_PER_AUTOSAVE) == 0)
		round_autosave();

	/* TODO:  update zones, update weather, update affects */

	beat_sleep();
}

void Server::beat_sleep(void)
{
	timeval tv;

	tv.tv_sec  = 0;
	tv.tv_usec = 1000000 / BEATS_PER_SEC;

	if (select(0, NULL, NULL, NULL, &tv) < 0)
		perror("Server::sleep");
}

void Server::round_tick(void)
{
	Object *o;

	/* update all objects */
	for (o = m_objects; o; o = o->m_nexto)
		o->on_tick();

	/* update mud clock (one RL minute is one MUD hour) */
}

void Server::round_monster(void)
{
	Character *c;
	Monster   *m;

	for (c = m_chars; c; c = c->m_nextc)
	{
		/* only move around the clones, not the prototypes */
		if (c->is_monster() && c->is_clone())
		{
			m = (Monster *) c;
			m->on_action();
		}
	}
}

void Server::round_violence(void)
{
	Object *o;

	for (o = m_objects; o; o = o->m_nexto)
		o->on_fight();
}

void Server::round_autosave(void)
{
	User *u;

	for (u = m_users; u; u = u->m_next)
	{
		if (u->m_player)
			u->m_player->save();
	}
}

/*******************************************************************/
