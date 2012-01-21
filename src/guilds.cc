/********************************************************************
* File: guilds.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "guilds.h"
#include "skills.h"

/*******************************************************************/

static void set_none_skills(void);

/*******************************************************************/

static const struct gskills warrior_skills[] = {
	/* common skills */
	{ SKILL_SPELLCASTING, 1, "" },
	{ SKILL_AGILITY     , 1, "" },
	{ SKILL_READING     , 1, "" },
	{ SKILL_WRITING     , 1, "" },

	/* guild specific skills */
	{ SKILL_BODY_KNOWLEDGE, 1, "" },
	{ SKILL_BASIC_WEAPON  , 1, "" },
	{ SKILL_DAGGERS_KNIVES, 1, "" },
	{ SKILL_AXES          , 1, "" },
	{ SKILL_SWORDS        , 1, "" },
	{ SKILL_MOUNTED_FIGHT , 1, "" },
	{ SKILL_UNARMED_FIGHT , 1, "" },
	{ SKILL_MARTIAL_ARTS  , 1, "" },
	{ SKILL_PARY          , 1, "" },
	{ SKILL_DODGE         , 1, "" },
	{ SKILL_WEAPON_CRAFT  , 1, "" },
	{ SKILL_ARMOUR_CRAFT  , 1, "" },
	{ SKILL_ARMOURS       , 1, "" },
	{ SKILL_GUARDING      , 1, "" },
	{ SKILL_ARCHERY       , 1, "" },

	/* end-of-table marker */
	{ 0, 0, NULL }
};

static const struct gskills thief_skills[] = {
	/* common skills */
	{ SKILL_SPELLCASTING, 1, "" },
	{ SKILL_AGILITY     , 1, "" },
	{ SKILL_READING     , 1, "" },
	{ SKILL_WRITING     , 1, "" },

	/* guild specific skills */
	{ SKILL_BODY_KNOWLEDGE, 1, "" },
	{ SKILL_BASIC_WEAPON  , 1, "" },
	{ SKILL_DAGGERS_KNIVES, 1, "" },
	{ SKILL_UNARMED_FIGHT , 1, "" },
	{ SKILL_MARTIAL_ARTS  , 1, "" },
	{ SKILL_PARY          , 1, "" },
	{ SKILL_DODGE         , 1, "" },
	{ SKILL_GUARDING      , 1, "" },
	{ SKILL_GUARDS        , 1, "" },
	{ SKILL_POISONS       , 1, "" },
	{ SKILL_DIPLOMACY     , 1, "" },
	{ SKILL_KEYS_AND_LOCKS, 1, "" },
	{ SKILL_STEALING      , 1, "" },
	{ SKILL_STEALTH       , 1, "" },
	{ SKILL_LANGUAGES     , 1, "" },
	{ SKILL_WORLD_HISTORY , 1, "" },
	{ SKILL_MYTHS_TALES   , 1, "" },
	{ SKILL_VALUABLES     , 1, "" },
	{ SKILL_ANTUIQUES     , 1, "" },

	/* end-of-table marker */
	{ 0, 0, NULL }
};

static const struct gskills ranger_skills[] = {
	/* common skills */
	{ SKILL_SPELLCASTING, 1, "" },
	{ SKILL_AGILITY     , 1, "" },
	{ SKILL_READING     , 1, "" },
	{ SKILL_WRITING     , 1, "" },

	/* guild specific skills */
	{ SKILL_BASIC_WEAPON  , 1, "" },
	{ SKILL_AXES          , 1, "" },
	{ SKILL_SWORDS        , 1, "" },
	{ SKILL_MOUNTED_FIGHT , 1, "" },
	{ SKILL_PARY          , 1, "" },
	{ SKILL_DODGE         , 1, "" },
	{ SKILL_GUARDING      , 1, "" },
	{ SKILL_ARCHERY       , 1, "" },
	{ SKILL_PLANTS_TREES  , 1, "" },
	{ SKILL_BASIC_HERB    , 1, "" },
	{ SKILL_ANIMALS       , 1, "" },
	{ SKILL_TRACK_AND_HUNT, 1, "" },
	{ SKILL_NATURAL_SIGNS , 1, "" },
	{ SKILL_STAFFS        , 1, "" },

	/* end-of-table marker */
	{ 0, 0, NULL }
};

static const struct gskills mage_skills[] = {
	/* common skills */
	{ SKILL_SPELLCASTING, 1, "" },
	{ SKILL_AGILITY     , 1, "" },
	{ SKILL_READING     , 1, "" },
	{ SKILL_WRITING     , 1, "" },

	/* guild specific skills */
	{ SKILL_BASIC_WEAPON , 1, "" },
	{ SKILL_ART_OF_MAGIC , 1, "" },
	{ SKILL_ELEMENTS     , 1, "" },
	{ SKILL_BASIC_SPELLS , 1, "" },
	{ SKILL_INT_SPELLS   , 1, "" },
	{ SKILL_ADV_SPELLS   , 1, "" },
	{ SKILL_DARK_SPELLS  , 1, "" },
	{ SKILL_POWER_SCROLLS, 1, "" },

	/* guild specific spells */

	/* end-of-table marker */
	{ 0, 0, NULL }
};

static const struct gskills druid_skills[] = {
	/* common skills */
	{ SKILL_SPELLCASTING, 1, "" },
	{ SKILL_AGILITY     , 1, "" },
	{ SKILL_READING     , 1, "" },
	{ SKILL_WRITING     , 1, "" },

	/* guild specific skills */
	{ SKILL_BASIC_WEAPON  , 1, "" },
	{ SKILL_PLANTS_TREES  , 1, "" },
	{ SKILL_BASIC_HERB    , 1, "" },
	{ SKILL_ADV_HERB      , 1, "" },
	{ SKILL_ANIMALS       , 1, "" },
	{ SKILL_TRACK_AND_HUNT, 1, "" },
	{ SKILL_NATURAL_SIGNS , 1, "" },
	{ SKILL_POTION_MAKING , 1, "" },
	{ SKILL_SHAPESHIFTING , 1, "" },
	{ SKILL_SORCERY       , 1, "" },
	{ SKILL_LIGHT_RITES   , 1, "" },
	{ SKILL_DARK_RITES    , 1, "" },
	{ SKILL_CHANTS        , 1, "" },
	{ SKILL_CRT_CALLING   , 1, "" },
	{ SKILL_STAFFS        , 1, "" },

	/* guild specific spells */

	/* end-of-table marker */
	{ 0, 0, NULL }
};

static const struct gskills priest_skills[] = {
	/* common skills */
	{ SKILL_SPELLCASTING, 1, "" },
	{ SKILL_AGILITY     , 1, "" },
	{ SKILL_READING     , 1, "" },
	{ SKILL_WRITING     , 1, "" },

	/* guild specific skills */
	{ SKILL_BASIC_WEAPON , 1, "" },
	{ SKILL_DIPLOMACY    , 1, "" },
	{ SKILL_LANGUAGES    , 1, "" },
	{ SKILL_WORLD_HISTORY, 1, "" },
	{ SKILL_MYTHS_TALES  , 1, "" },
	{ SKILL_CHANTS       , 1, "" },
	{ SKILL_MIRACLES     , 1, "" },
	{ SKILL_MEDECINE     , 1, "" },
	{ SKILL_HEALING      , 1, "" },
	{ SKILL_STORYTELLING , 1, "" },
	{ SKILL_PRAYERS      , 1, "" },
	{ SKILL_BLESSINGS    , 1, "" },

	/* guild specific spells */

	/* end-of-table marker */
	{ 0, 0, NULL }
};

struct guild guilds[N_GUILDS] = {
	{ "none"   , NULL, NULL },  /* can have all skills */
	{ "warrior", warrior_skills, NULL },
	{ "thief"  ,   thief_skills, NULL },
	{ "ranger" ,  ranger_skills, NULL },
	{ "mage"   ,    mage_skills, NULL },
	{ "druid"  ,   druid_skills, NULL },
	{ "priest" ,  priest_skills, NULL },
};

/*******************************************************************/

bool guild_init(void)
{
	guilds[GUILD_NONE   ].m_master = (Monster *) find_object(   NONE_MASTER);
	guilds[GUILD_WARRIOR].m_master = (Monster *) find_object(WARRIOR_MASTER);
	guilds[GUILD_THIEF  ].m_master = (Monster *) find_object(  THIEF_MASTER);
	guilds[GUILD_RANGER ].m_master = (Monster *) find_object( RANGER_MASTER);
	guilds[GUILD_MAGE   ].m_master = (Monster *) find_object(   MAGE_MASTER);
	guilds[GUILD_DRUID  ].m_master = (Monster *) find_object(  DRUID_MASTER);
	guilds[GUILD_PRIEST ].m_master = (Monster *) find_object( PRIEST_MASTER);

	set_none_skills();

	const struct gskills *gs;
	int g, i;
	for (g = 1; g < N_GUILDS; g++)
	{
		gs = guilds[g].m_skills;
		for (i = 0; gs[i].skill; i++)
			set_skill(gs[i].skill, g, gs[i].level, gs[i].teacher);
	}

	return true;
}

/*******************************************************************/

static void set_none_skills(void)
{
	/* players without a guild have all skills from their first level */

	/* TODO:  add skillmasters for all skills! */
	set_skill(SKILL_SPELLCASTING  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_AGILITY       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_READING       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_WRITING       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_BODY_KNOWLEDGE, GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_BASIC_WEAPON  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_DAGGERS_KNIVES, GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_AXES          , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_SWORDS        , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_SCROLL_READING, GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_MOUNTED_FIGHT , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_UNARMED_FIGHT , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_MARTIAL_ARTS  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_PARY          , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_DODGE         , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_WEAPON_CRAFT  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ARMOUR_CRAFT  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ARMOURS       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_GUARDING      , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_POISONS       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_DIPLOMACY     , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_KEYS_AND_LOCKS, GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_STEALING      , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_STEALTH       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_LANGUAGES     , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_WORLD_HISTORY , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_MYTHS_TALES   , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_VALUABLES     , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ANTUIQUES     , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_GUARDS        , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_PLANTS_TREES  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_BASIC_HERB    , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ADV_HERB      , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ANIMALS       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_TRACK_AND_HUNT, GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ARCHERY       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_NATURAL_SIGNS , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ART_OF_MAGIC  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ELEMENTS      , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_BASIC_SPELLS  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_INT_SPELLS    , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_ADV_SPELLS    , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_DARK_SPELLS   , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_POWER_SCROLLS , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_CHANTS        , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_POTION_MAKING , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_SHAPESHIFTING , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_SORCERY       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_LIGHT_RITES   , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_DARK_RITES    , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_CRT_CALLING   , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_MIRACLES      , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_MEDECINE      , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_HEALING       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_STORYTELLING  , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_PRAYERS       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_BLESSINGS     , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_HAMMERS       , GUILD_NONE, 1, (char *) NULL);
	set_skill(SKILL_STAFFS        , GUILD_NONE, 1, (char *) NULL);
}

/*******************************************************************/
