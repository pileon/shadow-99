/********************************************************************
* File: player.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "object.h"
#include "character.h"
#include "player.h"
#include "loader.h"

/*******************************************************************/

Player::Player()
	: Character()
{
	m_user    = NULL;
	m_rlname  = NULL;
	m_email   = NULL;
	m_passwd  = NULL;
	m_npasswd = 0;
	m_guild   = GUILD_NONE;
	m_host    = NULL;

	add_property("rlname" , &m_rlname);
	add_property("email"  , &m_email);
	add_property("passwd" , &m_passwd);
	add_property("npasswd", &m_npasswd);
	add_property("colour" , &m_colour);
	add_property("host"   , &m_host);
}

Player::~Player()
{
	if (m_rlname)
		delete [] m_rlname;
	if (m_email)
		delete [] m_email;
	if (m_passwd)
		delete [] m_passwd;
	if (m_host)
		delete [] m_host;
}

void Player::init(void)
{
	Character::init();
}

/*******************************************************************/

bool Player::save(FILE *fl)
{
	if (!Character::save(fl))
		return false;

	fprintf(fl, "\t/* Player properties */\n");
	fprintf(fl, "\trlname  = \"%s\";\n", NULLSTR(m_rlname));
	fprintf(fl, "\temail   = \"%s\";\n", NULLSTR(m_email ));
	fprintf(fl, "\tpasswd  = \"%s\";\n", NULLSTR(m_passwd));
	fprintf(fl, "\tnpasswd = %d;\n"    , m_npasswd);
	fprintf(fl, "\tcolour  = %s;\n"    , m_colour ? "true" : "false");
	fprintf(fl, "\thost    = \"%s\";\n", NULLSTR(m_host));
	fprintf(fl, "\n");

	return true;
}

bool Player::save(void)
{
	/* save a player to file */
	FILE *fl;
	char  fn[PATH_MAX + 1];
	char  name[64];
	bool  rc = true;

	strlower(m_name, name, 63);
	if (is_wizard())
		snprintf(fn, sizeof(fn), "etc/saves/wiz/%c/%s.sav", *name, name);
	else
		snprintf(fn, sizeof(fn), "etc/saves/%c/%s.sav", *name, name);

	if ((fl = fopen(fn, "w")) == NULL)
	{
		log("Failed to save %s %s\n",
			is_wizard() ? "wizard" : "player", m_name);
		perror("Player::save");
		/* TODO:  inform the player? */
		return false;
	}

	fprintf(fl, "/* This is the savefile for %s %s, do not touch! */\n\n"	,
			is_archwiz() ? "archwiz" : is_wizard() ? "wizard" : "player",
			m_name);
	fprintf(fl, "%s %s\n\{\n",
			is_archwiz() ? "archwiz" : is_wizard() ? "wizard" : "player",
			name);
	rc = save(fl);
	fprintf(fl, "}\n");

	fclose(fl);

	if (!rc)
	{
		/* TODO:  inform the player? */
	}

	return rc;
}

Player *load_player(const char *pname)
{
	Loader *loader;
	char    fn[PATH_MAX + 1];
	char    name[64];
	Player *plr = NULL;

	strlower(pname, name, 63);
	snprintf(fn, sizeof(fn), "etc/saves/%c/%s.sav", *name, name);
	if (file_exists(fn) <= 0)
	{
		snprintf(fn, sizeof(fn), "etc/saves/wiz/%c/%s.sav", *name, name);
		if (file_exists(fn) <= 0)
			return (Player *) -1;
	}

	if ((loader = new Loader(fn, fn + strlen("etc/saves/"))) == NULL)
		out_of_memory();

	if (!loader->load(true))
		plr = NULL;
	else
		plr = (Player *) loader->get_player();

	delete loader;

	return plr;
}

Player *find_player(const char *name)
{
	Character *ch;

	for (ch = server.m_chars; ch; ch = ch->m_nextc)
	{
		if (!ch->is_player())
			continue;
		if (*ch->get_name() == *name && strcasecmp(ch->get_name(), name) == 0)
			return (Player *) ch;
	}
	return NULL;
}

/*******************************************************************/

void Player::roll_stats(void)
{
	m_age = number(16, 20);

	/* roll the basics */
	m_maxhit = number(10, 20);
	m_maxmov = number(50, 80);
	m_maxman = number(40, 70);

	m_stats[STAT_STR] = number(5, 60);
	m_stats[STAT_INT] = number(5, 60);
	m_stats[STAT_WIS] = number(5, 60);
	m_stats[STAT_DEX] = number(5, 60);
	m_stats[STAT_CON] = number(5, 60);
	m_stats[STAT_CHA] = number(5, 60);

	/* some skills */
	m_skills[SKILL_SPELLCASTING] = number(0, 40) + (m_stats[STAT_INT] / 6);
	m_skills[SKILL_AGILITY] = number(0, m_stats[STAT_DEX] / 2);
	m_skills[SKILL_READING] = number(0, 40) + (m_stats[STAT_INT] / 4);
	m_skills[SKILL_WRITING] = max(-1, m_skills[SKILL_READING] - number(0, 40));

	/* other things */
	m_maxhit += number(1, m_stats[STAT_CON] / 12);
	m_maxmov += number(1, m_stats[STAT_DEX] / 4);
	m_maxman += number(1, m_skills[SKILL_SPELLCASTING] / 5);

	/* now add for hints */
	set_hint(m_hint1);
	set_hint(m_hint2);

	m_curhit = m_maxhit;
	m_curmov = m_maxmov;
	m_curman = m_maxman;
}

void Player::show_rolls(void)
{
	write("The following have been rolled for you:\r\n\r\n");
	write("Stats: Strength : %-5d Inteligence : %-5d Wisdom  : %-5d\r\n"
		  "       Dexterity: %-5d Constitution: %-5d Charisma: %-5d\r\n",
		  m_stats[STAT_STR], m_stats[STAT_INT], m_stats[STAT_WIS],
		  m_stats[STAT_DEX], m_stats[STAT_CON], m_stats[STAT_CHA]);
	write("\r\n");
	write("You have %d hitpoints, %d movepoints and %d manapoints.\r\n",
		  m_curhit, m_curmov, m_curman);

	/* now see what guild the player fits best in */
	write("\r\n");
	if (m_skills[SKILL_SPELLCASTING] > 40)
	{
		if (m_skills[SKILL_SPELLCASTING] == 100)
			write("You are an archmage!\r\n");
		else if (m_skills[SKILL_SPELLCASTING] > 90)
			write("You can compete with archmages.\r\n");
		else if (m_skills[SKILL_SPELLCASTING] > 80)
			write("You have very good and deep knowledge in magic.\r\n");
		else if (m_skills[SKILL_SPELLCASTING] > 70)
			write("You are a good magician.\r\n");
		else if (m_skills[SKILL_SPELLCASTING] > 60)
			write("You are pretty normal magician.\r\n");
		else if (m_skills[SKILL_SPELLCASTING] > 50)
			write("You might be a promising magician.\r\n");
		else
			write("You have some spellcasting abilities.\r\n");
	}
	if (m_stats[STAT_STR] > 50 && m_stats[STAT_CON] > 50)
		write("You are a promising fighter.\r\n");
}

void Player::set_hint(const hint_t hint)
{
	switch (hint)
	{
		/* these hints only gives higher stats */
	case HINT_STR:
		m_stats[STAT_STR] += number(1, 10);
		break;
	case HINT_INT:
		m_stats[STAT_INT] += number(2, 10);
		break;
	case HINT_WIS:
		m_stats[STAT_WIS] += number(2, 10);
		break;
	case HINT_DEX:
		m_stats[STAT_DEX] += number(2, 10);
		m_skills[SKILL_AGILITY] += m_stats[STAT_DEX] / 2;
		m_maxmov += number(1, m_stats[STAT_DEX] / 4);
		break;
	case HINT_CON:
		m_stats[STAT_CON] += number(2, 10);
		m_maxhit += number(1, m_stats[STAT_CON] / 12);
		break;
	case HINT_CHA:
		m_stats[STAT_CHA] += number(2, 10);
		break;

		/* these hints gives both higher stats and skills */
	case HINT_WEAPON:
		/* good for warriors */
		m_stats[STAT_STR] += number(1, 5);
		m_stats[STAT_CON] += number(1, 5);
		m_stats[STAT_DEX] += number(1, 5);
		m_skills[SKILL_AGILITY] += m_stats[STAT_DEX] / 2;
		m_maxmov += number(1, m_stats[STAT_DEX] / 4);
		m_maxhit += number(1, m_stats[STAT_CON] / 12);
		break;
	case HINT_ASSASIN:
		m_stats[STAT_DEX] += number(1, 5);
		m_skills[SKILL_AGILITY] += m_stats[STAT_DEX] / 2;
		break;
	case HINT_THIEF:
		m_stats[STAT_DEX] += number(1, 5);
		m_skills[SKILL_AGILITY] += m_stats[STAT_DEX] / 2;
		break;
	case HINT_HUNT:
		/* good for rangers */
		m_stats[STAT_CON] += number(1, 5);
		m_maxhit += number(1, m_stats[STAT_CON] / 12);
		break;
	case HINT_SPELL:
		m_stats[STAT_INT] += number(1, 5);
		break;
	case HINT_NATURE:
		/* good for druids and rangers */
		break;
	case HINT_ANIMAL:
		/* good for druids and rangers */
		break;
	case HINT_GODS:
		/* good for priests */
		m_stats[STAT_WIS] += number(1, 5);
		break;
	case HINT_MAGIC:
		/* good for mages */
		m_stats [STAT_INT] += number(1, 5);
		m_skills[SKILL_SPELLCASTING] += number(5, 50);
		m_maxman += number(1, m_skills[SKILL_SPELLCASTING] / 5);

		if (m_skills[SKILL_SPELLCASTING] >= 60)
		{
			if (m_skills[SKILL_SPELLCASTING] == 100)
			{
				/* five spells at 10% each */
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 10;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 10;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 10;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 10;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 10;
			}
			else if (m_skills[SKILL_SPELLCASTING] >= 90)
			{
				/* four spells at 8% each */
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 8;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 8;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 8;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 8;
			}
			else if (m_skills[SKILL_SPELLCASTING] >= 80)
			{
				/* three spells at 6% each */
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 6;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 6;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 6;
			}
			else if (m_skills[SKILL_SPELLCASTING] >= 70)
			{
				/* two spells at 4% each */
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 4;
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 4;
			}
			else
			{
				/* one spell at 2% */
				m_skills[number(MIN_SPELL, TOP_SPELL)] = 2;
			}
		}
		break;
	default:
		/* TODO:  log this condition */
		break;
	}
}

/*******************************************************************/

void Player::on_tick(void)
{
	/* check idling */

	/* check hunger and thirst and such */

	/* calculate regenerations */

	Character::on_tick();
}

/*******************************************************************/
