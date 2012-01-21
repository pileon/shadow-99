#ifndef __CHARACTER_H__
#define __CHARACTER_H__
/********************************************************************
* File: character.h                               Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "object.h"
#include "skills.h"

#include <stdarg.h>

/*******************************************************************/

enum
{
	STAT_STR = 0,
	STAT_INT,
	STAT_WIS,
	STAT_DEX,
	STAT_CON,
	STAT_CHA,
	STAT_LUK,
	STAT_EGO
};
#define N_STATS 8

class Character : public Object
{
public:
	Character();
	virtual ~Character();

	virtual void init(void);

	virtual const bool is_char(void) const { return true; }

	virtual const bool save(FILE *fl);
	virtual void describe(Character *to, const descr_t typ = DESC_SHORT);
	virtual const bool can_hear(void)     const { return true; }
	virtual const bool can_see(Object *o) const { return true; }

	/* make some of these variables private? */
	Character *m_nextc;   /* link for the servers m_chars list */
	int m_curhit;  /* current amount of hit-points this character have */
	int m_curmov;  /* current amount of move-points this character have */
	int m_curman;  /* current amount of mana-points this character have */
	int m_maxhit;  /* max amount of hit-points */
	int m_maxmov;  /* max amount of move-points */
	int m_maxman;  /* max amount of mana-points */
	int m_reghit;  /* the amount of hit-points to regenerate each tick */
	int m_regmov;  /* the amount of move-points to regenerate each tick */
	int m_regman;  /* the amount of mana-points to regenerate each tick */
	int m_level;   /* all characters have a level */
	int m_age;
	int m_stats[N_STATS];
	int m_skills[N_SKILLS];

	enum
	{
		SEX_MALE = 0,
		SEX_FEMALE,
		SEX_NEUTRAL
	} m_sex;

	const char *him(void) const
		{ return (m_sex == SEX_MALE ? "him" :
				  m_sex == SEX_FEMALE ? "her" : "it"); }
	const char *he(void) const
		{ return (m_sex == SEX_MALE ? "he" :
				  m_sex == SEX_FEMALE ? "she" : "it"); }
	const char *his(void) const
		{ return (m_sex == SEX_MALE ? "his" :
				  m_sex == SEX_FEMALE ? "her" : "its"); }

	virtual void on_tick (void);
	virtual void on_fight(void) {}
};

void write(Character *ch, const char *fmt, ...);
void write_all(const char *fmt, ...);
void write_all_but(Character *ch, const char *fmt, ...);

/*******************************************************************/
#endif /* __CHARACTER_H__ */
