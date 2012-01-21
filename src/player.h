#ifndef __PLAYER_H__
#define __PLAYER_H__
/********************************************************************
* File: player.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "character.h"
#include "user.h"

/*******************************************************************/

class User;

typedef enum hint_e
{
	HINT_STR = 1,
	HINT_INT,
	HINT_WIS,
	HINT_DEX,
	HINT_CON,
	HINT_CHA,
	HINT_WEAPON,
	HINT_ASSASIN,
	HINT_THIEF,
	HINT_HUNT,
	HINT_SPELL,
	HINT_NATURE,
	HINT_ANIMAL,
	HINT_GODS,
	HINT_MAGIC,
} hint_t;
#define MAX_HINT HINT_MAGIC

class Player : public Character
{
public:
	Player();
	virtual ~Player();

	virtual void init(void);
	virtual bool is_player(void) const { return true; }
	virtual bool save(FILE *fl);

	bool save(void);

	virtual bool is_name(const char *names)
		{ return (!names ? false : strcasecmp(names, get_name()) == 0); }

	User *m_user;

	char *m_rlname;   /* players real name */
	char *m_email;    /* players email address */
	char *m_passwd;   /* the players password */
	int   m_npasswd;  /* password attempt counter */
	bool  m_colour;   /* true is the user can see colours */
	int   m_guild;    /* guild this player belongs to */
	char *m_host;     /* last host the player connected from */

	void roll_stats(void);
	void show_rolls(void);

	hint_t m_hint1;
	hint_t m_hint2;

	virtual void write(const char *fmt, ...)
		{
			if (!fmt || !*fmt) return;
			va_list args;
			va_start(args, fmt);
			vwrite(fmt, args);
			va_end(args);
		}
	virtual void vwrite(const char *fmt, va_list args)
		{ if (m_user) m_user->vwrite(fmt, args); }

	void interpret(const char *cmdline, const int linelen);

	virtual void write_prompt(void) { write("> "); }

	virtual void on_tick(void);

private:
	void set_hint(const hint_t hint);
};

Player *load_player(const char *name);
Player *find_player(const char *name);

/*******************************************************************/

class Wizard : public Player
{
public:
	Wizard();
	virtual ~Wizard();
	virtual void init(void);
	virtual bool is_wizard(void) const { return true; }
};

/*******************************************************************/

class Archwiz : public Wizard
{
public:
	Archwiz();
	virtual ~Archwiz();
	virtual void init(void);
	virtual bool is_archwiz(void) const { return true; }
};

/*******************************************************************/
#endif /* __PLAYER_H__ */
