/********************************************************************
* File:                                           Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "character.h"

/*******************************************************************/

Character::Character()
	: Object()
{
	m_nextc  = NULL;
	m_curhit = m_maxhit = 15;
	m_curmov = m_maxmov = 100;
	m_curman = m_maxman = 75;
	m_reghit = 2;
	m_regmov = 4;
	m_regman = 3;
	m_level  = 0;

	m_stats[STAT_STR] = 40;
	m_stats[STAT_INT] = 40;
	m_stats[STAT_WIS] = 40;
	m_stats[STAT_DEX] = 40;
	m_stats[STAT_CON] = 40;
	m_stats[STAT_CHA] = 40;
	m_stats[STAT_LUK] = 40;
	m_stats[STAT_EGO] = 40;

	add_property("curhit", &m_curhit);
	add_property("curmov", &m_curmov);
	add_property("curman", &m_curman);
	add_property("maxhit", &m_maxhit);
	add_property("maxmov", &m_maxmov);
	add_property("maxman", &m_maxman);
	add_property("reghit", &m_reghit);
	add_property("regmov", &m_regmov);
	add_property("regman", &m_regman);
	add_property("level" , &m_level);
	add_property("str"   , &m_stats[STAT_STR]);
	add_property("wis"   , &m_stats[STAT_WIS]);
	add_property("int"   , &m_stats[STAT_INT]);
	add_property("dex"   , &m_stats[STAT_DEX]);
	add_property("con"   , &m_stats[STAT_CON]);
	add_property("cha"   , &m_stats[STAT_CHA]);
	add_property("luk"   , &m_stats[STAT_LUK]);
	add_property("ego"   , &m_stats[STAT_EGO]);
	add_property("age"   , &m_age);
	add_property("sex"   , (int *) &m_sex);

	char name[32];
	int  sn;
	for (sn = MIN_SKILL; sn <= TOP_SKILL; sn++)
	{
		sprintf(name, "skill%d", sn);
		add_property(name, &m_skills[sn]);

		m_skills[sn] = -1;
	}
	/*
	for (sn = MIN_SPELL; sn < TOP_SPELL; sn++)
	{
		sprintf(name, "spell%d", sn);
		add_property(name, &m_skills[sn]);

		m_skills[sn] = -1;
	}
	*/
}

Character::~Character()
{
	REMOVE_FROM_LIST(this, server.m_chars, m_nextc, Character);
}

void Character::init(void)
{
	Object::init();
	m_nextc = server.m_chars;
	server.m_chars = this;
}

/*******************************************************************/

bool Character::save(FILE *fl)
{
	if (!Object::save(fl))
		return false;

	fprintf(fl, "\t/* Character properties */\n");
	fprintf(fl, "\tcurhit = %d;\n", m_curhit);
	fprintf(fl, "\tcurmov = %d;\n", m_curmov);
	fprintf(fl, "\tcurman = %d;\n", m_curman);
	fprintf(fl, "\tmaxhit = %d;\n", m_maxhit);
	fprintf(fl, "\tmaxmov = %d;\n", m_maxmov);
	fprintf(fl, "\tmaxman = %d;\n", m_maxman);
	fprintf(fl, "\treghit = %d;\n", m_reghit);
	fprintf(fl, "\tregmov = %d;\n", m_regmov);
	fprintf(fl, "\tregman = %d;\n", m_regman);
	fprintf(fl, "\tlevel  = %d;\n", m_level);
	fprintf(fl, "\tstr    = %d;\n", m_stats[STAT_STR]);
	fprintf(fl, "\twis    = %d;\n", m_stats[STAT_WIS]);
	fprintf(fl, "\tint    = %d;\n", m_stats[STAT_INT]);
	fprintf(fl, "\tdex    = %d;\n", m_stats[STAT_DEX]);
	fprintf(fl, "\tcon    = %d;\n", m_stats[STAT_CON]);
	fprintf(fl, "\tcha    = %d;\n", m_stats[STAT_CHA]);
	fprintf(fl, "\tluk    = %d;\n", m_stats[STAT_LUK]);
	fprintf(fl, "\tego    = %d;\n", m_stats[STAT_EGO]);
	fprintf(fl, "\tsex    = %d;\n", m_sex);
	fprintf(fl, "\tage    = %d;\n", m_age);
	fprintf(fl, "\n");

	int sn;
	fprintf(fl, "\t/* Character skills */\n");
	for (sn = MIN_SKILL; sn < TOP_SKILL; sn++)
	{
		if (m_skills[sn] >= 0)
			fprintf(fl, "\tskill%-3d = %3d;\n", sn, m_skills[sn]);
	}
	fprintf(fl, "\n");
	//fprintf(fl, "\t/* Character spells */\n");
	/*
	for (sn = MIN_SPELL; sn < TOP_SPELL; sn++)
	{
		if (m_skills[sn] >= 0)
			fprintf(fl, "\tspell%-3d = %3d;\n", sn, m_skills[sn]);
	}
	fprintf(fl, "\n");
	*/

	return true;
}

void Character::describe(Character *to, const descr_t typ /* = DESC_SHORT */)
{
	if (!to)
		return;

	Object *o;
	Item   *i;

	if (typ == DESC_SHORT)
	{
		if (is_player())
			to->write("&3%s is standing here.&0\r\n", strcap(get_name()));
		else /* if (is_monster()) */
			to->write("&3%s&0\r\n", get_long());
	}
	else if (typ == DESC_LONG)
	{
		if (to == this)
		{
			write("You sure look handsome!\r\n");
			return;
		}

		if (get_descr() && *get_descr())
			to->write("%s\r\n", get_descr());
		else
			to->write("You see nothing special about %s.\r\n\r\n", him());

		/* write out equipment */
		for (o = m_children; o; o = o->m_nexto)
		{
			if (!o->is_item())
				continue;
			i = (Item *) o;
			if (i->is_equiped())
				i->describe(to, DESC_SHORT);
		}

		if (to->is_wizard())
		{
			/* write out inventory */
			to->write("\r\nYou try to peek into %ss inventory...\r\n", his());
			for (o = m_children; o; o = o->m_nexto)
			{
				if (!o->is_item())
					continue;
				i = (Item *) o;
				if (i->is_equiped())
					continue;
				i->describe(to, DESC_SHORT);
			}
		}
	}
}

/*******************************************************************/

void Character::on_tick(void)
{
	Object::on_tick();

	m_curhit = MIN(m_maxhit, m_curhit + m_reghit);
	m_curman = MIN(m_maxman, m_curman + m_regman);
	m_curmov = MIN(m_maxmov, m_curmov + m_regmov);
}

/*******************************************************************/

void write(Character *ch, const char *fmt, ...)
{
	if (!ch || !fmt || !*fmt)
		return;

	va_list args;

	va_start(args, fmt);
	ch->vwrite(fmt, args);
	va_end(args);
}

void write_all(const char *fmt, ...)
{
	if (!fmt || !*fmt)
		return;

	va_list    args;
	Character *to;

	va_start(args, fmt);
	for (to = server.m_chars; to; to = to->m_nextc)
		to->vwrite(fmt, args);
	va_end(args);
}

void write_all_but(Character *ch, const char *fmt, ...)
{
	if (!ch || !fmt || !*fmt)
		return;

	va_list args;
	Character *to;

	va_start(args, fmt);
	for (to = server.m_chars; to; to = to->m_nextc)
	{
		if (to != ch)
			to->vwrite(fmt, args);
	}
	va_end(args);
}

/*******************************************************************/
